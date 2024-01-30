/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32c0xx_hal.h"

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
#define StepY_Pin GPIO_PIN_0
#define StepY_GPIO_Port GPIOA
#define StepY_EXTI_IRQn EXTI0_1_IRQn
#define DirY_Pin GPIO_PIN_1
#define DirY_GPIO_Port GPIOA
#define DirX_Pin GPIO_PIN_2
#define DirX_GPIO_Port GPIOA
#define Y1_3_Pin GPIO_PIN_3
#define Y1_3_GPIO_Port GPIOA
#define Y0_3_Pin GPIO_PIN_4
#define Y0_3_GPIO_Port GPIOA
#define Y1_2_Pin GPIO_PIN_5
#define Y1_2_GPIO_Port GPIOA
#define Y0_2_Pin GPIO_PIN_6
#define Y0_2_GPIO_Port GPIOA
#define Y1_1_Pin GPIO_PIN_7
#define Y1_1_GPIO_Port GPIOA
#define Y0_1_Pin GPIO_PIN_8
#define Y0_1_GPIO_Port GPIOA
#define Y1_0_Pin GPIO_PIN_11
#define Y1_0_GPIO_Port GPIOA
#define Y0_0_Pin GPIO_PIN_12
#define Y0_0_GPIO_Port GPIOA
#define StepX_Pin GPIO_PIN_15
#define StepX_GPIO_Port GPIOA
#define StepX_EXTI_IRQn EXTI4_15_IRQn
#define X_0_Pin GPIO_PIN_4
#define X_0_GPIO_Port GPIOB
#define X_1_Pin GPIO_PIN_5
#define X_1_GPIO_Port GPIOB
#define X_2_Pin GPIO_PIN_6
#define X_2_GPIO_Port GPIOB
#define X_3_Pin GPIO_PIN_7
#define X_3_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define CONNECTORBOARD
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
