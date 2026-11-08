#include "motor.h"

/* htim3 定义在 main.c，这里用 extern 声明 */
extern TIM_HandleTypeDef htim3;

/* ================= TB6612 引脚定义 =================
 * A 路（左轮）：PWMA=PA6(TIM3_CH1)，AIN1=PB12，AIN2=PB13
 * B 路（右轮）：PWMB=PA7(TIM3_CH2)，BIN1=PB14，BIN2=PB15
 * ==================================================== */
#define AIN1_PORT   GPIOB
#define AIN1_PIN    GPIO_PIN_12
#define AIN2_PORT   GPIOB
#define AIN2_PIN    GPIO_PIN_13

#define BIN1_PORT   GPIOB
#define BIN1_PIN    GPIO_PIN_14
#define BIN2_PORT   GPIOB
#define BIN2_PIN    GPIO_PIN_15

/* 把 -100~100 的速度值限制在合法范围 */
static int limit_speed(int s)
{
    if (s >  MOTOR_SPEED_MAX) return  MOTOR_SPEED_MAX;
    if (s < -MOTOR_SPEED_MAX) return -MOTOR_SPEED_MAX;
    return s;
}

/* 单路电机输出：speed > 0 正转，speed < 0 反转，0 停止
 * 注意 TIM3 的 ARR = 99，所以占空比直接就是 0~100
 */
static void set_one_motor(GPIO_TypeDef *in1_port, uint16_t in1_pin,
                          GPIO_TypeDef *in2_port, uint16_t in2_pin,
                          TIM_HandleTypeDef *htim, uint32_t channel,
                          int speed)
{
    speed = limit_speed(speed);

    if (speed > 0)                          /* 正转 */
    {
        HAL_GPIO_WritePin(in1_port, in1_pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(in2_port, in2_pin, GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(htim, channel, (uint32_t)speed);
    }
    else if (speed < 0)                     /* 反转 */
    {
        HAL_GPIO_WritePin(in1_port, in1_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(in2_port, in2_pin, GPIO_PIN_SET);
        __HAL_TIM_SET_COMPARE(htim, channel, (uint32_t)(-speed));
    }
    else                                    /* 停止：方向全 0，占空比 0 */
    {
        HAL_GPIO_WritePin(in1_port, in1_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(in2_port, in2_pin, GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(htim, channel, 0);
    }
}

/* ================= 初始化 ================= */
void Motor_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* PB12~PB15：方向控制，推挽输出 */
    GPIO_InitStruct.Pin  = AIN1_PIN | AIN2_PIN | BIN1_PIN | BIN2_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* 启动两路 PWM（PA6/PA7 的复用配置由 MX_TIM3_Init 完成） */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

    Motor_Stop();
}

/* ================= 直接设置左右轮速度 ================= */
void Motor_Set(int left, int right)
{
    set_one_motor(AIN1_PORT, AIN1_PIN, AIN2_PORT, AIN2_PIN,
                  &htim3, TIM_CHANNEL_1, left);    /* A 路 = 左轮 */
    set_one_motor(BIN1_PORT, BIN1_PIN, BIN2_PORT, BIN2_PIN,
                  &htim3, TIM_CHANNEL_2, right);   /* B 路 = 右轮 */
}

/* ================= 常用动作 ================= */
void Motor_Forward(void)
{
    Motor_Set(MOTOR_SPEED_BASE, MOTOR_SPEED_BASE);
}

void Motor_Backward(void)
{
    Motor_Set(-MOTOR_SPEED_BASE, -MOTOR_SPEED_BASE);
}

/* 急左转：左轮反转（减速），右轮正转（全速） */
void Motor_TurnLeft(void)
{
    Motor_Set(MOTOR_SPEED_TURN_INNER, MOTOR_SPEED_BASE);
}

/* 急右转：左轮正转（全速），右轮反转（减速） */
void Motor_TurnRight(void)
{
    Motor_Set(MOTOR_SPEED_BASE, MOTOR_SPEED_TURN_INNER);
}

void Motor_Stop(void)
{
    Motor_Set(0, 0);
}
