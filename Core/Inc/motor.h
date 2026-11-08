#ifndef __MOTOR_H
#define __MOTOR_H

#include "app.h"

/* 速度范围：-100 ~ 100（负值反转，0 停止） */
#define MOTOR_SPEED_MAX       100
#define MOTOR_SPEED_BASE      50    /* 巡线基础速度（直行） */
#define MOTOR_SPEED_TURN_INNER (-20)/* 转弯内侧轮速度：负值反转，差速更大、转向更急 */

void Motor_Init(void);              /* 初始化方向引脚 + 启动 PWM */
void Motor_Set(int left, int right);/* 直接设置左右轮速度（带方向） */
void Motor_Forward(void);           /* 直行 */
void Motor_Backward(void);          /* 后退 */
void Motor_TurnLeft(void);          /* 急左转：左轮反转，右轮正转 */
void Motor_TurnRight(void);         /* 急右转：左轮正转，右轮反转 */
void Motor_Stop(void);              /* 停止（两轮断电） */

#endif
