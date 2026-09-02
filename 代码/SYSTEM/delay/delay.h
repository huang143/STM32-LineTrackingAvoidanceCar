#ifndef __DELAY_H
#define __DELAY_H 			   
#include "sys.h"  
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
void delay_init(u8 SYSCLK);
void delay_ms(u16 nms);
void delay_us(u32 nus);

#endif





























