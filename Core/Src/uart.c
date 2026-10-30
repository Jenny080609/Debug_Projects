#include "uart.h"
#include <string.h>

/* huart1 是在 main.c 里定义的，这里必须用 extern 声明后才能使用 */
extern UART_HandleTypeDef huart1;

/* ================= 串口接收相关变量 ================= */
char             rx_buf[RX_BUF_SIZE];      /* 一行的内容，如 "a 111" */
volatile uint8_t rx_flag = 0;              /* 收到换行后置 1         */
volatile uint8_t uart_refresh_flag = 0;    /* var_a 改变后置 1       */

static volatile uint8_t rx_len = 0;        /* 当前已收到多少个字符   */
static uint8_t rx_byte;                    /* 中断接收用的单字节临时变量 */

/* ==================================================================
 * 启动第一次接收中断
 * 之后每收到一个字节，HAL 会自动进入 HAL_UART_RxCpltCallback
 * ================================================================== */
void UART_Start(void)
{
    rx_len = 0;
    rx_flag = 0;
    memset(rx_buf, 0, sizeof(rx_buf));

    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

/* ==================================================================
 * 串口接收完成回调：每收到一个字符进来一次
 * 收到 \n 或 \r 表示一行结束，置位 rx_flag
 * ================================================================== */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
			if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_ORE) != RESET)
        {
            __HAL_UART_CLEAR_OREFLAG(&huart1);
        }
        if (rx_byte == '\n' || rx_byte == '\r')
        {
            /* 一行结束：补字符串结束符，置标志，长度清零 */
            if (rx_len > 0)
            {
                rx_buf[rx_len] = '\0';
                rx_flag = 1;
                rx_len = 0;
            }
        }
        else if (rx_len < RX_BUF_SIZE - 1)
        {
            /* 普通字符：存入缓冲区 */
            rx_buf[rx_len++] = (char)rx_byte;
        }

        /* 重新开启接收，等待下一个字节（必须写，否则只收一个字节） */
        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
    }
}

/* ==================================================================
 * 串口任务：主循环里反复调用
 * 解析两种命令："111" 修改变量；"GET A" 读取变量
 * ================================================================== */
void UART_Task(void)
{
    int value;

    if (rx_flag == 0)
        return;                 /* 还没收到完整一行 */

    rx_flag = 0;
		printf("---收到指令：[%s]\r\n",rx_buf);

    /* 1. 数字命令：如 "111"，直接修改变量 a */
    if (sscanf(rx_buf, "%d", &value) == 1)
    {
        var_a = value;              /* 修改全局变量 a */
        uart_refresh_flag = 1;      /* 通知主循环刷新 OLED */
			  printf("---解析成功！var_a = %d\r\n",var_a);
    }
    /* 2. 查询命令："GET A"，把当前变量 a 的值发回上位机 */
    else if (strncmp(rx_buf, "GET A", 5) == 0)
    {
        printf("%d\r\n", var_a);
    }
    rx_flag = 0;
    rx_len = 0;

    /* 清空缓冲区，等待下一条命令 */
    memset(rx_buf, 0, sizeof(rx_buf));
}
