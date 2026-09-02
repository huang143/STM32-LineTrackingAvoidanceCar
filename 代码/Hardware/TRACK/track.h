#ifndef __TRACK_H
#define __TRACK_H

#include "sys.h"

/**************************************************************************
八路 IO 巡线模块
模块输出：X1~X8，左到右排列。
当前接线：X1->PA0，X2->PA1，X3->PA2，X4->PA3，X5->PA4，X6->PA5，X7->PB0，X8->PB1。
引脚模式：普通 GPIO 输入，上拉。
**************************************************************************/

#define TRACK_BLACK_LEVEL  0
#define TRACK_WHITE_LEVEL  1

void Track_Init(void);
u8 Track_ReadRaw(void);
u8 Track_ReadBlackMask(void);
u8 Track_GetActiveCount(void);
int Track_GetError(void);
int Track_GetLastError(void);

#endif
