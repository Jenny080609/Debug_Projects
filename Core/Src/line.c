#include "line.h"

/* ================= 5 路传感器引脚定义 =================
 * OUT1 -> PB0
 * OUT2 -> PB1
 * OUT3 -> PB5
 * OUT4 -> PB3
 * OUT5 -> PB4
 * ====================================================== */
#define S1_PORT   GPIOB
#define S1_PIN    GPIO_PIN_0

#define S2_PORT   GPIOB
#define S2_PIN    GPIO_PIN_1

#define S3_PORT   GPIOB
#define S3_PIN    GPIO_PIN_5

#define S4_PORT   GPIOB
#define S4_PIN    GPIO_PIN_3

#define S5_PORT   GPIOB
#define S5_PIN    GPIO_PIN_4

/* 保存 5 位状态 + 结束符 */
static char line_state[LINE_SENSOR_NUM + 1];

/* ==================================================================
 * 初始化：GPIOB 时钟 + 关闭 JTAG（PB3/PB4 复位后是 JTAG 引脚）
 * ================================================================== */
void Line_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();

    /* PB3=JTDO、PB4=NJTRST 默认被 JTAG 占用
     * 关闭 JTAG、保留 SWD（PA13/PA14），才能把 PB3/PB4 当普通输入使用
     */
    __HAL_AFIO_REMAP_SWJ_NOJTAG();

    GPIO_InitStruct.Pin  = S1_PIN | S2_PIN | S3_PIN | S4_PIN | S5_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;          /* 上拉输入 */
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/* ==================================================================
 * 读取 5 路传感器
 * 低电平(RESET) = 压黑线 = '0'
 * 高电平(SET)   = 白底   = '1'
 * ================================================================== */
const char *Line_ReadSensors(void)
{
    line_state[0] = (HAL_GPIO_ReadPin(S1_PORT, S1_PIN) == GPIO_PIN_RESET) ? '0' : '1';
    line_state[1] = (HAL_GPIO_ReadPin(S2_PORT, S2_PIN) == GPIO_PIN_RESET) ? '0' : '1';
    line_state[2] = (HAL_GPIO_ReadPin(S3_PORT, S3_PIN) == GPIO_PIN_RESET) ? '0' : '1';
    line_state[3] = (HAL_GPIO_ReadPin(S4_PORT, S4_PIN) == GPIO_PIN_RESET) ? '0' : '1';
    line_state[4] = (HAL_GPIO_ReadPin(S5_PORT, S5_PIN) == GPIO_PIN_RESET) ? '0' : '1';

    line_state[LINE_SENSOR_NUM] = '\0';          /* 字符串结束符 */

    return line_state;
}

/* 5 路全部压黑线（"00000"）：通常是一条横向起跑线 / 十字标记 */
uint8_t Line_IsAllBlack(void)
{
    uint8_t i;
    const char *s = Line_ReadSensors();

    for (i = 0; i < LINE_SENSOR_NUM; i++)
    {
        if (s[i] != '0') return 0;
    }
    return 1;
}

/* 5 路全部在白底（"11111"）：脱线 */
uint8_t Line_IsAllWhite(void)
{
    uint8_t i;
    const char *s = Line_ReadSensors();

    for (i = 0; i < LINE_SENSOR_NUM; i++)
    {
        if (s[i] != '1') return 0;
    }
    return 1;
}
