/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32g0xx_it.c
  * @brief   Interrupt Service Routines.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32g0xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdbool.h>
#include "analogSystemConstants.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define avg4(v1, v2, v3, v4) 	((v1+v2+v3+v4)/4)
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint8_t lastENCState = 00;
float ISENSE, VOUT, POUT;
float error = 0, lastError = 0, integral = 0, derivative, output;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_adc1;
extern DAC_HandleTypeDef hdac1;
extern TIM_HandleTypeDef htim16;
/* USER CODE BEGIN EV */
extern bool UIBTN_Pressed;
extern bool ENCBTN_Pressed;
extern int16_t ENC_Position;

extern float VOUT_TARGET;
extern float IOUT_TARGET;
extern uint16_t adc[];

unsigned long adc_VOUT = 0, adc_ISENS1 = 0, adc_ISENS2 = 0;

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M0+ Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVC_IRQn 0 */

  /* USER CODE END SVC_IRQn 0 */
  /* USER CODE BEGIN SVC_IRQn 1 */

  /* USER CODE END SVC_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32G0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32g0xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles EXTI line 4 to 15 interrupts.
  */
void EXTI4_15_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI4_15_IRQn 0 */
   if(__HAL_GPIO_EXTI_GET_FLAG(UI_BTN_Pin))
   {
	   UIBTN_Pressed = true;
   }
   else if (__HAL_GPIO_EXTI_GET_FLAG(ENCSW_Pin))
   {
	   ENCBTN_Pressed = true;
   }
   else if (__HAL_GPIO_EXTI_GET_FLAG(ENCA_Pin) || __HAL_GPIO_EXTI_GET_FLAG(ENCB_Pin))
   {
	   uint8_t A = HAL_GPIO_ReadPin(GPIOA, ENCA_Pin) == 1 ? 2 : 0;
	   uint8_t B = HAL_GPIO_ReadPin(GPIOA, ENCB_Pin) == 1 ? 1 : 0;
	   uint8_t ENCState = A+B;

	   if ((ENCState == 1 && lastENCState == 0) || (ENCState == 3 && lastENCState == 1) || (ENCState == 2 && lastENCState == 3) || (ENCState == 0 && lastENCState == 2))
	   {
		   ENC_Position--;
	   }
	   else if ((ENCState == 0 && lastENCState == 1) || (ENCState == 2 && lastENCState == 0) || (ENCState == 3 && lastENCState == 2) || (ENCState == 1 && lastENCState == 3))
	   {
		   ENC_Position++;
	   }

	   lastENCState = ENCState;
   }

  /* USER CODE END EXTI4_15_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(UI_BTN_Pin);
  HAL_GPIO_EXTI_IRQHandler(ENCSW_Pin);
  HAL_GPIO_EXTI_IRQHandler(ENCB_Pin);
  HAL_GPIO_EXTI_IRQHandler(ENCA_Pin);
  /* USER CODE BEGIN EXTI4_15_IRQn 1 */

  /* USER CODE END EXTI4_15_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel 1 interrupt.
  */
void DMA1_Channel1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel1_IRQn 0 */

  /* USER CODE END DMA1_Channel1_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_adc1);
  /* USER CODE BEGIN DMA1_Channel1_IRQn 1 */

  /* USER CODE END DMA1_Channel1_IRQn 1 */
}

/**
  * @brief This function handles TIM6, DAC and LPTIM1 global Interrupts.
  */
void TIM6_DAC_LPTIM1_IRQHandler(void)
{
  /* USER CODE BEGIN TIM6_DAC_LPTIM1_IRQn 0 */



  /* USER CODE END TIM6_DAC_LPTIM1_IRQn 0 */
  HAL_DAC_IRQHandler(&hdac1);
  /* USER CODE BEGIN TIM6_DAC_LPTIM1_IRQn 1 */


  /* USER CODE END TIM6_DAC_LPTIM1_IRQn 1 */
}

/**
  * @brief This function handles TIM16 global interrupt.
  */
void TIM16_IRQHandler(void)
{
  /* USER CODE BEGIN TIM16_IRQn 0 */

	// START CONTROL LOOP UPDATE

	// Calculate digital LPF values for ADC inputs
	adc_VOUT = 0; adc_ISENS1 = 0; adc_ISENS2 = 0;
	for (int i=0; i<ADC_BUF_LEN; i+=4)
	{
		adc_VOUT += adc[i];
		adc_ISENS1 += adc[i+2];
		adc_ISENS2 += adc[i+3];
	}
	adc_VOUT /= (ADC_BUF_LEN/4);
	adc_ISENS1 /= (ADC_BUF_LEN/4);
	adc_ISENS2 /= (ADC_BUF_LEN/4);

	// Apply mapping of ADC values to VOUT
	VOUT = ( ((float)adc_VOUT) / ADC_RES) * ADC_REF * ADC_VOUT_VDIV;
	VOUT = ADC_VOUT_CAL_SLOPE * (VOUT + ADC_VOUT_CAL_OFFSET);

	// Set ISENSE using either filtered or unfiltered ADC input
#ifdef USE_FILTERED_ISENSE
	ISENSE = ADC_ISENSE_CAL_SLOPE*((float)adc_ISENS2) + ADC_ISENSE_CAL_OFFSET;
#else
	ISENSE = ADC_ISENSE_CAL_SLOPE*((float)adc_ISENS1) + ADC_ISENSE_CAL_OFFSET;
#endif

	// Ignore incorrect ISENSE values (can happen with poor calibration at small ISENSE)
	if (ISENSE < 0)
	  ISENSE = 0;

	POUT = VOUT*ISENSE;

	// Current sense control loop

	// Calculate Error = Output - Reference
	lastError = error;
	error = (ISENSE - IOUT_TARGET);

	float PID = 0;


#ifndef DISABLE_CURRENT_LIMIT
	// Numerical methods calculation for integral, derivative
	integral += (error+lastError)/2 * TIMESCALE;
	derivative = (error-lastError)/TIMESCALE;

	// Calculate PID summation
	PID = KP*error + KI*integral + KD*derivative;
#endif

	// If current has dropped below current limit threshold, disable current control
	if (PID < -10)
	{
		PID = 0;
		integral = 0;
	}

	// VOUT is a fixed mapping to DAC value, plus small correction from ISENSE, and PID control loop for ISENSE limit
	output = DAC_CAL_SLOPE*VOUT_TARGET + DAC_CAL_OFFSET + PID;// - 15.0f*ISENSE;

	// Clip to allowable DAC output values
	if (output > DAC_OUT_MAX)
	  output = DAC_OUT_MAX;
	if (output < DAC_OUT_MIN)
	  output = DAC_OUT_MIN;

	// Send update to DAC
	HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (uint16_t)(output));
	// END CONTROL LOOP UPDATE

  /* USER CODE END TIM16_IRQn 0 */
  HAL_TIM_IRQHandler(&htim16);
  /* USER CODE BEGIN TIM16_IRQn 1 */

  /* USER CODE END TIM16_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
