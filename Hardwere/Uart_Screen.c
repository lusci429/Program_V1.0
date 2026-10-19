#include "Uart_Screen.h"
#include <string.h>
#include "usart.h"

extern UART_HandleTypeDef huart5;

#define SCREEN_BUF_SIZE  64

uint8_t screen_tx_buf[SCREEN_BUF_SIZE];

uint8_t  screen_rx_buffer[SCREEN_BUF_SIZE];
uint8_t  screen_rx_temp[SCREEN_BUF_SIZE];
volatile uint16_t screen_rx_len  = 0;
volatile uint8_t  screen_rx_flag = 0;

volatile uint8_t  Uart_Screen_OK = 0;

void Uart_Screen_Init(void)
{
    memset(screen_rx_buffer, 0, sizeof(screen_rx_buffer));
    memset(screen_rx_temp,   0, sizeof(screen_rx_temp));

    screen_rx_len   = 0;
    screen_rx_flag  = 0;
    Uart_Screen_OK  = 0;

    HAL_UART_Receive_IT(&huart5, screen_rx_buffer, 1);
}

void Uart_Screen_RxCallback(void)
{
    uint8_t byte = screen_rx_buffer[0];

    if (screen_rx_len < SCREEN_BUF_SIZE - 1)
    {
        screen_rx_temp[screen_rx_len++] = byte;
    }

    /* ¼ì²â AA BB 01 CC DD */
    if (screen_rx_len >= 5)
    {
        uint16_t k = screen_rx_len - 5;
        if (screen_rx_temp[k]     == 0xAA &&
            screen_rx_temp[k + 1] == 0xBB &&
            screen_rx_temp[k + 2] == 0x01 &&
            screen_rx_temp[k + 3] == 0xCC &&
            screen_rx_temp[k + 4] == 0xDD)
        {
            Uart_Screen_OK = 1;
            screen_rx_len  = 0;
        }
    }

    if (byte == '\r' || byte == '\n')
    {
        screen_rx_flag = 1;
        screen_rx_len  = 0;
    }

    HAL_UART_Receive_IT(&huart5, screen_rx_buffer, 1);
}

void Uart_Screen_Send(uint8_t *buf, uint16_t len)
{
    HAL_UART_Transmit(&huart5, buf, len, 1000);
}

void Uart_Screen_SendPacket(uint8_t *buf, uint16_t len)
{
    uint8_t head[2] = {0xAA, 0xBB};
    uint8_t tail[2] = {0xCC, 0xDD};

    Uart_Screen_Send(head, 2);
    Uart_Screen_Send(buf, len);
    Uart_Screen_Send(tail, 2);
}

