/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
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
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
uint8_t recArr[32], recData, cnt = 0, endPackCnt = 0, packReady = 0, pageId = 0;
uint8_t sendStr[32], sendLen = 0;
uint16_t adcVal = 0;
uint32_t lastTimeSend = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//UART Rx data receieve callback function
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == &huart1)
	{
		//add received data to buffer
		recArr[cnt++] = recData;

		// count received 0xFF
		if(recData == 0xFF)
		{
			endPackCnt++;
		}else{
			endPackCnt = 0;
		}

		// if 0xFF count reach 3, make packet flag true
		if(endPackCnt == 3)
		{
			packReady = 1;
			endPackCnt = 0;
		}


		HAL_UART_Receive_IT(&huart1, (uint8_t *)&recData, 1);
	}
}

// adc conversion complete callback function
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if(hadc == &hadc1)
	{
		adcVal = HAL_ADC_GetValue(hadc);
		HAL_ADC_Start_IT(&hadc1);
	}
}

// update widget function which have value parameter
void setValueNextion(const char *pId, int pVal)
{
	sendLen = sprintf((char *)sendStr, "%s.val=%d%c%c%c", pId, pVal, 0xFF, 0xFF, 0xFF);
	HAL_UART_Transmit(&huart1, sendStr, sendLen, 2000);
	HAL_Delay(10);
}

// update widget function which have text parameter
void setTextNextion(const char *pId, const char *pVal)
{
	sendLen = sprintf((char *)sendStr, "%s.txt=\"%s\"%c%c%c", pId, pVal, 0xFF, 0xFF, 0xFF);
	HAL_UART_Transmit(&huart1, sendStr, sendLen, 2000);
	HAL_Delay(10);
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

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  // start ADC, UART and Timer
  HAL_UART_Receive_IT(&huart1, (uint8_t *)&recData, 1);
  HAL_ADC_Start_IT(&hadc1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0);

  // reset nextion screen
  sendLen = sprintf((char *)sendStr, "rest%c%c%c", 0xFF, 0xFF, 0xFF);
  HAL_UART_Transmit(&huart1, sendStr, sendLen, 2000);
  HAL_Delay(10);

  // set LEDs initial stutus to OFF
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);

  // set LED status text' s in Nextion screen
  setTextNextion("t0", HAL_GPIO_ReadPin(LED1_GPIO_Port, LED1_Pin) ? "LED Yandi" : "LED Sondu");
  setTextNextion("t1", HAL_GPIO_ReadPin(LED2_GPIO_Port, LED2_Pin) ? "LED Yandi" : "LED Sondu");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  // update screen in every 100ms
	  if(HAL_GetTick() - lastTimeSend > 100)
	  {
		  // according to open page send updated data
		  if(pageId == 0)
		  {
			  // if first page opened, send LED status
	  		  setTextNextion("t0", HAL_GPIO_ReadPin(LED1_GPIO_Port, LED1_Pin) ? "LED Yandi" : "LED Sondu");
	  		  setTextNextion("t1", HAL_GPIO_ReadPin(LED2_GPIO_Port, LED2_Pin) ? "LED Yandi" : "LED Sondu");
		  }else if(pageId == 1)
		  {
			  // if second page opened, send ADC data

			  // scale 12 bit adc to between 0-100
			  uint8_t sendAdc = adcVal * 100 / 4096 ;

			  // convert int value to str
			  char txtPtr[5];
			  itoa(sendAdc, txtPtr, 10);
			  if(sendAdc > 100) sendAdc = 100;
			  setValueNextion("j0", sendAdc);
			  setTextNextion("t0", txtPtr);
		  }

		  lastTimeSend = HAL_GetTick();
	  }

	  // check nextion packet flag is true
	  // if flag is true all of bytes in buffer received
	  if(packReady && cnt > 0)
	  {
		  // check all received packet to where is start byte
		  for(int i = 0; i < cnt; i++)
		  {
			  // check 0x65 byte index in buffer
			  if(recArr[i] == 0x65)
			  {
				  // read bytes from receive buffer
				  uint8_t pId = recArr[i + 1];
				  uint8_t mId = recArr[i + 2];
				  uint8_t mVal = recArr[i + 3];

				  uint16_t pwmVal = mVal * 10;

				  // make choose according to used button' s pageId and widget ID
				  if(pId == 0)
				  {
					  switch(mId)
					  {
					  	  // LED 1 toggle button ID
						  case 2:
							  HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
					  		  setTextNextion("t0", HAL_GPIO_ReadPin(LED1_GPIO_Port, LED1_Pin) ? "LED Yandi" : "LED Sondu");
							  break;
						  // LED 2 toggle button ID
						  case 3:
							  HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
					  		  setTextNextion("t1", HAL_GPIO_ReadPin(LED2_GPIO_Port, LED2_Pin) ? "LED Yandi" : "LED Sondu");
							  break;
						  // change page 1 button ID
						  case 6:
							  pageId = 1;
					  		  HAL_Delay(10);
							  break;
					  }
				  }else if(pId == 1)
				  {
					  switch(mId)
					  {
					  	  // change page 0 button ID
					  	  case 2:
					  		  pageId = 0;
					  		  setTextNextion("t0", HAL_GPIO_ReadPin(LED1_GPIO_Port, LED1_Pin) ? "LED Yandi" : "LED Sondu");
					  		  setTextNextion("t1", HAL_GPIO_ReadPin(LED2_GPIO_Port, LED2_Pin) ? "LED Yandi" : "LED Sondu");
					  		  break;
						  // LED 1 switch button ID
						  case 4:
							  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, mVal);
							  break;
						  // slider ID
						  case 7:
							  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, pwmVal);
							  break;
					  }
				  }

				  lastTimeSend = HAL_GetTick();

			  }
		  }
		  packReady = 0;
		  cnt = 0;
	  }
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
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

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

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
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 71;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED1_Pin|LED2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED1_Pin LED2_Pin */
  GPIO_InitStruct.Pin = LED1_Pin|LED2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
