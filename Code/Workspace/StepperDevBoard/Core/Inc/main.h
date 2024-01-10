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
#define User_Button_Pin GPIO_PIN_13
#define User_Button_GPIO_Port GPIOC
#define User_Button_EXTI_IRQn EXTI4_15_IRQn
#define StepX_Pin GPIO_PIN_0
#define StepX_GPIO_Port GPIOA
#define DirX_Pin GPIO_PIN_1
#define DirX_GPIO_Port GPIOA
#define DirY_Pin GPIO_PIN_2
#define DirY_GPIO_Port GPIOA
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
#define ClockOut_Pin GPIO_PIN_0
#define ClockOut_GPIO_Port GPIOB
#define Y0_1_Pin GPIO_PIN_8
#define Y0_1_GPIO_Port GPIOA
#define Y1_0_Pin GPIO_PIN_9
#define Y1_0_GPIO_Port GPIOA
#define Y0_0_Pin GPIO_PIN_10
#define Y0_0_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define StepY_Pin GPIO_PIN_15
#define StepY_GPIO_Port GPIOA
#define StepY_EXTI_IRQn EXTI4_15_IRQn
#define X_0_Pin GPIO_PIN_4
#define X_0_GPIO_Port GPIOB
#define X_1_Pin GPIO_PIN_5
#define X_1_GPIO_Port GPIOB
#define X_2_Pin GPIO_PIN_6
#define X_2_GPIO_Port GPIOB
#define X_3_Pin GPIO_PIN_7
#define X_3_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
