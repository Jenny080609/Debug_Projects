#ifndef __LED_H
#define __LED_H

#include "app.h"

/* ---------------- LED 状态定义 ---------------- */
typedef enum {
    LED_MODE_OFF   = 0,   /* 常灭 */
    LED_MODE_ON    = 1,   /* 常亮 */
    LED_MODE_BLINK = 2    /* 交替闪烁 */
} LED_Mode_t;

/* 闪烁参数（想改时间只改这里） */
#define LED_BLINK_INTERVAL_MS  500    /* 每 500ms 翻转一次     */
#define LED_BLINK_DURATION_MS  30000  /* 闪烁持续 30 秒后自动停 */

/* 当前 LED 状态，供 OLED 显示使用 */
typedef struct {
    LED_Mode_t mode;        /* 当前模式：灭 / 亮 / 闪烁 */
    uint8_t    led1_on;     /* LED1 实际状态：1=亮 0=灭 */
    uint8_t    led2_on;     /* LED2 实际状态：1=亮 0=灭 */
    uint32_t   remain_sec;  /* 闪烁剩余秒数，非闪烁时为 0 */
} LED_Status_t;

/* 当前 LED 模式，定义在 led.c，其它文件可以只读 */
extern LED_Mode_t led_mode;

/* ---------------- 对外函数 ---------------- */
void LED_Task(void);            /* 放在 while(1) 里反复调用：处理闪烁计时 */
void LED_Toggle_All(void);      /* 按键1：常亮/常灭 切换，并打断闪烁      */
void LED_Start_Blink(void);     /* 按键2：开始交替闪烁（重新计时 30 秒）  */
void LED_Force_Off(void);       /* 按键4：强制关闭所有 LED 并清除闪烁     */
void LED_GetStatus(LED_Status_t *status);  /* 获取当前状态快照           */

#endif
