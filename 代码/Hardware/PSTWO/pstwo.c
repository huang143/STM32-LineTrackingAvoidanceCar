#include "pstwo.h"
/*********************************************************     
**********************************************************/	 
#define DELAY_TIME  delay_us(5); 
u16 Handkey;	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
u8 Comd[2]={0x01,0x42};	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
u8 Data[9]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
u16 MASK[]={
    PSB_SELECT,
    PSB_L3,
    PSB_R3 ,
    PSB_START,
    PSB_PAD_UP,
    PSB_PAD_RIGHT,
    PSB_PAD_DOWN,
    PSB_PAD_LEFT,
    PSB_L2,
    PSB_R2,
    PSB_L1,
    PSB_R1 ,
    PSB_GREEN,
    PSB_RED,
    PSB_BLUE,
    PSB_PINK
	};	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。


void PS2_Init(void)
{
   GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitStruct.Pin=GPIO_PIN_13;
  GPIO_InitStruct.Mode=GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull= GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC,&GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	PWR->CR|=1<<8;	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	RCC->BDCR&=0xFFFFFFFE;	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	BKP->CR&=0xFFFFFFFE; 	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	PWR->CR&=0xFFFFFEFF;	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
}

// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void PS2_Cmd(u8 CMD)
{
	volatile u16 ref=0x01;
	Data[1] = 0;
	for(ref=0x01;ref<0x0100;ref<<=1)
	{
		if(ref&CMD)
		{
			DO_H;                   // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		}
		else DO_L;

		CLK_H;                        // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		DELAY_TIME;
		CLK_L;
		DELAY_TIME;
		CLK_H;
		if(DI)
			Data[1] = ref|Data[1];
	}
	delay_us(16);
}
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
u8 PS2_RedLight(void)
{
	CS_L;
	PS2_Cmd(Comd[0]);  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	PS2_Cmd(Comd[1]);  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	CS_H;
	if( Data[1] == 0X73)   return 0 ;
	else return 1;

}
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void PS2_ReadData(void)
{
	volatile u8 byte=0;
	volatile u16 ref=0x01;
	CS_L;
	PS2_Cmd(Comd[0]);  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	PS2_Cmd(Comd[1]);  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	for(byte=2;byte<9;byte++)          // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	{
		for(ref=0x01;ref<0x100;ref<<=1)
		{
			CLK_H;
			DELAY_TIME;
			CLK_L;
			DELAY_TIME;
			CLK_H;
		      if(DI)
		      Data[byte] = ref|Data[byte];
		}
        delay_us(16);
	}
	CS_H;
}

// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
u8 PS2_DataKey()
{
	u8 index;

	PS2_ClearData();
	PS2_ReadData();

	Handkey=(Data[4]<<8)|Data[3];     // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	for(index=0;index<16;index++)
	{	    
		if((Handkey&(1<<(MASK[index]-1)))==0)
		return index+1;
	}
	return 0;          // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
}

// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
u8 PS2_AnologData(u8 button)
{
	return Data[button];
}

// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void PS2_ClearData()
{
	u8 a;
	for(a=0;a<9;a++)
		Data[a]=0x00;
}
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
void PS2_Vibration(u8 motor1, u8 motor2)
{
	CS_L;
	delay_us(16);
    PS2_Cmd(0x01);  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	PS2_Cmd(0x42);  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	PS2_Cmd(0X00);
	PS2_Cmd(motor1);
	PS2_Cmd(motor2);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	CS_H;
	delay_us(16);  
}
//short poll
void PS2_ShortPoll(void)
{
	CS_L;
	delay_us(16);
	PS2_Cmd(0x01);  
	PS2_Cmd(0x42);  
	PS2_Cmd(0X00);
	PS2_Cmd(0x00);
	PS2_Cmd(0x00);
	CS_H;
	delay_us(16);	
}
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void PS2_EnterConfing(void)
{
    CS_L;
	delay_us(16);
	PS2_Cmd(0x01);  
	PS2_Cmd(0x43);  
	PS2_Cmd(0X00);
	PS2_Cmd(0x01);
	PS2_Cmd(0x00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	CS_H;
	delay_us(16);
}
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void PS2_TurnOnAnalogMode(void)
{
	CS_L;
	PS2_Cmd(0x01);  
	PS2_Cmd(0x44);  
	PS2_Cmd(0X00);
	PS2_Cmd(0x01); // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	PS2_Cmd(0x03); // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
				   // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	CS_H;
	delay_us(16);
}
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void PS2_VibrationMode(void)
{
	CS_L;
	delay_us(16);
	PS2_Cmd(0x01);  
	PS2_Cmd(0x4D);  
	PS2_Cmd(0X00);
	PS2_Cmd(0x00);
	PS2_Cmd(0X01);
	CS_H;
	delay_us(16);	
}
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void PS2_ExitConfing(void)
{
    CS_L;
	delay_us(16);
	PS2_Cmd(0x01);  
	PS2_Cmd(0x43);  
	PS2_Cmd(0X00);
	PS2_Cmd(0x00);
	PS2_Cmd(0x5A);
	PS2_Cmd(0x5A);
	PS2_Cmd(0x5A);
	PS2_Cmd(0x5A);
	PS2_Cmd(0x5A);
	CS_H;
	delay_us(16);
}
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
void PS2_SetInit(void)
{
	PS2_ShortPoll();
	PS2_ShortPoll();
	PS2_ShortPoll();
	PS2_EnterConfing();		// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	PS2_TurnOnAnalogMode();	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	PS2_ExitConfing();		// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
}

/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
void PS2_Read(void)
{
	static int Strat;
	//Reading key
  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	PS2_KEY=PS2_DataKey(); 
	//Read the analog of the remote sensing x axis on the left
  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	PS2_LX=PS2_AnologData(PSS_LX); 
	//Read the analog of the directional direction of remote sensing on the left
  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	PS2_LY=PS2_AnologData(PSS_LY);
	//Read the analog of the remote sensing x axis
  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	PS2_RX=PS2_AnologData(PSS_RX);
	//Read the analog of the directional direction of the remote sensing y axis
  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	PS2_RY=PS2_AnologData(PSS_RY);  

	if(PS2_KEY==4&&PS2_Mode==1) 
		//The start button on the // handle is pressed
		// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
		Strat=1; 
	//When the button is pressed, you need to push the right side forward to the formal ps2 control car
	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	if(Strat&&(PS2_LY<118)&&PS2_Mode==1)
	{
		PS2_Flag=1;
	}
	
}
















