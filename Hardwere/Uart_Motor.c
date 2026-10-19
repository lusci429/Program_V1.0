#include "Uart_Motor.h"

/**
 * @brief 多机速度模式控制
 * @param spd1~spd4 四个电机的速度 (单位：RPM)
 * @param acc       加速度档位 (0~255)
 */
void Send_Multi_Speed(int16_t spd1, int16_t spd2, int16_t spd3, int16_t spd4, uint8_t acc)
{
    uint8_t tx_buf[32];
    uint8_t idx = 0;
    
    // 帧头
    tx_buf[idx++] = 0x00; tx_buf[idx++] = 0xAA; 
    tx_buf[idx++] = 0x00; tx_buf[idx++] = 0x1C;

    // 电机 1
    tx_buf[idx++] = 0x01; tx_buf[idx++] = 0xF6;
    tx_buf[idx++] = (spd1 >= 0) ? 0x00 : 0x01; 
    uint16_t a1 = (spd1 >= 0) ? spd1 : (uint16_t)(-spd1);
    tx_buf[idx++] = (a1 >> 8) & 0xFF; tx_buf[idx++] = a1 & 0xFF;
    tx_buf[idx++] = acc; tx_buf[idx++] = 0x00;

    // 电机 2
    tx_buf[idx++] = 0x02; tx_buf[idx++] = 0xF6;
    tx_buf[idx++] = (spd2 >= 0) ? 0x00 : 0x01;
    uint16_t a2 = (spd2 >= 0) ? spd2 : (uint16_t)(-spd2);
    tx_buf[idx++] = (a2 >> 8) & 0xFF; tx_buf[idx++] = a2 & 0xFF;
    tx_buf[idx++] = acc; tx_buf[idx++] = 0x00;

    // 电机 3
    tx_buf[idx++] = 0x03; tx_buf[idx++] = 0xF6;
    tx_buf[idx++] = (spd3 >= 0) ? 0x00 : 0x01;
    uint16_t a3 = (spd3 >= 0) ? spd3 : (uint16_t)(-spd3);
    tx_buf[idx++] = (a3 >> 8) & 0xFF; tx_buf[idx++] = a3 & 0xFF;
    tx_buf[idx++] = acc; tx_buf[idx++] = 0x00;

    // 电机 4
    tx_buf[idx++] = 0x04; tx_buf[idx++] = 0xF6;
    tx_buf[idx++] = (spd4 >= 0) ? 0x00 : 0x01;
    uint16_t a4 = (spd4 >= 0) ? spd4 : (uint16_t)(-spd4);
    tx_buf[idx++] = (a4 >> 8) & 0xFF; tx_buf[idx++] = a4 & 0xFF;
    tx_buf[idx++] = acc; tx_buf[idx++] = 0x00;

    // 校验码
    tx_buf[idx++] = 0x6B;

    // ?? 关键修改：强行清除可能卡死的状态，并用阻塞方式发
    huart2.gState = HAL_UART_STATE_READY;
    huart2.RxState = HAL_UART_STATE_READY;
    HAL_UART_Transmit(&huart2, tx_buf, idx, 100); 
}