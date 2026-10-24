#include "app.h"

/* ================= 全局变量定义 ================= */
System_State_t sys_state   = MENU_MAIN;   /* 上电默认进入主菜单 */
uint8_t        menu_cursor = 0;           /* 光标默认在第 0 行   */

/* 四个菜单项，顺序必须和 menu_cursor 的 0~3 对应 */
const char *menu_names[MENU_ITEMS] = {
    "LED控制",
    "信息显示",
    "巡线功能",
    "拓展功能"
};

/* 拿到当前子菜单的名字，只给本文件内部用 */
static const char *Get_State_Name(System_State_t state)
{
    switch (state)
    {
        case MENU_LED:  return "LED控制";
        case MENU_INFO: return "信息显示";
        case MODE_LINE: return "巡线功能";
        case MENU_EXT:  return "拓展功能";
        default:        return "主菜单";
    }
}

/* ================= 串口版"OLED刷新" ================= */
void Refresh_Display(void)
{
    uint8_t i;

    /* 1. 清屏：用换行把旧内容顶出可见区域。
       如果你的串口助手支持 ANSI 转义，可以改成：
       printf("\033[2J\033[H");  效果更干净            */
    for (i = 0; i < 30; i++)
        printf("\r\n");

    /* 2. 按当前状态画界面 */
    if (sys_state == MENU_MAIN)
    {
        printf("==== 主菜单 ====\r\n\r\n");

        /* 循环打印 4 行，当前光标行前面加 "-> "，其余加 "   " */
        for (i = 0; i < MENU_ITEMS; i++)
        {
            if (i == menu_cursor)
                printf("-> %s\r\n", menu_names[i]);
            else
                printf("   %s\r\n", menu_names[i]);
        }

        printf("\r\n1:上移  2:下移  3:进入\r\n");
    }
    else
    {
        /* 子菜单统一显示：标题 + 返回提示 */
        printf("==== %s ====\r\n\r\n", Get_State_Name(sys_state));
        printf("按 4 返回主菜单\r\n");
    }
}
