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
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
char sendBuff[64], sendBuff2[64];
uint8_t recArr[64], recData;
uint8_t buffLen, buffLen2, buffCnt = 0;
uint8_t dhtval[2];

const char API_KEY[] = "YOUR_THINGSPEAK_API_KEY";
const char WIFI_NAME[] = "YOUR_WIFI_SSID";
const char WIFI_PASSW[] = "YOUR_WIFI_PSWD";

#define OUTPUT 1
#define INPUT 0

//set DHT pin direction with given parameter
void set_gpio_mode(uint8_t pMode)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	//if direction parameter OUTPUT
	if(pMode == OUTPUT)
	{
	/*Configure GPIO pins : LD3_Pin LD2_Pin */
	  GPIO_InitStruct.Pin = DHT11_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	}else if(pMode == INPUT)   //else if direction parameter INPUT
	{
	  GPIO_InitStruct.Pin = DHT11_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	}
}

//give microsecond delay
void mDelay(uint32_t mDelay)
{
	uint16_t initTime = (uint16_t)__HAL_TIM_GET_COUNTER(&htim4);			//get initial time
	while(((uint16_t)__HAL_TIM_GET_COUNTER(&htim4)-initTime) < mDelay){		//wait until (time now - inital time == 0)
	}
}

// readDHT11 funciton
uint8_t readDHT11(uint8_t *pData)
{
	uint16_t mTime1 = 0, mTime2 = 0, mBit = 0;
	uint8_t humVal = 0, tempVal = 0, parityVal = 0, genParity = 0;
	uint8_t mData[40];

	//start comm
	set_gpio_mode(OUTPUT);			//set pin direction as input
	HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_RESET);
	HAL_Delay(20);					//wait 20 ms in Low state
	set_gpio_mode(INPUT);			//set pin direction as input

	//check dht answer
	__HAL_TIM_SET_COUNTER(&htim4, 0);				//set timer counter to zero
	while(HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == GPIO_PIN_SET) if((uint16_t)__HAL_TIM_GET_COUNTER(&htim4) > 500) return 0;
	__HAL_TIM_SET_COUNTER(&htim4, 0);
	while(HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == GPIO_PIN_RESET) if((uint16_t)__HAL_TIM_GET_COUNTER(&htim4) > 500) return 0;
	mTime1 = (uint16_t)__HAL_TIM_GET_COUNTER(&htim4);
	__HAL_TIM_SET_COUNTER(&htim4, 0);
	while(HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == GPIO_PIN_SET) if((uint16_t)__HAL_TIM_GET_COUNTER(&htim4) > 500) return 0;
	mTime2 = (uint16_t)__HAL_TIM_GET_COUNTER(&htim4);

	//if answer is wrong return
	if(mTime1 < 75 && mTime1 > 85 && mTime2 < 75 && mTime2 > 85)
	{
		return 0;
	}

	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
	for(int j = 0; j < 40; j++)
	{
		__HAL_TIM_SET_COUNTER(&htim4, 0);
		while(HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == GPIO_PIN_RESET) if((uint16_t)__HAL_TIM_GET_COUNTER(&htim4) > 500) return 0;
		__HAL_TIM_SET_COUNTER(&htim4, 0);
		while(HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == GPIO_PIN_SET) if((uint16_t)__HAL_TIM_GET_COUNTER(&htim4) > 500) return 0;
		mTime1 = (uint16_t)__HAL_TIM_GET_COUNTER(&htim4);

		//check pass time in high state
		//if pass time 25uS set as LOW

		if(mTime1 > 20 && mTime1 < 30)
		{
			mBit = 0;
		}
		else if(mTime1 > 60 && mTime1 < 80) //if pass time 70 uS set as HIGH
		{
			 mBit = 1;
		}

		//set i th data in data buffer
		mData[j] = mBit;

	}

	//get hum value from data buffer
	for(int i = 0; i < 8; i++)
	{
		humVal += mData[i];
		humVal = humVal << 1;
	}

	//get temp value from data buffer
	for(int i = 16; i < 24; i++)
	{
		tempVal += mData[i];
		tempVal = tempVal << 1;
	}

	//get parity value from data buffer
	for(int i = 32; i < 40; i++)
	{
		parityVal += mData[i];
		parityVal = parityVal << 1;
	}

	parityVal = parityVal >> 1;
	humVal = humVal >> 1;
	tempVal = tempVal >> 1;

	genParity = humVal + tempVal;

	if(genParity == parityVal)

	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

	pData[0] = tempVal;
	pData[1] = humVal;

	return 1;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	recArr[buffCnt++] = recData;
	if(buffCnt == 64) buffCnt = 0;
	HAL_UART_Receive_IT(&huart1, &recData, 1);
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
  MX_USART1_UART_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim4);

  // Wifi Connection
  buffLen = sprintf(sendBuff, "AT+CWMODE_CUR=1\r\n");
  HAL_UART_Transmit(&huart1, (uint8_t *)sendBuff, buffLen, 1000);

  buffLen = sprintf(sendBuff, "AT+CWLAP\r\n");
  HAL_UART_Transmit(&huart1, (uint8_t *)sendBuff, buffLen, 1000);

  HAL_Delay(2000);

  buffLen = sprintf(sendBuff, "AT+CWJAP_CUR=\"%s\",\"%s\"\r\n", WIFI_NAME, WIFI_PASSW);
  HAL_UART_Transmit(&huart1, (uint8_t *)sendBuff, buffLen, 1000);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  // check DHT11 is read succesfully
	  if(readDHT11(dhtval) == 1)
	  {
		  // connect Thinkspeak server
		  buffLen = sprintf(sendBuff, "AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\r\n");
		  HAL_UART_Transmit(&huart1, (uint8_t *)sendBuff, buffLen, 1000);
		  HAL_Delay(3000);

		  // after connection to Thingspeak prepare data to send
		  buffLen = sprintf(sendBuff, "GET /update?api_key=%s&field1=%d&field2=%d\r\n", API_KEY, dhtval[0], dhtval[1]);
		  buffLen2 = sprintf(sendBuff2, "AT+CIPSEND=%d\r\n", buffLen);

		  // send preapeared buffers to Thingspeak using ESP8266
		  HAL_UART_Transmit(&huart1, (uint8_t *)sendBuff2, buffLen2, 1000);
		  HAL_Delay(1000);
		  HAL_UART_Transmit(&huart1, (uint8_t *)sendBuff, buffLen, 1000);

		  // wait for 30sn to send new data
		  HAL_Delay(30000);
	  }else{
		  HAL_Delay(3000);
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
}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 72;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 0xFFFF;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_DISABLE;
  sSlaveConfig.InputTrigger = TIM_TS_ITR0;
  if (HAL_TIM_SlaveConfigSynchro(&htim4, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DHT11_Pin */
  GPIO_InitStruct.Pin = DHT11_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);

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
