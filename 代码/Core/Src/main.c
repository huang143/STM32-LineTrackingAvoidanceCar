/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * This file is modified for:
  * STM32F103C8T6 + TB6612FNG + MG310 encoder motors straight running test.
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sys.h"
#include "control.h"
#include "oled.h"
#include "track.h"
#include "ultrasonic.h"
#include "camera.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
u8 Mode = 0;
u8 BLUETOOTH_Mode = 0;
u8 ELE_Line_Patrol_Mode = 0;
u8 CCD_Line_Patrol_Mode = 0;
u8 PS2_Mode = 0;
u8 PS2_Flag = 0;
u8 APP_RX = 0;
u8 APP_Flag = 0;
u8 Flag_Way = 0;
u8 Flag_Show = 0;
u8 Flag_Stop = 0;
u8 Flag_Next = 0;
u8 Delay_50 = 0;
u8 Delay_Flag = 0;
u8 CCD_Zhongzhi = 0;
u8 CCD_Yuzhi = 0;
u8 PID_Send = 0;
u8 Flash_Send = 0;

u16 ADV[128] = {0};
u16 PID_Parameter[10] = {0};
u16 Flash_Parameter[10] = {0};

int Encoder_Left = 0;
int Encoder_Right = 0;
int Flag_Direction = 0;
int Motor_A = 0;
int Motor_B = 0;
int Servo = 0;
int Target_A = 0;
int Target_B = 0;
int Voltage = 1200;
int PS2_LX = 0;
int PS2_LY = 0;
int PS2_RX = 0;
int PS2_RY = 0;
int PS2_KEY = 0;
int Sensor_Left = 0;
int Sensor_Middle = 0;
int Sensor_Right = 0;
int Sensor = 0;

float Velocity = 0.0f;
float Velocity_Set = 0.0f;
float Angle = 0.0f;
float Angle_Set = 0.0f;
float Turn = 0.0f;
float Velocity_KP = 15.0f;
float Velocity_KI = 7.0f;
float Bluetooth_Velocity = 30.0f;
float PS2_Velocity = 30.0f;
float PS2_Turn_Velocity = 0.0f;
float Show_Data_Mb = 0.0f;
uint32_t Lap_Time_Ms = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void Board_Led_Blink_Twice(void)
{
  uint8_t i;

  for (i = 0; i < 2; i++)
  {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);  /* PC13 LED 常见为低电平点亮 */
    HAL_Delay(250);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    HAL_Delay(250);
  }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  MX_GPIO_Init();
  Board_Led_Blink_Twice();
  OLED_Init();
  OLED_ShowString(0, 0, "STATE:READY");
  OLED_Refresh_Gram();
  Track_Init();
  Ultrasonic_Init();
  Mode_Key_Init();
  Mode_Select_Before_Run();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  Camera_Init();

#if PA8_GPIO_OUTPUT_TEST
  PA8_GPIO_Output_Test();
#endif

#if MOTOR_CHANNEL_SELF_TEST
  Motor_Channel_Self_Test();
#endif
  #if USE_ENCODER_FEEDBACK
  MX_TIM3_Init();
  MX_TIM4_Init();
  #endif

  /* USER CODE BEGIN 2 */
  HAL_Delay(500);
  Lap_Time_Ms = LineTracking_Run_OneLap();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
#if REPEAT_RUN_TEST
    HAL_Delay(2000);
    Lap_Time_Ms = LineTracking_Run_OneLap();
#else
    Motor_Stop();
    HAL_Delay(100);
#endif
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* 防止外部晶振异常时直接卡死：退回 HSI 内部时钟。 */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSEState = RCC_HSE_OFF;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
      Error_Handler();
    }
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  __disable_irq();

  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  (void)file;
  (void)line;
  /* USER CODE END 6 */
}
#endif













