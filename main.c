/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2026 STMicroelectronics.
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
#include "fsm.h"

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
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */

static uint8_t displayColumn;
static volatile uint8_t displayRoadATime;
static volatile uint8_t displayRoadBTime;
static TrafficLightOutput_t appliedOutput;
static uint8_t outputInitialized;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

static void TrafficLight_Set(GPIO_PinState redA, GPIO_PinState yellowA,
                             GPIO_PinState greenA, GPIO_PinState redB,
                             GPIO_PinState yellowB, GPIO_PinState greenB);
static void Output_ApplyFsm(void);
static void SevenSegment_RefreshOnTimerInterrupt(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* Segment bits are abcdefg. The displays are common-anode, hence 0 turns on a
 * segment and the PNP digit-enable inputs (EN0..EN3) are active low. */
static void TrafficLight_Set(GPIO_PinState redA, GPIO_PinState yellowA,
                             GPIO_PinState greenA, GPIO_PinState redB,
                             GPIO_PinState yellowB, GPIO_PinState greenB)
{
  HAL_GPIO_WritePin(REDA_GPIO_Port, REDA_Pin, redA);
  HAL_GPIO_WritePin(YELLOWA_GPIO_Port, YELLOWA_Pin, yellowA);
  HAL_GPIO_WritePin(GREENA_GPIO_Port, GREENA_Pin, greenA);
  HAL_GPIO_WritePin(REDB_GPIO_Port, REDB_Pin, redB);
  HAL_GPIO_WritePin(YELLOWB_GPIO_Port, YELLOWB_Pin, yellowB);
  HAL_GPIO_WritePin(GREENB_GPIO_Port, GREENB_Pin, greenB);
}

static void Output_ApplyFsm(void)
{
  TrafficLightOutput_t output;
  uint8_t modeDigits;
  uint8_t valueDigits;

  fsm_get_display(&modeDigits, &valueDigits);
  displayRoadATime = modeDigits;
  displayRoadBTime = valueDigits;
  fsm_get_lights(&output);

  if ((outputInitialized != 0U) &&
      (output.redA == appliedOutput.redA) &&
      (output.yellowA == appliedOutput.yellowA) &&
      (output.greenA == appliedOutput.greenA) &&
      (output.redB == appliedOutput.redB) &&
      (output.yellowB == appliedOutput.yellowB) &&
      (output.greenB == appliedOutput.greenB) &&
      (output.extraRed == appliedOutput.extraRed))
    return;

  TrafficLight_Set(output.redA, output.yellowA, output.greenA,
                   output.redB, output.yellowB, output.greenB);
  HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, output.extraRed);
  appliedOutput = output;
  outputInitialized = 1U;
}

/* Returns abcdefg bits. A bit at 1 means that segment must be lit. */
static uint8_t display7SEG(int num)
{
  switch (num)
  {
    case 0: return 0x3FU;
    case 1: return 0x06U;
    case 2: return 0x5BU;
    case 3: return 0x4FU;
    case 4: return 0x66U;
    case 5: return 0x6DU;
    case 6: return 0x7DU;
    case 7: return 0x07U;
    case 8: return 0x7FU;
    case 9: return 0x6FU;
    default: return 0x00U;
  }
}

/* GPIO B: a1..g1 are PB0..PB6; g2 is PB7.
 * GPIO A: a2 is PA4 and b2..f2 are PA11..PA15.
 * The displays are common-anode, so BSRR's upper half resets (lights) a pin. */
