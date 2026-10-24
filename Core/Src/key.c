#include "key.h"

/*
 * 读取 PA0~PA3 的实际电平，返回当前按下的键号
 * 按下为低电平，所以判断 == GPIO_PIN_RESET
 * 多个键同时按下时，按 1→2→3→4 的优先级取第一个
 */
static uint8_t Key_Read_Pin(void)
{
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) return 1;
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_RESET) return 2;
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == GPIO_PIN_RESET) return 3;
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_RESET) return 4;
    return 0;
}

/*
 * 按键扫描函数
 * 思路：
 *   1. 先读一次，如果和上次状态一样，直接返回 0（说明没变化）
 *   2. 如果发生变化，延时 20ms 等抖动过去
 *   3. 再读一次，两次结果一致才认为是真的按下/松开
 *   4. 只有"稳定按下"的那一瞬间返回键号，按住不放不会重复触发
 */
uint8_t Key_Scan(void)
{
    static uint8_t key_last = 0;   /* 上一次稳定后的按键状态 */
    uint8_t key_now;
    uint8_t key_ok;

    key_now = Key_Read_Pin();          /* 第一次读 */

    if (key_now == key_last)           /* 和上次一样，没有新动作 */
        return 0;

    HAL_Delay(20);                     /* 软件消抖：等 20ms */

    key_ok = Key_Read_Pin();           /* 第二次读，确认电平稳定 */

    key_last = key_ok;                 /* 记录这次稳定后的状态 */

    if (key_ok == 0)                   /* 是"松开"，不返回键号 */
        return 0;

    return key_ok;                     /* 是"按下"，返回 1~4 */
}
