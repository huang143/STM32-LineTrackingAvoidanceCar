#include "ele_ccd.h"
#define TSL_SI    PBout(1)   //SI  
#define TSL_CLK   PAout(1)   //CLK 
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
void Dly_us(void)
{
   int ii;    
   for(ii=0;ii<10;ii++);      
}
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
void RD_TSL(void) 
{
	u8 i=0,tslp=0;
  TSL_CLK=0;
  TSL_SI=0; 
  Dly_us();
      
  TSL_SI=1; 
  Dly_us();
	
  TSL_CLK=0;
  Dly_us();
      
  TSL_CLK=1;
  Dly_us();
	
  TSL_SI=0;
  Dly_us(); 
  for(i=0;i<128;i++)
  { 
    TSL_CLK=0; 
    Dly_us();  // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
    ADV[tslp]=(Get_Adc(2))>>4;
    ++tslp;
    TSL_CLK=1;
     Dly_us();
	}
}

/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */	 
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
u16 Get_Adc(u8 ch)   
{
	u16 result;
	ADC_ChannelConfTypeDef sConfig;// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	sConfig.Channel=ch;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;		// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	sConfig.Rank = 1;
	HAL_ADC_ConfigChannel(&hadc1,&sConfig);		
	
	HAL_ADC_Start(&hadc1);								// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	HAL_ADC_PollForConversion(&hadc1,30);				// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	if(HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc1), HAL_ADC_STATE_REG_EOC))
	{
		result=HAL_ADC_GetValue(&hadc1);	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	}
	return result;
}

///**************************************************************************
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
//**************************************************************************/
//void CCD(void)   
//{  
//	int i,j;
//	usart1_send(0xff);			
//   for(i=0; i<100;i++) 
//  { 
//    RD_TSL();           
//   	j=0;
//	for(j=0;j<128;j++)
//	{
//      if(ADV[j] ==0XFF)  ADV[j]--;    
//     usart1_send(ADV[j]);
//    }   
//  }
//}
/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */	 	
void CCD_ADC_Mode_Config(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */
  HAL_ADCEx_Calibration_Start(&hadc1); // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。

}

/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */	 	
void CCD_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_AFIO_CLK_ENABLE();
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	
	// 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
	GPIO_InitStruct.Pin = GPIO_PIN_1;				//PA1
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);	


	GPIO_InitStruct.Pin = GPIO_PIN_1;				//PB1
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);		
	
}


/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */	 	
void CCD_Init(void)
{
	CCD_GPIO_Config();
//	CCD_ADC_Mode_Config();

}

/*
 * 注释已整理：原厂注释编码异常，已移除乱码内容。
 * 本次仅修改注释，不改变任何代码逻辑。
 */
void  ELE_Init(void)
{    
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  
   sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
	
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */
  HAL_ADCEx_Calibration_Start(&hadc1); // 注释已整理：原注释编码异常，避免 UTF-8 打开时乱码。
}		


