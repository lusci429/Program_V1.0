#ifndef __UART_SCREEN_H
#define __UART_SCREEN_H

#include "main.h"

extern uint8_t screen_tx_buf[64];

extern uint8_t  screen_rx_buffer[64];
extern uint8_t  screen_rx_temp[64];
extern volatile uint16_t screen_rx_len;
extern volatile uint8_t  screen_rx_flag;

extern volatile uint8_t  Uart_Screen_OK;

void Uart_Screen_Init(void);
void Uart_Screen_RxCallback(void);
void Uart_Screen_Send(uint8_t *buf, uint16_t len);
void Uart_Screen_SendPacket(uint8_t *buf, uint16_t len);

#endif
