#ifndef __BEEP_H
#define __BEEP_H

#include "app.h"

void Beep_Init(void);        /* 上电初始化（默认不响） */
void Beep_Once(void);        /* 响一声，约 100ms */
void Beep_On(void);
void Beep_Off(void);

#endif
