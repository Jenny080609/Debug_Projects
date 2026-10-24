#include "led.h"

/* ==================================================================
 * 内部变量
 * ================================================================== */

/* 当前模式，上电默认常灭 */
LED_Mode_t led_mode = LED_MODE_OFF;

/* 两个 LED 当前实际亮灭：1=亮，0=灭 */
static uint8_t led1_on = 0;
static uint8_t led2_on = 0;

/* 闪烁计时用的两个时间戳（单位：ms） */
static uint32_t blink_start_tick  = 0;   /* 本次闪烁开始的时间 */
static uint32_t blink_last_toggle = 0;   /* 上次翻转的时间     */

/* ==================================================================
 * 底层写引脚
 * LED1 接 PA4，LED2 接 PA5，低电平点亮
 * ================================================================== */
static void LED_Write(uint8_t on1, uint8_t on2)
{
    /* 亮 → 输出低电平(RESET)；灭 → 输出高电平(SET) */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, on1 ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, on2 ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

/* ==================================================================
 * 串口打印当前状态（模拟 OLED）
 * ================================================================== */
void LED_Display_Status(void)
{
    if (led_mode == LED_MODE_BLINK)
    {
        /* 算剩余秒数：(总时长 - 已过时间 + 999) / 1000 向上取整 */
        uint32_t elapsed = HAL_GetTick() - blink_start_tick;
        uint32_t remain_sec;

        if (elapsed >= LED_BLINK_DURATION_MS)
            remain_sec = 0;
        else
            remain_sec = (LED_BLINK_DURATION_MS - elapsed + 999) / 1000;

        printf("状态: 交替闪烁中 (剩余 %lu 秒) | LED1: %s, LED2: %s\r\n",
               (unsigned long)remain_sec,
               led1_on ? "ON" : "OFF",
               led2_on ? "ON" : "OFF");
    }
    else if (led_mode == LED_MODE_ON)
    {
        printf("状态: 常亮 | LED1: ON, LED2: ON\r\n");
    }
    else
    {
        printf("状态: 常灭 | LED1: OFF, LED2: OFF\r\n");
    }
}

/* ==================================================================
 * 按键1：常灭 → 常亮，常亮 → 常灭
 * 如果正在闪烁，先打断闪烁，再变成常亮
 * ================================================================== */
void LED_Toggle_All(void)
{
    if (led_mode == LED_MODE_BLINK)
    {
        /* 闪烁中按按键1：直接打断，变为常亮（两个都亮） */
        led_mode = LED_MODE_ON;
    }
    else if (led_mode == LED_MODE_ON)
    {
        led_mode = LED_MODE_OFF;
    }
    else
    {
        led_mode = LED_MODE_ON;
    }

    /* 按新模式设置实际引脚 */
    if (led_mode == LED_MODE_ON)
    {
        led1_on = 1;
        led2_on = 1;
    }
    else
    {
        led1_on = 0;
        led2_on = 0;
    }
    LED_Write(led1_on, led2_on);

    LED_Display_Status();   /* 状态改变，立即打印 */
}

/* ==================================================================
 * 按键2：开始交替闪烁
 * 每次按下都重新计时 30 秒
 * ================================================================== */
void LED_Start_Blink(void)
{
    uint32_t now = HAL_GetTick();

    led_mode          = LED_MODE_BLINK;
    blink_start_tick  = now;      /* 记录开始时间，用于判断 30 秒到期 */
    blink_last_toggle = now;      /* 记录上次翻转时间，用于 500ms 间隔 */

    /* 起始状态：LED1 亮、LED2 灭 */
    led1_on = 1;
    led2_on = 0;
    LED_Write(led1_on, led2_on);

    LED_Display_Status();         /* 状态改变，立即打印 */
}

/* ==================================================================
 * 按键4 / 离开 LED 界面：强制关闭
 * ================================================================== */
void LED_Force_Off(void)
{
    led_mode = LED_MODE_OFF;
    led1_on  = 0;
    led2_on  = 0;
    LED_Write(0, 0);
}

/* ==================================================================
 * 闪烁任务：必须放在 while(1) 里每轮调用
 * 全程用 HAL_GetTick() 判断时间，不使用 HAL_Delay，不会阻塞其它任务
 * ================================================================== */
void LED_Task(void)
{
    uint32_t now;

    /* 不是闪烁模式，什么都不用做（常亮/常灭的引脚已经设好了） */
    if (led_mode != LED_MODE_BLINK)
        return;

    now = HAL_GetTick();

    /* 1. 先判断 30 秒有效期是否已到 */
    if ((now - blink_start_tick) >= LED_BLINK_DURATION_MS)
    {
        /* 到期：自动停止闪烁，两个 LED 全部熄灭 */
        led_mode = LED_MODE_OFF;
        led1_on  = 0;
        led2_on  = 0;
        LED_Write(0, 0);
        LED_Display_Status();     /* 状态改变，打印一次 */
        return;
    }

    /* 2. 每满 500ms 翻转一次 */
    if ((now - blink_last_toggle) >= LED_BLINK_INTERVAL_MS)
    {
        blink_last_toggle = now;  /* 记录本次翻转时间 */

        led1_on = !led1_on;
        led2_on = !led2_on;
        LED_Write(led1_on, led2_on);

        LED_Display_Status();     /* 每次翻转都打印最新状态 */
    }
}
