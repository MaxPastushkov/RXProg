/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32g4xx_hal.h"

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
#define VBAT_RES_Pin GPIO_PIN_13
#define VBAT_RES_GPIO_Port GPIOC
#define RES_OE_Pin GPIO_PIN_14
#define RES_OE_GPIO_Port GPIOC
#define WE_Pin GPIO_PIN_15
#define WE_GPIO_Port GPIOC
#define D0_Pin GPIO_PIN_0
#define D0_GPIO_Port GPIOA
#define D1_Pin GPIO_PIN_1
#define D1_GPIO_Port GPIOA
#define D2_Pin GPIO_PIN_2
#define D2_GPIO_Port GPIOA
#define D3_Pin GPIO_PIN_3
#define D3_GPIO_Port GPIOA
#define D4_Pin GPIO_PIN_4
#define D4_GPIO_Port GPIOA
#define D5_Pin GPIO_PIN_5
#define D5_GPIO_Port GPIOA
#define D6_Pin GPIO_PIN_6
#define D6_GPIO_Port GPIOA
#define D7_Pin GPIO_PIN_7
#define D7_GPIO_Port GPIOA
#define A0_Pin GPIO_PIN_0
#define A0_GPIO_Port GPIOB
#define A1_Pin GPIO_PIN_1
#define A1_GPIO_Port GPIOB
#define A2_Pin GPIO_PIN_2
#define A2_GPIO_Port GPIOB
#define A10_Pin GPIO_PIN_10
#define A10_GPIO_Port GPIOB
#define WE_A11_Pin GPIO_PIN_11
#define WE_A11_GPIO_Port GPIOB
#define A12_Pin GPIO_PIN_12
#define A12_GPIO_Port GPIOB
#define CS1_Pin GPIO_PIN_13
#define CS1_GPIO_Port GPIOB
#define CS1_EXTI_IRQn EXTI15_10_IRQn
#define CS2_Pin GPIO_PIN_14
#define CS2_GPIO_Port GPIOB
#define CS2_EXTI_IRQn EXTI15_10_IRQn
#define CS3_Pin GPIO_PIN_15
#define CS3_GPIO_Port GPIOB
#define CS3_EXTI_IRQn EXTI15_10_IRQn
#define DATA_OE_Pin GPIO_PIN_8
#define DATA_OE_GPIO_Port GPIOA
#define ADDR_OE_Pin GPIO_PIN_9
#define ADDR_OE_GPIO_Port GPIOA
#define RX5_Pin GPIO_PIN_15
#define RX5_GPIO_Port GPIOA
#define A3_Pin GPIO_PIN_3
#define A3_GPIO_Port GPIOB
#define A4_Pin GPIO_PIN_4
#define A4_GPIO_Port GPIOB
#define A5_Pin GPIO_PIN_5
#define A5_GPIO_Port GPIOB
#define A6_Pin GPIO_PIN_6
#define A6_GPIO_Port GPIOB
#define A7_Pin GPIO_PIN_7
#define A7_GPIO_Port GPIOB
#define A8_Pin GPIO_PIN_8
#define A8_GPIO_Port GPIOB
#define A9_Pin GPIO_PIN_9
#define A9_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
