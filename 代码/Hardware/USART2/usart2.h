#ifndef __USRAT2_H
#define __USRAT2_H 
#include "sys.h"	  	

extern u8 Usart2_Receive;
extern u8 Usart2_Receive_buf[1];// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void uart2_init(u32 bound);

#endif

