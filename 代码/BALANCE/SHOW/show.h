#ifndef __SHOW_H
#define __SHOW_H
#include "sys.h"
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
void oled_show(void);
void APP_Show(void);
void DataScope(void);
void OLED_DrawPoint_Shu(u8 x,u8 y,u8 t);
void OLED_Show_CCD(void);
void oled_show_once(void);
#endif
