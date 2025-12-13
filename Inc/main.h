/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "stm32g0xx_hal.h"

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
#define VOUT_Pin GPIO_PIN_0
#define VOUT_GPIO_Port GPIOA
#define ISENSE_Pin GPIO_PIN_1
#define ISENSE_GPIO_Port GPIOA
#define VSENSE_Pin GPIO_PIN_2
#define VSENSE_GPIO_Port GPIOA
#define VSENSE_F_Pin GPIO_PIN_3
#define VSENSE_F_GPIO_Port GPIOA
#define DAC_OUT_Pin GPIO_PIN_4
#define DAC_OUT_GPIO_Port GPIOA
#define LCD_CS_Pin GPIO_PIN_0
#define LCD_CS_GPIO_Port GPIOB
#define LCD_DC_Pin GPIO_PIN_1
#define LCD_DC_GPIO_Port GPIOB
#define LCD_RS_Pin GPIO_PIN_8
#define LCD_RS_GPIO_Port GPIOA
#define UI_BTN_Pin GPIO_PIN_6
#define UI_BTN_GPIO_Port GPIOC
#define UI_BTN_EXTI_IRQn EXTI4_15_IRQn
#define ENCSW_Pin GPIO_PIN_11
#define ENCSW_GPIO_Port GPIOA
#define ENCSW_EXTI_IRQn EXTI4_15_IRQn
#define ENCB_Pin GPIO_PIN_12
#define ENCB_GPIO_Port GPIOA
#define ENCB_EXTI_IRQn EXTI4_15_IRQn
#define ENCA_Pin GPIO_PIN_15
#define ENCA_GPIO_Port GPIOA
#define ENCA_EXTI_IRQn EXTI4_15_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
