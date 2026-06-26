/**
 * @file   uart.h
 * @brief  UART0 中断接收模块 (PA31 RX, 115200bps)
 *
 * 协议: 以 "/n" 为帧分隔符
 */

#ifndef UART_H
#define UART_H

#include <stdbool.h>

/**
 * @brief 初始化UART中断接收
 */
void UART_Init(void);

/**
 * @brief 检查是否收到完整一帧
 * @return true=有数据可读
 */
bool UART_LineReady(void);

/**
 * @brief 获取接收到的帧数据 (以 '\0' 结尾)
 */
const char *UART_GetLine(void);

/**
 * @brief 释放当前帧, 准备接收下一帧
 */
void UART_NextLine(void);

#endif /* UART_H */
