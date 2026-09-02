#include "delay.h"
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#endif

////////////////////////////////////////////////////////////////////////////////// 	 

static u8  fac_us=0;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
static u16 fac_ms=0;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。

#ifdef OS_CRITICAL_METHOD 	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void SysTick_Handler(void)
{				   
	OSIntEnter();		// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
    OSTimeTick();       // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
    OSIntExit();        // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
}
#endif
			   
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void delay_init(u8 SYSCLK)
{
#ifdef OS_CRITICAL_METHOD 	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	u32 reload;
#endif
 	SysTick->CTRL&=~(1<<2);	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	fac_us=SYSCLK/8;		// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	    
#ifdef OS_CRITICAL_METHOD 	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	reload=SYSCLK/8;		// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	reload*=1000000/OS_TICKS_PER_SEC;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
							// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	fac_ms=1000/OS_TICKS_PER_SEC;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	SysTick->CTRL|=1<<1;   	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	SysTick->LOAD=reload; 	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	SysTick->CTRL|=1<<0;   	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#else
	fac_ms=(u16)fac_us*1000;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#endif
}								    

#ifdef OS_CRITICAL_METHOD 	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void delay_us(u32 nus)
{		
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	ticks=nus*fac_us; 			// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	tcnt=0;
	OSSchedLock();				// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	told=SysTick->VAL;        	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		}  
	};
	OSSchedUnlock();			// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
}
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void delay_ms(u16 nms)
{	
	if(OSRunning==OS_TRUE)// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	{		  
		if(nms>=fac_ms)// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		{
   			OSTimeDly(nms/fac_ms);// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		}
		nms%=fac_ms;			// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	}
	delay_us((u32)(nms*1000));	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
}
#else// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	SysTick->VAL=0x00;        // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	SysTick->CTRL=0x01 ;      // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	do
	{
		temp=SysTick->CTRL;
	}
	while((temp&0x01)&&!(temp&(1<<16)));// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	SysTick->CTRL=0x00;       // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	SysTick->VAL =0X00;       // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
}
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
//nms<=0xffffff*8*1000/SYSCLK
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void delay_ms(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	SysTick->VAL =0x00;           // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	SysTick->CTRL=0x01 ;          // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	do
	{
		temp=SysTick->CTRL;
	}
	while((temp&0x01)&&!(temp&(1<<16)));// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	SysTick->CTRL=0x00;       // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	SysTick->VAL =0X00;       // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
} 
#endif
			 



































