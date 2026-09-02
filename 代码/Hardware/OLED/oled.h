#ifndef __OLED_H
#define __OLED_H

#include "sys.h"

/**************************************************************************
0.96寸四针 I2C OLED 显示屏接口
屏幕四针：GND、VCC、SCL、SDA
当前接线：SCL -> PB10，SDA -> PB11
说明：PB10/PB11 在本工程中按普通开漏 GPIO 模拟 I2C 使用，未启用硬件 I2C 外设。
**************************************************************************/

#define OLED_CMD   0
#define OLED_DATA  1

void OLED_WR_Byte(u8 dat, u8 cmd);
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Refresh_Gram(void);
void OLED_Init(void);
void OLED_Clear(void);
void OLED_DrawPoint(u8 x, u8 y, u8 t);
void OLED_ShowChar(u8 x, u8 y, u8 chr, u8 size, u8 mode);
void OLED_ShowNumber(u8 x, u8 y, u32 num, u8 len, u8 size);
void OLED_ShowString(u8 x, u8 y, const u8 *p);

#endif
