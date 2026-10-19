#include "HWT101.h"
#include <string.h>

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef  hdma_usart1_rx;

static float yaw         = 0.0f;    // -180 ~ +180
static float yaw_0_360   = 0.0f;    //    0 ~ 360

static uint8_t  rx_buffer[HWT101_RX_BUFFER_SIZE];
static uint8_t  rx_temp[HWT101_RX_BUFFER_SIZE];
static volatile uint16_t rx_len  = 0;
static volatile uint8_t  rx_flag = 0;
static uint8_t  update_flag      = 0;

void HWT101_Init(void)
{
    memset(rx_buffer, 0, sizeof(rx_buffer));
    memset(rx_temp,   0, sizeof(rx_temp));

    HAL_UART_Receive_DMA(&huart1, rx_buffer, HWT101_RX_BUFFER_SIZE);
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
}

void HWT101_IDLE_Handler(void)
{
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE))
    {
        __HAL_UART_CLEAR_IDLEFLAG(&huart1);

        HAL_UART_DMAStop(&huart1);
        uint16_t remain = __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);
        rx_len = HWT101_RX_BUFFER_SIZE - remain;

        if (rx_len > 0 && rx_len <= HWT101_RX_BUFFER_SIZE)
        {
            memcpy(rx_temp, rx_buffer, rx_len);
            rx_flag = 1;
        }

        HAL_UART_Receive_DMA(&huart1, rx_buffer, HWT101_RX_BUFFER_SIZE);
    }
}

void HWT101_Process(void)
{
    uint8_t i;
    if (rx_flag == 0) return;
    rx_flag = 0;

    for (i = 0; i + 10 < rx_len; i++)
    {
        if (rx_temp[i] == 0x55 && rx_temp[i + 1] == 0x53)
        {
            uint8_t sum = 0;
            uint8_t k;
            for (k = 0; k < 10; k++) sum += rx_temp[i + k];
            
            // 校验和正确
            if (sum == rx_temp[i + 10])
            {
                // ? 修正：Yaw 在第 4 和第 5 字节
                int16_t rawYaw = (int16_t)((rx_temp[i + 5] << 8) | rx_temp[i + 4]);
                
                yaw = rawYaw / 32768.0f * 180.0f;
                
                if (yaw < 0.0f)
                    yaw_0_360 = yaw + 360.0f;
                else
                    yaw_0_360 = yaw;

                update_flag = 1;
                i += 10; // 跳过整帧
            }
        }
    }
}

float HWT101_Get_Yaw(void) { return yaw; }
float HWT101_Get_Yaw_0_360(void) { return yaw_0_360; }

uint8_t HWT101_Get_UpdateFlag(void)
{
    if (update_flag) { update_flag = 0; return 1; }
    return 0;
}

// Z 轴置零 (设置当前角度为 0 度)
void HWT101_Reset_Yaw(void)
{
    uint8_t cmd[5] = {0xFF, 0xAA, 0x76, 0x00, 0x00};
    HAL_UART_Transmit(&huart1, cmd, 5, 100);
}

// 零偏校准 (需静止 20 秒)
void HWT101_Calibrate(void)
{
    uint8_t cmd_start[5] = {0xFF, 0xAA, 0x48, 0x01, 0x00};
    uint8_t cmd_stop[5]  = {0xFF, 0xAA, 0x48, 0x00, 0x00};
    HAL_UART_Transmit(&huart1, cmd_start, 5, 100);
    HAL_Delay(20000); // 等待 20 秒，期间小车必须静止！
    HAL_UART_Transmit(&huart1, cmd_stop, 5, 100);
}
