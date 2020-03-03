/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "ucpd.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "encoder.h"
#include "bluetooth.h"
#include <stdio.h>
#include "midiController.h"
#include "msgDecoder.h"

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

uint16_t midiFifoIndex;
uint8_t midiFifo[500], uartMsgDecodeBuff[300];


unsigned int sw = 0;
uint8_t c, cycles = 0 ;
char buffer[60];

int x = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_UCPD1_Init();
  MX_TIM2_Init();
  MX_TIM1_Init();
  MX_RTC_Init();
  MX_TIM3_Init();
  MX_USART2_UART_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);

  HAL_RTC_Init(&hrtc);

  oled_begin();

  HAL_TIM_Base_Start_IT(&htim1);

  bluetoothInit();


  oledType = OLED_MENU;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  //Pokud byl request na skenovani
	 	  if(workerBtScanDev){
	 		 bluetoothGetScannedDevices();
	 		 oled_setDisplayedMenu("btScanedDevices", &btScanedDevices, sizeof(btScanedDevices)-(20-btScannedCount-1)*sizeof(btScanedDevices[19]), 0);
	 		 workerBtScanDev = 0;
	 	  }

	 	  if(workerBtBondDev){
	 		 bluetoothGetBondedDevices();
	 		 oled_setDisplayedMenu("btBondedDevicesMenu", &btBondedDevicesMenu, sizeof(btBondedDevicesMenu)-(10-btBondedCount-1)*sizeof(btBondedDevicesMenu[9]), 0);
	 		 workerBtBondDev = 0;
	 	  }

	 	  if(btMsgReceivedFlag){
	 		  decodeMessage(uartMsgDecodeBuff, btMessageLen+6, ((uartMsgDecodeBuff[6] & 0x04) >> 2));
	 		  btMsgReceivedFlag = 0;
	 	  }

	 	  if(workerBtEnterPairingKey){
	 		  if(!btCmdMode) bluetoothEnterCMD();
	 		  char pin[10];
	 		  sprintf(pin, "%06ld\r", btPairReq.pin);
	 		  bluetoothCMD_ACK(pin, "");
	 		 if(btCmdMode) bluetoothLeaveCMD();
	 		 workerBtEnterPairingKey = 0;
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

  /** Configure the main internal regulator output voltage 
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the peripherals clocks 
  */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_ADC
                              |RCC_PERIPHCLK_TIM1;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_SYSCLK;
  PeriphClkInit.Tim1ClockSelection = RCC_TIM1CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){

	if(htim->Instance == TIM2){
		//Obnovi se oled displej
		oled_refresh();
	}

	if(htim->Instance == TIM1){
		midiControl_checkKeyboard();

		if(keypad.changed){
			if(keypad.up){
				encoderpos--;
				keyboardVertPosOld = keyboardVertPos;
				keyboardVertPos++;
			}else if(keypad.down){
				encoderpos++;
				keyboardVertPosOld = keyboardVertPos;
				keyboardVertPos--;
			}

			if(keypad.right){
				keyboardSidePos++;
			}else if(keypad.left){
				keyboardSidePos--;
			}

			keypadClicks.zero |= keypad.zero;
			keypadClicks.one |= keypad.one;
			keypadClicks.two |= keypad.two;
			keypadClicks.three |= keypad.three;
			keypadClicks.four |= keypad.four;
			keypadClicks.five |= keypad.five;
			keypadClicks.six |= keypad.six;
			keypadClicks.seven |= keypad.seven;
			keypadClicks.eight |= keypad.eight;
			keypadClicks.nine |= keypad.nine;
			keypadClicks.down |= keypad.down;
			keypadClicks.up |= keypad.up;
			keypadClicks.left |= keypad.left;
			keypadClicks.right |= keypad.right;
			keypadClicks.enter |= keypad.enter;
			keypadClicks.mf1 |= keypad.mf1;
			keypadClicks.power |= keypad.power;

			//Dopocita se pozice v dispmenu
			if(encoderpos >= (signed int)(dispmenusize)-1){
				encoderpos = (signed int)(dispmenusize)-1;
			}else if(encoderpos < (signed int)0){
				encoderpos = 0;
			}

			if(keyboardSidePos >= keyboardSidePosMax){
				keyboardSidePos = keyboardSidePosMax;
			}else if(keyboardSidePos < 0){
				keyboardSidePos = 0;
			}

			if(keyboardVertPos >= keyboardVertPosMax){
				keyboardVertPos = keyboardVertPosMax;
			}else if(keyboardVertPos < 0){
				keyboardVertPos = 0;
			}


			if(keypad.enter > 0){
				encoderclick = 1;
				keypad.enter = 0;
			}

			keypad.changed = 0;
		}


	}

	if(htim->Instance == TIM3){

		//Tady se dela scrollovani
		if(scrollPauseDone){
			if(scrollIndex <= scrollMax){
				scrollIndex++;
			}else{
				scrollPauseDone = 0;
			}
		}else scrollPause++;

		if(scrollPause == OLED_MENU_SCROLL_PAUSE){
			if(scrollIndex > 0){
				scrollPauseDone = 0;
			}else scrollPauseDone = 1;

			scrollPause = 0;
			scrollIndex = 0;
		}

		if(loadingStat < 4){
			loadingStat <<= 1;
		}else loadingStat = 1;

		loadingToggle = ~loadingToggle;

		if(btStatusMsg){
			btStatusMsgWD++;

			if(btStatusMsgWD >= 2){
				bluetoothMsgFifoFlush();
				btStatusMsg = 0;
				btStatusMsgWD = 0;
			}
		}else btStatusMsgWD = 0;

	}

	if(htim->Instance == TIM6){
		HAL_TIM_Base_Stop_IT(&htim6);
		HAL_NVIC_DisableIRQ(TIM6_DAC_LPTIM1_IRQn);

	    keypad.enter = !(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14));
		keypad.changed = 1;
		x++;

		sprintf(oledHeader, "%d %d", x,  keypad.enter);

		HAL_NVIC_ClearPendingIRQ(EXTI4_15_IRQn);
		HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
		HAL_NVIC_ClearPendingIRQ(EXTI4_15_IRQn);
		HAL_NVIC_EnableIRQ(TIM6_DAC_LPTIM1_IRQn);
	}

}

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == GPIO_PIN_14 && HAL_TIM_Base_GetState(&htim6) != HAL_TIM_STATE_BUSY){
		HAL_NVIC_EnableIRQ(TIM6_DAC_LPTIM1_IRQn);
		HAL_TIM_Base_Start_IT(&htim6);
		HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){

	if(huart->Instance == USART2){
			//Pokud dostal status message od modulu
			if((btFifoByte == '%' || btStatusMsg) && !btCmdMode){
				if(btFifoByte == '%') btStatusMsg = ~btStatusMsg;
				if(btFifoByte == '%' && !btStatusMsg) bluetoothDecodeMsg();
				btMsgFifo[btMsgFifoIndex++] = btFifoByte;
			}else if(!btStatusMsg){

				if(!btCmdMode && btFifoByte == 0 && btNullCounter < 4 && btComMessageSizeFlag < 2){
					//Odpocitaji se 4 nully
					btNullCounter++;
				}else if(btNullCounter == 4 && btComMessageSizeFlag < 2){
					btMsgReceivedFlag = 0;
					//Odpocita se velikost
					btComMessageSizeFlag++;
					btMessageLen = 0;
				}else if(btNullCounter == 4 && btComMessageSizeFlag == 2 && btMessageLen == 0){
					//Zapise se index zacatku zpravy
					btComMessageStartIndex = btFifoIndex-6;

					btMessageLen = ((btFifo[btComMessageStartIndex+4] << 8) & 0xff00) | (btFifo[btComMessageStartIndex+5] & 0xff);

				}

				btFifo[btFifoIndex++] = btFifoByte;

				if(btMessageLen > 0 && (btFifoIndex) >= btMessageLen+btComMessageStartIndex+6 && btNullCounter == 4 && btComMessageSizeFlag == 2){

					memcpy(uartMsgDecodeBuff, btFifo+btComMessageStartIndex, btMessageLen+6);
					btMsgReceivedFlag = 1;
					bluetoothFifoFlush();
					btNullCounter = 0;
					btComMessageSizeFlag = 0;
				}

			}

			HAL_UART_Receive_IT(&huart2, &btFifoByte, 1);
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){

}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
