#ifndef __QR_H
#define __QR_H

#include "main.h"

/* 已确认的数据（用户通过 QR_Get_Val 或直接访问） */
extern uint8_t QR_Val[4][3];

void QR_Init(void);
void QR_RxCallback(void);
void QR_Process(void);
uint8_t QR_Get_Val(uint8_t row, uint8_t col);

#endif
