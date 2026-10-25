#ifndef __OLED_H
#define __OLED_H

#include "app.h"

/*
 * 0.96 寸 SSD1306 OLED（128x64，4 针 I2C）
 * 软件模拟 I2C：SCL = PB6，SDA = PB7
 */

#define OLED_WIDTH   128
#define OLED_HEIGHT  64

/* 基本驱动 */
void OLED_Init(void);                                   /* 上电初始化          */
void OLED_Clear(void);                                  /* 全屏清空（全黑）    */
void OLED_ShowChar(uint8_t x, uint8_t y, char ch);      /* 显示一个字符        */
void OLED_ShowString(uint8_t x, uint8_t y, const char *str);  /* 显示字符串    */
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len); /* 显示数字 */

/*
 * 坐标说明：
 *   x：0~127，横向像素位置
 *   y：0~7，纵向"页"编号（每页 8 像素高），y=0 是最上面一行
 *   每个字符宽 6 像素、高 8 像素
 */

#endif
