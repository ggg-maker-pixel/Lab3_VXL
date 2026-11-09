/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

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
#define LED_RED_Pin GPIO_PIN_3
#define LED_RED_GPIO_Port GPIOA
#define a2_Pin GPIO_PIN_4
#define a2_GPIO_Port GPIOA
#define REDA_Pin GPIO_PIN_5
#define REDA_GPIO_Port GPIOA
#define GREENA_Pin GPIO_PIN_6
#define GREENA_GPIO_Port GPIOA
#define YELLOWA_Pin GPIO_PIN_7
#define YELLOWA_GPIO_Port GPIOA
#define a1_Pin GPIO_PIN_0
#define a1_GPIO_Port GPIOB
#define b1_Pin GPIO_PIN_1
#define b1_GPIO_Port GPIOB
#define c1_Pin GPIO_PIN_2
#define c1_GPIO_Port GPIOB
#define BUT3_Pin GPIO_PIN_10
#define BUT3_GPIO_Port GPIOB
#define EN1_Pin GPIO_PIN_12
#define EN1_GPIO_Port GPIOB
#define EN2_Pin GPIO_PIN_13
#define EN2_GPIO_Port GPIOB
#define EN3_Pin GPIO_PIN_14
#define EN3_GPIO_Port GPIOB
#define EN0_Pin GPIO_PIN_15
#define EN0_GPIO_Port GPIOB
#define REDB_Pin GPIO_PIN_8
#define REDB_GPIO_Port GPIOA
#define GREENB_Pin GPIO_PIN_9
#define GREENB_GPIO_Port GPIOA
#define YELLOWB_Pin GPIO_PIN_10
#define YELLOWB_GPIO_Port GPIOA
#define b2_Pin GPIO_PIN_11
#define b2_GPIO_Port GPIOA
#define c2_Pin GPIO_PIN_12
#define c2_GPIO_Port GPIOA
#define d2_Pin GPIO_PIN_13
#define d2_GPIO_Port GPIOA
#define e2_Pin GPIO_PIN_14
#define e2_GPIO_Port GPIOA
#define f2_Pin GPIO_PIN_15
#define f2_GPIO_Port GPIOA
#define d1_Pin GPIO_PIN_3
#define d1_GPIO_Port GPIOB
#define e1_Pin GPIO_PIN_4
#define e1_GPIO_Port GPIOB
#define f1_Pin GPIO_PIN_5
#define f1_GPIO_Port GPIOB
#define g1_Pin GPIO_PIN_6
#define g1_GPIO_Port GPIOB
#define g2_Pin GPIO_PIN_7
#define g2_GPIO_Port GPIOB
#define BUT1_Pin GPIO_PIN_8
#define BUT1_GPIO_Port GPIOB
#define BUT2_Pin GPIO_PIN_9
#define BUT2_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
