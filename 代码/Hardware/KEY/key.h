#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
#define KEY PAin(5)   

void KEY_Init(void);          // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
u8 click_N_Double (u8 time);  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
u8 click(void);               // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
u8 Long_Press(void);          // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
u8  select(void);             // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#endif 
