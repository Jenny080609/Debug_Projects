#ifndef __LINE_H
#define __LINE_H

#include "app.h"

#define LINE_SENSOR_NUM   5      /* 5 路循迹传感器 */

/* 初始化 5 路传感器引脚（PB0/PB1/PB5/PB3/PB4，上拉输入） */
void Line_Init(void);

/* 读取 5 路传感器，返回 "11111" 这样的 5 位字符串
 * 0 = 压黑线，1 = 白底
 */
const char *Line_ReadSensors(void);

/* 辅助判断：5 路全压黑线 / 5 路全在白底 */
uint8_t Line_IsAllBlack(void);
uint8_t Line_IsAllWhite(void);

#endif
