#ifndef __APP_H
#define __APP_H

/*
 * 应用层公共头文件
 * 依赖 CubeMX 生成的 main.h（里面有 HAL 库、huart1 等）
 */
#include "main.h"
#include <stdio.h>

/* ---------------- 状态机 ---------------- */
typedef enum {
    MENU_MAIN = 0,   /* 主菜单     */
    MENU_LED,        /* 子菜单：LED控制  */
    MENU_INFO,       /* 子菜单：信息显示 */
    MODE_LINE,       /* 子菜单：巡线功能 */
    MENU_EXT         /* 子菜单：拓展功能 */
} System_State_t;

/* ---------------- 菜单 ---------------- */
#define MENU_ITEMS 4

extern System_State_t  sys_state;      /* 当前状态（定义在 app.c） */
extern uint8_t         menu_cursor;    /* 光标位置 0~3（定义在 app.c） */
extern const char     *menu_names[MENU_ITEMS];  /* 四个菜单项名称 */

/* 用串口打印当前界面，替代 OLED 刷新 */
void Refresh_Display(void);

#endif
