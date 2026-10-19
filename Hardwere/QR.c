#include "QR.h"
#include <string.h>
#include "Uart_Screen.h"
#include "usart.h"

#define QR_RX_BUF_SIZE   64

extern UART_HandleTypeDef huart4;
extern volatile uint8_t  Uart_Screen_OK;

/* ========== 接收缓冲 ========== */
static uint8_t  qr_rx_byte;
static uint8_t  qr_rx_temp[QR_RX_BUF_SIZE];
static uint16_t qr_rx_len = 0;

/* ========== 解析状态 ========== */
static uint8_t  qr_group = 0;    // 当前第几组 0~3
static uint8_t  qr_digit = 0;    // 组内第几位 0~2

/* ========== 暂存数组（扫到的合法数据，未确认） ========== */
static uint8_t  qr_temp_val[4][3] = {0};

/* ========== 确认数组（只有收到 AA BB 01 CC DD 才更新） ========== */
uint8_t QR_Val[4][3] = {0};

/* ========== 暂存数据是否有待确认的完整帧 ========== */
static uint8_t  qr_frame_ready = 0;    // 1 = qr_temp_val 里有一帧完整数据

/* ============================================================
 *  初始化
 * ============================================================ */
void QR_Init(void)
{
    memset(qr_rx_temp,  0, sizeof(qr_rx_temp));
    memset(qr_temp_val, 0, sizeof(qr_temp_val));
    memset(QR_Val,      0, sizeof(QR_Val));

    qr_rx_len      = 0;
    qr_group       = 0;
    qr_digit       = 0;
    qr_frame_ready = 0;

    HAL_UART_Receive_IT(&huart4, &qr_rx_byte, 1);
}

/* ============================================================
 *  ★ 清空接收状态（格式错误时调用）
 *  注意：不清 QR_Val（确认数组），不清 qr_frame_ready
 * ============================================================ */
static void QR_ResetRxBuffer(void)
{
    qr_rx_len = 0;
    qr_group  = 0;
    qr_digit  = 0;
    memset(qr_rx_temp, 0, sizeof(qr_rx_temp));
}

/* ============================================================
 *  收到 1 字节
 * ============================================================ */
void QR_RxCallback(void)
{
    /* 存原始字节（用于转发） */
    if (qr_rx_len < QR_RX_BUF_SIZE - 1)
    {
        qr_rx_temp[qr_rx_len++] = qr_rx_byte;
    }

    char c = (char)qr_rx_byte;

    /* ---------- 1. 数字 ---------- */
    if (c >= '0' && c <= '9')
    {
        if (qr_group < 4 && qr_digit < 3)
        {
            qr_temp_val[qr_group][qr_digit] = c - '0';
            qr_digit++;
        }
        else
        {
            /* 一组超过 3 位 → 格式错误，清空 */
            QR_ResetRxBuffer();
        }

        /* 第 4 组第 3 位收完 → 完整一帧 */
        if (qr_group == 3 && qr_digit == 3)
        {
            qr_frame_ready = 1;                    // ★ 暂存区有完整帧

            /* 通过 UART5 发送 */
            Uart_Screen_SendPacket(qr_rx_temp, qr_rx_len);

            /* 清接收状态，准备下一帧 */
            QR_ResetRxBuffer();
        }
    }
    /* ---------- 2. '+' 分隔符 ---------- */
    else if (c == '+')
    {
        if (qr_digit == 3 && qr_group < 3)
        {
            qr_group++;
            qr_digit = 0;
        }
        else
        {
            /* 组内不足 3 位 / 第 4 组后又出现 + → 格式错误 */
            QR_ResetRxBuffer();
        }
    }
    /* ---------- 3. 空白字符 ---------- */
    else if (c == '\r' || c == '\n' || c == ' ')
    {
        /* 遇到换行时，如果当前还有未完成的数据 → 丢弃 */
        if ((c == '\r' || c == '\n') && qr_rx_len > 0)
        {
            if (!(qr_group == 3 && qr_digit == 3))
            {
                QR_ResetRxBuffer();
            }
        }
    }
    /* ---------- 4. 其它字符 → 格式错误 ---------- */
    else
    {
        QR_ResetRxBuffer();
    }

    /* 继续接收 */
	if(Uart_Screen_OK==0)
	{
		HAL_UART_Receive_IT(&huart4, &qr_rx_byte, 1);
	}
    
}

/* ============================================================
 *  主循环：收到屏幕确认 → 把暂存数组转正
 * ============================================================ */
void QR_Process(void)
{
    /* 收到确认 + 暂存区有完整帧 → 转正 */
    if (Uart_Screen_OK == 1)
    {
        if (qr_frame_ready == 1)
        {
            /* ★ 把暂存数组拷贝到确认数组 */
            memcpy(QR_Val, qr_temp_val, sizeof(QR_Val));
            qr_frame_ready = 0;
        }

        /* 无论是否转正，都清确认标志 */
        Uart_Screen_OK = 0;
    }
}

/* ============================================================
 *  对外查询
 * ============================================================ */
uint8_t QR_Get_Val(uint8_t row, uint8_t col)
{
    if (row >= 4 || col >= 3) return 0;
    return QR_Val[row][col];
}
