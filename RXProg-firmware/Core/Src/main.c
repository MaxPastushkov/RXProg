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
#include "usbd_cdc_if.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef void (*exti_handler_t)(uint16_t GPIO_Pin);
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define GPIOA_OUTPUT_MODE 0x2BF55555
#define GPIOA_INPUT_MODE  0x2BF50000

#define SAVE_ADDR 0x0807A000UL
#define BANK_SIZE 8192u
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

static uint8_t RX5 = 0;

static volatile uint8_t ram1_data[BANK_SIZE];
static volatile uint8_t ram2_data[BANK_SIZE];
static volatile uint8_t ram3_data[BANK_SIZE];

// Count accesses to each address
static volatile uint16_t ram1_count[BANK_SIZE];
static volatile uint16_t ram2_count[BANK_SIZE];
static volatile uint16_t ram3_count[BANK_SIZE];

volatile uint8_t dump_request = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */
static void write_ram_to_flash(void);
static void usb_send(const uint8_t *data, uint32_t len);
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

  uint32_t last_reset_edge_time = 0;
  GPIO_PinState last_reset_state = 0;

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

  // Enable both level shifters
  HAL_GPIO_WritePin(ADDR_OE_GPIO_Port, ADDR_OE_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(DATA_OE_GPIO_Port, DATA_OE_Pin, GPIO_PIN_RESET);

  CDC_Printf("Copying saved flash data to RAM...\n");

  // Copy saved flash values to buffers
  memcpy((void *)ram1_data, (const void *)(SAVE_ADDR + 0 * BANK_SIZE), BANK_SIZE);
  memcpy((void *)ram2_data, (const void *)(SAVE_ADDR + 1 * BANK_SIZE), BANK_SIZE);
  memcpy((void *)ram3_data, (const void *)(SAVE_ADDR + 2 * BANK_SIZE), BANK_SIZE);

  CDC_Printf("Done.\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
   while (1)
   {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

     if (dump_request)
     {
       usb_send((const uint8_t *)ram1_count, sizeof(uint16_t));
       usb_send((const uint8_t *)ram2_count, sizeof(uint16_t));
       usb_send((const uint8_t *)ram3_count, sizeof(uint16_t));
       dump_request = 0;
     }

     GPIO_PinState reset_state;
     if (RX5)
       reset_state = !HAL_GPIO_ReadPin(VBAT_RES_GPIO_Port, VBAT_RES_Pin);
     else
       reset_state = HAL_GPIO_ReadPin(RES_OE_GPIO_Port, RES_OE_Pin);

     if (reset_state)
     {
       if (!last_reset_state)
         last_reset_edge_time = HAL_GetTick();

       if (HAL_GetTick() - last_reset_edge_time > 1000)
       {
         // Go into reset

         // Copy ram buffers to flash
         write_ram_to_flash();

         HAL_GPIO_WritePin(ADDR_OE_GPIO_Port, ADDR_OE_Pin, GPIO_PIN_SET);
         HAL_GPIO_WritePin(DATA_OE_GPIO_Port, DATA_OE_Pin, GPIO_PIN_SET);

         GPIOA->MODER = 0xFFF5FFFF; // Keep addr/data level shifters disabled
         GPIOB->MODER = 0xFFFFFFFF;
         GPIOC->MODER = 0xC3FFFFFF; // Set all analog but the reset lines

         // Configure interrupts on reset lines
         GPIO_InitTypeDef GPIO_InitStruct = {0};
         GPIO_InitStruct.Pin = VBAT_RES_Pin|RES_OE_Pin;
         GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
         GPIO_InitStruct.Pull = GPIO_NOPULL;
         HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

         HAL_SuspendTick();
         HAL_PWREx_EnterSTOP1Mode(PWR_STOPENTRY_WFI);

         // The STM is now waiting for the device to power up again

         // Re-init everything
         SystemClock_Config();
         HAL_ResumeTick();
         MX_GPIO_Init();

         // Enable both level shifters
         HAL_GPIO_WritePin(ADDR_OE_GPIO_Port, ADDR_OE_Pin, GPIO_PIN_RESET);
         HAL_GPIO_WritePin(DATA_OE_GPIO_Port, DATA_OE_Pin, GPIO_PIN_RESET);
       }
     }

     last_reset_state = reset_state;
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
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
  volatile uint8_t *ram_data;
  volatile uint16_t *ram_count;

  if (GPIO_Pin == CS1_Pin)
  {
    ram_data = ram1_data;
    ram_count = ram1_count;
  }
  else if (GPIO_Pin == CS2_Pin)
  {
    ram_data = ram2_data;
    ram_count = ram2_count;
  }
  else
  {
    ram_data = ram3_data;
    ram_count = ram3_count;
  }

  uint16_t addr = GPIOB->IDR & (RX5 ? 0x1FFF : 0x7FF);

  uint8_t reading = RX5 ? !(RES_OE_GPIO_Port->IDR & RES_OE_Pin)
                        : (WE_A11_GPIO_Port->IDR & WE_A11_Pin);

  if (reading) // CPU is reading
  {
    GPIOA->ODR =
      (GPIOA->ODR & 0xFFFFFF00)
      | (ram_data[addr] & 0xFF);

    GPIOA->MODER = GPIOA_OUTPUT_MODE; // Set port to output
  }
  else // CPU is writing
  {
    GPIOA->MODER = GPIOA_INPUT_MODE; // Set port to input

    ram_data[addr] = GPIOA->IDR & 0xFF;
  }

  ram_count[addr]++;
}

static void write_ram_to_flash(void)
{
  volatile uint8_t *bufs[3] = { ram1_data, ram2_data, ram3_data };

  HAL_FLASH_Unlock();
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

  /* --- erase the save region --- */
  uint32_t dbank  = READ_BIT(FLASH->OPTR, FLASH_OPTR_DBANK);
  uint32_t pagesz = dbank ? 0x800u : 0x1000u; /* 2 KB vs 4 KB pages */

  FLASH_EraseInitTypeDef e = {0};
  e.TypeErase = FLASH_TYPEERASE_PAGES;
  e.NbPages   = (3u * BANK_SIZE) / pagesz;
  if (dbank) {
    e.Banks = FLASH_BANK_2;                          /* 0x08040000-0x0807FFFF */
    e.Page  = (SAVE_ADDR - 0x08040000UL) / pagesz;
  } else {
    e.Banks = FLASH_BANK_1;
    e.Page  = (SAVE_ADDR - 0x08000000UL) / pagesz;
  }
  uint32_t page_err;
  if (HAL_FLASHEx_Erase(&e, &page_err) != HAL_OK) { HAL_FLASH_Lock(); return; }

  /* --- program 8 bytes at a time --- */
  uint32_t addr = SAVE_ADDR;
  for (int b = 0; b < 3; b++) {
    for (uint32_t i = 0; i < BANK_SIZE; i += 8) {
      uint64_t dw;
      memcpy(&dw, (const void *)(bufs[b] + i), 8); /* avoids alignment/aliasing issues */
      if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr, dw) != HAL_OK) {
        HAL_FLASH_Lock();
        return;
      }
      addr += 8;
    }
  }
  HAL_FLASH_Lock();
}

static void usb_send(const uint8_t *data, uint32_t len)
{
  while (len)
  {
    uint32_t chunk = len > APP_TX_DATA_SIZE ? APP_TX_DATA_SIZE : len;
    while (CDC_Transmit_FS((uint8_t *)data, chunk) == USBD_BUSY) { }  // wait for EP
    data += chunk;
    len  -= chunk;
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
