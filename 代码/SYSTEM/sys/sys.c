#include "sys.h"
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void MY_NVIC_SetVectorTable(u32 NVIC_VectTab, u32 Offset)	 
{ 	   	 
	SCB->VTOR = NVIC_VectTab|(Offset & (u32)0x1FFFFF80);// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
}



void WFI_SET(void)
{
	__ASM volatile("wfi");		  
}
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void INTX_DISABLE(void)
{		  
	__ASM volatile("cpsid i");
}
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void INTX_ENABLE(void)
{
	__ASM volatile("cpsie i");		  
}
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
__asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}


// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
//#define JTAG_SWD_DISABLE   0X02
//#define SWD_ENABLE         0X01
//#define JTAG_SWD_ENABLE    0X00		  
void JTAG_Set(u8 mode)
{
	u32 temp;
	temp=mode;
	temp<<=25;
	RCC->APB2ENR|=1<<0;     // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	AFIO->MAPR&=0XF8FFFFFF; // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	AFIO->MAPR|=temp;       // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
} 
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