static void SevenSegment_WritePair(uint8_t upperSegments, uint8_t lowerSegments)
{
  const uint16_t topMask = a1_Pin | b1_Pin | c1_Pin | d1_Pin | e1_Pin | f1_Pin | g1_Pin;
  const uint16_t lowerAMask = a2_Pin | b2_Pin | c2_Pin | d2_Pin | e2_Pin | f2_Pin;
  uint16_t setA = 0U;
  uint16_t resetA = 0U;
  uint16_t setB = 0U;
  uint16_t resetB = 0U;
  uint8_t bit;
  const uint16_t lowerAPin[6] = {a2_Pin, b2_Pin, c2_Pin, d2_Pin, e2_Pin, f2_Pin};

  resetB = (uint16_t)(upperSegments & 0x7FU);
  setB = (uint16_t)(topMask & (uint16_t)~resetB);

  for (bit = 0U; bit < 6U; bit++)
  {
    if ((lowerSegments & (1U << bit)) != 0U) resetA |= lowerAPin[bit];
  }
  setA = (uint16_t)(lowerAMask & (uint16_t)~resetA);
  if ((lowerSegments & 0x40U) != 0U) resetB |= g2_Pin;
  else setB |= g2_Pin;

  GPIOA->BSRR = (uint32_t)setA | ((uint32_t)resetA << 16U);
  GPIOB->BSRR = (uint32_t)setB | ((uint32_t)resetB << 16U);
}

/* TIM2 overflows every 10 ms. One digit per road is refreshed per interrupt. */
static void SevenSegment_RefreshOnTimerInterrupt(void)
{
  uint8_t upperDigit;
  uint8_t lowerDigit;
  uint8_t upperSegments;
  uint8_t lowerSegments;

  GPIOB->BSRR = EN0_Pin | EN1_Pin | EN2_Pin | EN3_Pin;

  upperDigit = (displayColumn == 0U) ? displayRoadATime / 10U : displayRoadATime % 10U;
  lowerDigit = (displayColumn == 0U) ? displayRoadBTime / 10U : displayRoadBTime % 10U;
  upperSegments = display7SEG(upperDigit);
  lowerSegments = display7SEG(lowerDigit);
  SevenSegment_WritePair(upperSegments, lowerSegments);

  /* Refresh one digit on each road at the same time: 50 Hz per digit. */
  GPIOB->BSRR = ((uint32_t)((displayColumn == 0U) ? (EN0_Pin | EN2_Pin) :
                             (EN1_Pin | EN3_Pin)) << 16U);
  displayColumn ^= 1U;
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
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  fsm_init();
  HAL_NVIC_SetPriority(TIM2_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(TIM2_IRQn);
  HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    fsm_system_run();
    Output_ApplyFsm();
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
  * in the RCC_OscInitStruct structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9;  /* 8 MHz / (7999 + 1) / (9 + 1) = 10 ms */
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED_RED_Pin|a2_Pin|REDA_Pin|GREENA_Pin
                          |YELLOWA_Pin|REDB_Pin|GREENB_Pin|YELLOWB_Pin
                          |b2_Pin|c2_Pin|d2_Pin|e2_Pin
                          |f2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, a1_Pin|b1_Pin|c1_Pin|EN1_Pin
                          |EN2_Pin|EN3_Pin|EN0_Pin|d1_Pin
                          |e1_Pin|f1_Pin|g1_Pin|g2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED_RED_Pin a2_Pin REDA_Pin GREENA_Pin
                           YELLOWA_Pin REDB_Pin GREENB_Pin YELLOWB_Pin
                           b2_Pin c2_Pin d2_Pin e2_Pin
                           f2_Pin */
  GPIO_InitStruct.Pin = LED_RED_Pin|a2_Pin|REDA_Pin|GREENA_Pin
                          |YELLOWA_Pin|REDB_Pin|GREENB_Pin|YELLOWB_Pin
                          |b2_Pin|c2_Pin|d2_Pin|e2_Pin
                          |f2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : a1_Pin b1_Pin c1_Pin EN1_Pin
                           EN2_Pin EN3_Pin EN0_Pin d1_Pin
                           e1_Pin f1_Pin g1_Pin g2_Pin */
  GPIO_InitStruct.Pin = a1_Pin|b1_Pin|c1_Pin|EN1_Pin
                          |EN2_Pin|EN3_Pin|EN0_Pin|d1_Pin
                          |e1_Pin|f1_Pin|g1_Pin|g2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : BUT3_Pin BUT1_Pin BUT2_Pin */
  GPIO_InitStruct.Pin = BUT3_Pin|BUT1_Pin|BUT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* TIM2 interrupt: HAL calls getKeyInput() in timer.c, then refresh one
 * seven-segment digit for each road. */
void TIM2_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim2);
  SevenSegment_RefreshOnTimerInterrupt();
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
