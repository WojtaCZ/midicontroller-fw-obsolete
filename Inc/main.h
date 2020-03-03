/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"
#include "stm32g0xx_ll_ucpd.h"
#include "stm32g0xx_ll_bus.h"
#include "stm32g0xx_ll_cortex.h"
#include "stm32g0xx_ll_rcc.h"
#include "stm32g0xx_ll_system.h"
#include "stm32g0xx_ll_utils.h"
#include "stm32g0xx_ll_pwr.h"
#include "stm32g0xx_ll_gpio.h"
#include "stm32g0xx_ll_dma.h"

#include "stm32g0xx_ll_exti.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BATT_VOLTAGE_Pin GPIO_PIN_0
#define BATT_VOLTAGE_GPIO_Port GPIOA
#define POWER_ON_Pin GPIO_PIN_1
#define POWER_ON_GPIO_Port GPIOA
#define SPI_CS_Pin GPIO_PIN_4
#define SPI_CS_GPIO_Port GPIOA
#define DET_5V_Pin GPIO_PIN_5
#define DET_5V_GPIO_Port GPIOA
#define KB_R1_Pin GPIO_PIN_1
#define KB_R1_GPIO_Port GPIOB
#define KB_R2_Pin GPIO_PIN_2
#define KB_R2_GPIO_Port GPIOB
#define KB_R3_Pin GPIO_PIN_10
#define KB_R3_GPIO_Port GPIOB
#define KB_R4_Pin GPIO_PIN_11
#define KB_R4_GPIO_Port GPIOB
#define KB_R5_Pin GPIO_PIN_12
#define KB_R5_GPIO_Port GPIOB
#define SW_POWER_Pin GPIO_PIN_13
#define SW_POWER_GPIO_Port GPIOB
#define SW_ENTER_Pin GPIO_PIN_14
#define SW_ENTER_GPIO_Port GPIOB
#define SW_ENTER_EXTI_IRQn EXTI4_15_IRQn
#define CC__Pin GPIO_PIN_15
#define CC__GPIO_Port GPIOB
#define CC_A8_Pin GPIO_PIN_8
#define CC_A8_GPIO_Port GPIOA
#define DBCC__Pin GPIO_PIN_9
#define DBCC__GPIO_Port GPIOA
#define DBCC_A10_Pin GPIO_PIN_10
#define DBCC_A10_GPIO_Port GPIOA
#define KB_C1_Pin GPIO_PIN_0
#define KB_C1_GPIO_Port GPIOD
#define KB_C2_Pin GPIO_PIN_1
#define KB_C2_GPIO_Port GPIOD
#define KB_C3_Pin GPIO_PIN_2
#define KB_C3_GPIO_Port GPIOD
#define BQ_INT_Pin GPIO_PIN_4
#define BQ_INT_GPIO_Port GPIOB
#define BQ_INT_EXTI_IRQn EXTI4_15_IRQn
#define BQ_OTG_Pin GPIO_PIN_5
#define BQ_OTG_GPIO_Port GPIOB
#define BT_RST_Pin GPIO_PIN_8
#define BT_RST_GPIO_Port GPIOB
#define BT_MODE_Pin GPIO_PIN_9
#define BT_MODE_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
