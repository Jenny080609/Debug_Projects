#ifndef __KEY_H
#define __KEY_H

#include "app.h"   /* 间接包含 main.h 和 stdint */

/*
 * 按键扫描（带 20ms 软件消抖）
 * 返回值：1=按键1  2=按键2  3=按键3  4=按键4  0=没有按键按下
 */
uint8_t Key_Scan(void);

#endif
