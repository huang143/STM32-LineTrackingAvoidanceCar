#include "stmflash.h"
#include "delay.h"

#define FLASH_SAVE_ADDR  0X0800E000 	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void STMFLASH_Unlock(void)
{
  FLASH->KEYR=FLASH_Key1;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
  FLASH->KEYR=FLASH_Key2;
}
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void STMFLASH_Lock(void)
{
  FLASH->CR|=1<<7;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
}
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
u8 STMFLASH_GetStatus(void)
{	
	u32 res;		
	res=FLASH->SR; 
	if(res&(1<<0))return 1;		    //æ
	else if(res&(1<<2))return 2;	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	else if(res&(1<<4))return 3;	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	return 0;						// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
}
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
u8 STMFLASH_WaitDone(u16 time)
{
	u8 res;
	do
	{
		res=STMFLASH_GetStatus();
		if(res!=1)break;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		delay_us(1);
		time--;
	 }while(time);
	 if(time==0)res=0xff;//TIMEOUT
	 return res;
}
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
u8 STMFLASH_ErasePage(u32 paddr)
{
	u8 res=0;
	res=STMFLASH_WaitDone(0X5FFF);// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	if(res==0)
	{ 
		FLASH->CR|=1<<1;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		FLASH->AR=paddr;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		FLASH->CR|=1<<6;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		res=STMFLASH_WaitDone(0X5FFF);// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		if(res!=1)// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		{
			FLASH->CR&=~(1<<1);// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		}
	}
	return res;
}
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
u8 STMFLASH_WriteHalfWord(u32 faddr, u16 dat)
{
	u8 res;	   	    
	res=STMFLASH_WaitDone(0XFF);	 
	if(res==0)//OK
	{
		FLASH->CR|=1<<0;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		*(vu16*)faddr=dat;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		res=STMFLASH_WaitDone(0XFF);// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		if(res!=1)// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		{
			FLASH->CR&=~(1<<0);// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		}
	} 
	return res;
} 
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr; 
}
#if STM32_FLASH_WREN	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)   
{ 			 		 
	u16 i;
	for(i=0;i<NumToWrite;i++)
	{
		STMFLASH_WriteHalfWord(WriteAddr,pBuffer[i]);
	    WriteAddr+=2;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	}  
} 
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
#else 
#define STM_SECTOR_SIZE	2048
#endif		 
u16 STMFLASH_BUF[STM_SECTOR_SIZE/2];// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)	
{
	u32 secpos;	   // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	u16 secoff;	   // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	u16 secremain; // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
 	u16 i;    
	u32 offaddr;   // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	STMFLASH_Unlock();						// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	offaddr=WriteAddr-STM32_FLASH_BASE;		// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	secpos=offaddr/STM_SECTOR_SIZE;			// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	secoff=(offaddr%STM_SECTOR_SIZE)/2;		// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	secremain=STM_SECTOR_SIZE/2-secoff;		// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	if(NumToWrite<=secremain)secremain=NumToWrite;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	while(1) 
	{	
		STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		for(i=0;i<secremain;i++)// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		{
			if(STMFLASH_BUF[secoff+i]!=0XFFFF)break;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		}
		if(i<secremain)// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		{
			STMFLASH_ErasePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
			for(i=0;i<secremain;i++)// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
			{
				STMFLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		}else STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		if(NumToWrite==secremain)break;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		else// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		{
			secpos++;				// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
			secoff=0;				// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		   	pBuffer+=secremain;  	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
			WriteAddr+=secremain*2;	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		   	NumToWrite-=secremain;	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
			if(NumToWrite>(STM_SECTOR_SIZE/2))secremain=STM_SECTOR_SIZE/2;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
			else secremain=NumToWrite;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		}	 
	};	
	STMFLASH_Lock();// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
}
#endif
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)   	
{
	u16 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		ReadAddr+=2;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	}
}

// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void Test_Write(u32 WriteAddr,u16 WriteData)   	
{
	STMFLASH_Write(WriteAddr,&WriteData,1);// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
}
 
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
void Flash_Read(void)
{
	STMFLASH_Read(FLASH_SAVE_ADDR,(u16*)PID_Parameter,10);
	if(PID_Parameter[0]==65535&&PID_Parameter[1]==65535&&PID_Parameter[2]==65535&&PID_Parameter[3]==65535)
	{
	Bluetooth_Velocity=30;
	Velocity_KP=15;
	Velocity_KI=7;
	}
  else
	{		
	Bluetooth_Velocity=(float)PID_Parameter[0];
	Velocity_KP=(float)PID_Parameter[1];
	Velocity_KI=(float)PID_Parameter[2];
	}
}	
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
void Flash_Write(void)
{
Flash_Parameter[0]=Bluetooth_Velocity;	
Flash_Parameter[1]=Velocity_KP;		
Flash_Parameter[2]=Velocity_KI;		
STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)Flash_Parameter,10);	
}	



