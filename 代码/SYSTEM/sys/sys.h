#ifndef __SYS_H
#define __SYS_H	
#include "stm32f1xx_hal.h"
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */	 

// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define SYSTEM_SUPPORT_OS		0		// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
										typedef int32_t  s32;
typedef int16_t s16;
typedef int8_t  s8;
 
typedef const int32_t sc32;
typedef const int16_t sc16;
typedef const int8_t sc8;
 
typedef __IO int32_t  vs32;
typedef __IO int16_t  vs16;
typedef __IO int8_t   vs8;
 
typedef __I int32_t vsc32;
typedef __I int16_t vsc16;
typedef __I int8_t vsc8;
 
typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;
 
typedef const uint32_t uc32;
typedef const uint16_t uc16;
typedef const uint8_t uc8;
 
typedef __IO uint32_t  vu32;
typedef __IO uint16_t vu16;
typedef __IO uint8_t  vu8;
 
typedef __I uint32_t vuc32;
typedef __I uint16_t vuc16;
typedef __I uint8_t vuc8;									    
	 
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 
 
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
/////////////////////////////////////////////////////////////////////////////
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define GPIO_A 0
#define GPIO_B 1
#define GPIO_C 2
#define GPIO_D 3
#define GPIO_E 4
#define GPIO_F 5
#define GPIO_G 6 
#define SERVO_INIT 1500
#define FTIR   1  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define RTIR   2  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。

#include "delay.h"
#include  "gpio.h"
#include "led.h"
#include "adc.h"
#include "encoder.h"
#include "ele_ccd.h"
//#include "tim.h"
//#include "exti.h"
#include "key.h"
//#include "moto.h"
#include "oled.h"
#include "stmflash.h"
#include "usart.h"
#include "control.h"
#include "show.h"
#include "DataScope_DP.h"
#include "usart3.h"
#include "pstwo.h"
#include "DataScope_DP.h"
#include "filter.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "mpu6050.h"
#include "dmpKey.h"
#include "dmpmap.h"
//#include "timer.h"
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define JTAG_SWD_DISABLE   0X02
#define SWD_ENABLE         0X01
#define JTAG_SWD_ENABLE    0X00	   

// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define    Diff_Car    						  	0				
#define    Akm_Car 							 	1
#define    Small_Tank_Car				  		0

extern u8 Mode,BLUETOOTH_Mode,ELE_Line_Patrol_Mode,CCD_Line_Patrol_Mode,PS2_Mode;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
extern int Flag_Direction; // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
extern u8 Flag_Way,Flag_Show,Flag_Stop,Flag_Next;   // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
extern int Encoder_Left,Encoder_Right;             // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
extern float Velocity,Velocity_Set,Angle,Angle_Set,Turn,PS2_Velocity,PS2_Turn_Velocity,Bluetooth_Velocity;
extern int Motor_A,Motor_B,Servo,Target_A,Target_B;  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
extern int Voltage;                                // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
extern float Show_Data_Mb;                         // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
extern u8 Delay_50,Delay_Flag; // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
extern float Velocity_KP,Velocity_KI;	       // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
extern int PS2_LX,PS2_LY,PS2_RX,PS2_RY,PS2_KEY;
extern u16 ADV[128];
extern u8 CCD_Zhongzhi,CCD_Yuzhi,PID_Send,Flash_Send,APP_RX,APP_Flag,PS2_Flag;     // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
extern int Sensor_Left,Sensor_Middle,Sensor_Right,Sensor;
extern u16 PID_Parameter[10],Flash_Parameter[10];

/////////////////////////////////////////////////////////////////  
void Stm32_Clock_Init(u8 PLL);  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void Sys_Soft_Reset(void);      // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void Sys_Standby(void);         // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void MY_NVIC_SetVectorTable(u32 NVIC_VectTab, u32 Offset);// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void MY_NVIC_PriorityGroupConfig(u8 NVIC_Group);// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void MY_NVIC_Init(u8 NVIC_PreemptionPriority,u8 NVIC_SubPriority,u8 NVIC_Channel,u8 NVIC_Group);// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void Ex_NVIC_Config(u8 GPIOx,u8 BITx,u8 TRIM);// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void JTAG_Set(u8 mode);
//////////////////////////////////////////////////////////////////////////////
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void WFI_SET(void);		// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void INTX_DISABLE(void);// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void INTX_ENABLE(void);	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void MSR_MSP(u32 addr);	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#include <string.h> 
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#endif
