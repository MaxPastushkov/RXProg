/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef void (*exti_handler_t)(uint16_t GPIO_Pin);
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define GPIOA_OUTPUT_MODE 0x2BF55555
#define GPIOA_INPUT_MODE  0x2BF50000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

static uint8_t RX5 = 0;
static exti_handler_t exti_handler;

static volatile uint8_t ram1_data[8192];
static volatile uint8_t ram2_data[8192];
static volatile uint8_t ram3_data[8192];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */
static void RX5_EXTI_Callback(uint16_t GPIO_Pin);
static void RX11_EXTI_Callback(uint16_t GPIO_Pin);
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
  MX_USB_Device_Init();
  /* USER CODE BEGIN 2 */

  // Check if RX5 jumper is shorted
  RX5 = (HAL_GPIO_ReadPin(RX5_GPIO_Port, RX5_Pin) == GPIO_PIN_RESET);

  // Set correct interrupt handler
  exti_handler = RX5 ? RX5_EXTI_Callback : RX11_EXTI_Callback;

  // Enable both level shifters
  HAL_GPIO_WritePin(ADDR_OE_GPIO_Port, ADDR_OE_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(DATA_OE_GPIO_Port, DATA_OE_Pin, GPIO_PIN_RESET);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
   while (1)
   {
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

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV6;
  RCC_OscInitStruct.PLL.PLLN = 108;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV6;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  exti_handler = RX11_EXTI_Callback; // Prevent a null-pointer dereference
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, D0_Pin|D1_Pin|D2_Pin|D3_Pin
                          |D4_Pin|D5_Pin|D6_Pin|D7_Pin
                          |DATA_OE_Pin|ADDR_OE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : VBAT_RES_Pin RES_OE_Pin WE_Pin */
  GPIO_InitStruct.Pin = VBAT_RES_Pin|RES_OE_Pin|WE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : D0_Pin D1_Pin D2_Pin D3_Pin
                           D4_Pin D5_Pin D6_Pin D7_Pin
                           DATA_OE_Pin ADDR_OE_Pin */
  GPIO_InitStruct.Pin = D0_Pin|D1_Pin|D2_Pin|D3_Pin
                          |D4_Pin|D5_Pin|D6_Pin|D7_Pin
                          |DATA_OE_Pin|ADDR_OE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : A0_Pin A1_Pin A2_Pin A10_Pin
                           WE_A11_Pin A12_Pin A3_Pin A4_Pin
                           A5_Pin A6_Pin A7_Pin A8_Pin
                           A9_Pin */
  GPIO_InitStruct.Pin = A0_Pin|A1_Pin|A2_Pin|A10_Pin
                          |WE_A11_Pin|A12_Pin|A3_Pin|A4_Pin
                          |A5_Pin|A6_Pin|A7_Pin|A8_Pin
                          |A9_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : CS1_Pin CS2_Pin CS3_Pin */
  GPIO_InitStruct.Pin = CS1_Pin|CS2_Pin|CS3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : RX5_Pin */
  GPIO_InitStruct.Pin = RX5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(RX5_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

__attribute__((section(".RamFunc")))
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  exti_handler(GPIO_Pin);
}

__attribute__((section(".RamFunc")))
static void RX11_EXTI_Callback(uint16_t GPIO_Pin)
{
  volatile uint8_t *ram_data;
  if (GPIO_Pin == CS1_Pin)
    ram_data = ram1_data;
  else if (GPIO_Pin == CS2_Pin)
    ram_data = ram2_data;
  else
    ram_data = ram3_data;

  if (WE_A11_GPIO_Port->IDR & WE_A11_Pin) // CPU is reading
  {
    GPIOA->ODR =
      (GPIOA->ODR & 0xFFFFFF00)
      | (ram_data[GPIOB->IDR & 0x7FF] & 0xFF);

    GPIOA->MODER = GPIOA_OUTPUT_MODE; // Set port to output
  }
  else // CPU is writing
  {
    GPIOA->MODER = GPIOA_INPUT_MODE; // Set port to input

    ram_data[GPIOB->IDR & 0x7FF] = GPIOA->IDR & 0xFF;
  }
}

__attribute__((section(".RamFunc")))
static void RX5_EXTI_Callback(uint16_t GPIO_Pin)
{
  volatile uint8_t *ram_data;
  if (GPIO_Pin == CS1_Pin)
    ram_data = ram1_data;
  else if (GPIO_Pin == CS2_Pin)
    ram_data = ram2_data;
  else
    ram_data = ram3_data;

  if (!(RES_OE_GPIO_Port->IDR & RES_OE_Pin)) // CPU is reading
  {
    GPIOA->ODR =
      (GPIOA->ODR & 0xFFFFFF00)
      | (ram_data[GPIOB->IDR & 0x1FFF] & 0xFF);

    GPIOA->MODER = GPIOA_OUTPUT_MODE; // Set port to output
  }
  else
  {
    GPIOA->MODER = GPIOA_INPUT_MODE; // Set port to input

    ram_data[GPIOB->IDR & 0x1FFF] = GPIOA->IDR & 0xFF;
  }
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
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
