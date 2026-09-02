#ifndef __LED_H
#define __LED_H
#include "sys.h"
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define LED PAout(4) // PB13
void Led_Flash(u16 time);

#endif
