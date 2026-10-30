#ifndef __UART_H
#define __UART_H

#include "app.h"

#define RX_BUF_SIZE  64

/* 接收缓冲区与标志（全局，中断和主循环共用） */
extern char             rx_buf[RX_BUF_SIZE];   /* 存放收到的一整行命令 */
extern volatile uint8_t rx_flag;                /* 1 = 收到完整一行     */
extern volatile uint8_t uart_refresh_flag;      /* 1 = var_a 被修改，需刷新屏幕 */

void UART_Start(void);   /* 启动第一次接收中断（main 里调用一次） */
void UART_Task(void);    /* 主循环里反复调用：解析命令           */

#endif
