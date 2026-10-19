#ifndef __HWT101_H
#define __HWT101_H

#include "main.h"

#define HWT101_RX_BUFFER_SIZE 64

void HWT101_Init(void);
void HWT101_IDLE_Handler(void);
void HWT101_Process(void);
float HWT101_Get_Yaw(void);
float HWT101_Get_Yaw_0_360(void);
uint8_t HWT101_Get_UpdateFlag(void);
void HWT101_Reset_Yaw(void);
void HWT101_Calibrate(void); 

#endif
