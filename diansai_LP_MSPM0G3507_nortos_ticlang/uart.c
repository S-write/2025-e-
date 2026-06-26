/**
 * @file   uart.c
 * @brief  UART0 中断接收模块实现
 */

#include "uart.h"
#include "ti_msp_dl_config.h"

#define RX_BUF_SIZE  64

static volatile char    g_rx_buf[RX_BUF_SIZE];
static volatile uint8_t g_rx_idx   = 0;
static volatile bool    g_rx_done  = false;

void UART_Init(void)
{
    __enable_irq();
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN);
}

bool UART_LineReady(void)
{
    return g_rx_done;
}

const char *UART_GetLine(void)
{
    return (const char *)g_rx_buf;
}

void UART_NextLine(void)
{
    g_rx_idx  = 0;
    g_rx_done = false;
}

void UART_0_INST_IRQHandler(void)
{
    static bool slash_seen = false;

    switch (DL_UART_Main_getPendingInterrupt(UART_0_INST)) {
    case DL_UART_MAIN_IIDX_RX:
        while (!DL_UART_isRXFIFOEmpty(UART_0_INST)) {
            char c = (char)DL_UART_Main_receiveData(UART_0_INST);

            if (slash_seen) {
                slash_seen = false;
                if (c == 'n') {
                    g_rx_buf[g_rx_idx] = '\0';
                    g_rx_done = true;
                } else {
                    if (g_rx_idx < RX_BUF_SIZE - 1) g_rx_buf[g_rx_idx++] = '/';
                    if (g_rx_idx < RX_BUF_SIZE - 1) g_rx_buf[g_rx_idx++] = c;
                }
            } else if (c == '/') {
                slash_seen = true;
            } else if (c != '\r' && g_rx_idx < RX_BUF_SIZE - 1) {
                g_rx_buf[g_rx_idx++] = c;
            }
        }
        break;

    default:
        break;
    }
}
