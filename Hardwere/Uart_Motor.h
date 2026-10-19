#ifndef __UART_MOTOR_H
#define __UART_MOTOR_H

#include "main.h"
#include <stdint.h>

extern UART_HandleTypeDef huart2;

// 速度控制函数
void Send_Multi_Speed(int16_t spd1, int16_t spd2, int16_t spd3, int16_t spd4, uint8_t acc);

#endif