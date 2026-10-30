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

/* ---------------- 信息显示用的变量 ---------------- */
extern volatile int var_a;                      /* 全局变量 a，上电初始值 0 */

/* 重画整个界面（切换菜单时调用） */
void Refresh_Display(void);

/* 只刷新 LED 子菜单的实时状态（闪烁时周期调用） */
void Refresh_LED_Status(void);

#endif
