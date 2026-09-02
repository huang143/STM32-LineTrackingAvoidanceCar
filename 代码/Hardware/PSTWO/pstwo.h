#ifndef __PSTWO_H
#define __PSTWO_H
#include "delay.h"
#include "sys.h"
/*********************************************************
      
**********************************************************/	 
#define DI   PCin(13)          	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。

#define DO_H PCout(14)=1        	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define DO_L PCout(14)=0        	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。

#define CS_H PCout(15)=1       	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define CS_L PCout(15)=0       	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。

#define CLK_H PAout(2)=1      	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define CLK_L PAout(2)=0      	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。



//These are our button constants
#define PSB_SELECT      1
#define PSB_L3          2
#define PSB_R3          3
#define PSB_START       4
#define PSB_PAD_UP      5
#define PSB_PAD_RIGHT   6
#define PSB_PAD_DOWN    7
#define PSB_PAD_LEFT    8
#define PSB_L2          9
#define PSB_R2          10
#define PSB_L1          11
#define PSB_R1          12
#define PSB_GREEN       13
#define PSB_RED         14
#define PSB_BLUE        15
#define PSB_PINK        16

#define PSB_TRIANGLE    13
#define PSB_CIRCLE      14
#define PSB_CROSS       15
#define PSB_SQUARE      16

//#define WHAMMY_BAR		8

//These are stick values
#define PSS_RX 5                // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define PSS_RY 6
#define PSS_LX 7
#define PSS_LY 8

extern u8 Data[9];
extern u16 MASK[16];
extern u16 Handkey;

void PS2_Init(void);
u8 PS2_RedLight(void);   // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void PS2_ReadData(void); // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void PS2_Cmd(u8 CMD);		  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
u8 PS2_DataKey(void);		  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
u8 PS2_AnologData(u8 button); // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void PS2_ClearData(void);	  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void PS2_Vibration(u8 motor1, u8 motor2);// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。

void PS2_EnterConfing(void);	 // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void PS2_TurnOnAnalogMode(void); // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void PS2_VibrationMode(void);    // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void PS2_ExitConfing(void);	     // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void PS2_SetInit(void);		     // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void PS2_Read(void);
#endif

