#ifndef __STMFLASH_H__
#define __STMFLASH_H__
#include "sys.h"  


//////////////////////////////////////////////////////////////////////////////////////////////////////
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define STM32_FLASH_SIZE 	64 	 			// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define STM32_FLASH_WREN 	1              		// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
//////////////////////////////////////////////////////////////////////////////////////////////////////

// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define STM32_FLASH_BASE 0x08000000 			// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#define FLASH_Key1               0X45670123
#define FLASH_Key2               0XCDEF89AB

void STMFLASH_Unlock(void);					  	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void STMFLASH_Lock(void);					  	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
u8 STMFLASH_GetStatus(void);				  	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
u8 STMFLASH_WaitDone(u16 time);				  	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
u8 STMFLASH_ErasePage(u32 paddr);			  	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
u8 STMFLASH_WriteHalfWord(u32 faddr, u16 dat);	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
u16 STMFLASH_ReadHalfWord(u32 faddr);		  	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void STMFLASH_WriteLenByte(u32 WriteAddr,u32 DataToWrite,u16 Len);	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
u32 STMFLASH_ReadLenByte(u32 ReadAddr,u16 Len);						// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);		// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);   		// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。

// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void Test_Write(u32 WriteAddr,u16 WriteData);			

void Flash_Read(void);
void Flash_Write(void);
#endif

















