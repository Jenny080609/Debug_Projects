#include "beep.h"

/* ================= 蜂鸣器定义 =================
 * 蜂鸣器接 PB8
 * 低电平触发：RESET(0) = 响，SET(1) = 不响
 * ============================================== */
#define BEEP_PORT   GPIOB
#define BEEP_PIN    GPIO_PIN_8

/* 初始化：默认不响（输出高电平） */
void Beep_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* 先写 ODR 为高电平，再配置成输出，避免配置瞬间输出低电平把蜂鸣器拉响 */
    HAL_GPIO_WritePin(BEEP_PORT, BEEP_PIN, GPIO_PIN_SET);

    GPIO_InitStruct.Pin   = BEEP_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BEEP_PORT, &GPIO_InitStruct);

    Beep_Off();                 /* 再次确认不响 */
}

/* 响：低电平 */
void Beep_On(void)
{
    HAL_GPIO_WritePin(BEEP_PORT, BEEP_PIN, GPIO_PIN_RESET);
}

/* 不响：高电平 */
void Beep_Off(void)
{
    HAL_GPIO_WritePin(BEEP_PORT, BEEP_PIN, GPIO_PIN_SET);
}

/* 响一声：响 100ms 后恢复不响。开始巡线时调用，提醒计时开始 */
void Beep_Once(void)
{
    Beep_On();
    HAL_Delay(100);
    Beep_Off();
}
