#include "app.h"
#include "oled.h"

/* ================= 全局变量定义 ================= */
System_State_t sys_state   = MENU_MAIN;   /* 上电默认进入主菜单 */
uint8_t        menu_cursor = 0;           /* 光标默认在第 0 行   */

/*
 * 四个菜单项，顺序必须和 menu_cursor 的 0~3 对应。
 * 注意：SSD1306 这里用的是 6x8 的 ASCII 字库，无法显示中文，
 *       所以菜单项用英文；以后想显示中文需要另外加中文点阵字库。
 */
const char *menu_names[MENU_ITEMS] = {
    "LED Control",
    "Information",
    "Line Follow",
    "Extension"
};

/* 拿到当前子菜单的名字，只给本文件内部用 */
static const char *Get_State_Name(System_State_t state)
{
    switch (state)
    {
        case MENU_LED:  return "LED Control";
        case MENU_INFO: return "Information";
        case MODE_LINE: return "Line Follow";
        case MENU_EXT:  return "Extension";
        default:        return "Main Menu";
    }
}

/* ================= OLED 刷新界面（替代原来的串口打印） ================= */
void Refresh_Display(void)
{
    uint8_t i;

    OLED_Clear();          /* 先清屏，再重新画 */

    if (sys_state == MENU_MAIN)
    {
        /* 第 0 页：标题 */
        OLED_ShowString(24, 0, "==== MENU ====");

        /* 第 2~5 页：四个菜单项，光标行前面显示 "->" */
        for (i = 0; i < MENU_ITEMS; i++)
        {
            if (i == menu_cursor)
                OLED_ShowString(0, i + 2, "->");   /* 当前选中 */
            else
                OLED_ShowString(0, i + 2, "  ");   /* 未选中   */

            OLED_ShowString(18, i + 2, menu_names[i]);
        }

        /* 第 7 页：操作提示 */
        OLED_ShowString(0, 7, "1/2 Move 3 OK");
    }
    else
    {
        /* 子菜单：标题 + 子菜单名称 + 返回提示 */
        OLED_ShowString(0, 0, "==== SUBMENU ====");
        OLED_ShowString(0, 2, Get_State_Name(sys_state));
        OLED_ShowString(0, 4, "4: Back to Menu");
    }
}
