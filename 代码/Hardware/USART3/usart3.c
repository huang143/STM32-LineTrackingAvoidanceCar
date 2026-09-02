#include "usart3.h"
#include "camera.h"
u8 Usart3_Receive;
u8 Usart3_Receive_buf[1];// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle) // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
{	
	if(UartHandle->Instance == USART1)
	{
		Camera_UartRxCpltCallback(UartHandle);
		return;
	}

	if(UartHandle->Instance == USART3)// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	{	  
		static u8 Flag_PID,i,j,Receive[50];
		static float Data;
  	    Usart3_Receive=Usart3_Receive_buf[0]; 
		APP_RX=Usart3_Receive;
		if(Usart3_Receive>=0x41&&Usart3_Receive<=0x48)  
		  Flag_Direction=Usart3_Receive-0x40;
		else if(Usart3_Receive<10)  
		  Flag_Direction=Usart3_Receive;	
		else if(Usart3_Receive==0X5A)  
		  Flag_Direction=0;	
		else if(Usart3_Receive==0X59)
			Bluetooth_Velocity -=10;
		else if(Usart3_Receive==0x58)
			Bluetooth_Velocity +=10; 
		Bluetooth_Velocity = limit(Bluetooth_Velocity,0,60);
			
				// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		if(Usart3_Receive==0x7B) Flag_PID=1;   // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		if(Usart3_Receive==0x7D) Flag_PID=2;   // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。

		 if(Flag_PID==1)  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		 {
			Receive[i]=Usart3_Receive;
			i++;
		 }
		 if(Flag_PID==2)  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		 {
			 if(Receive[3]==0x50) 	 PID_Send=1;
			 else  if(Receive[3]==0x57) 	 Flash_Send=1;
			 else  if(Receive[1]!=0x23) 
			 {								
				for(j=i;j>=4;j--)
				{
				  Data+=(Receive[j-1]-48)*pow(10,i-j);
				}
				switch(Receive[1])
				 {
					 case 0x30:  break;
					 case 0x31:  Velocity_KP=Data;break;
					 case 0x32:  Velocity_KI=Data;break;
					 case 0x33:  break;
					 case 0x34:  break;
					 case 0x35:  break;
					 case 0x36:  break;
					 case 0x37:  break; // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
					 case 0x38:  break; // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
				 }
				}				 
			 Flag_PID=0;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
			 i=0;
			 j=0;
			 Data=0;
			 memset(Receive, 0, sizeof(u8)*50);// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		 } 	 	
      HAL_UART_Receive_IT(&huart3,Usart3_Receive_buf,sizeof(Usart3_Receive_buf));// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	}  											 
} 


