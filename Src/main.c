/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "st7735.h"
#include "gfx_functions.h"
#include "analogSystemConstants.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FSFORMAT_FIXED2D2 	0

#define ISFORMAT_4096 		0

#define DISPMODE_DEFAULT 	0
#define DISPMODE_GRAPH		1
#define DISPMODE_ADC		2
#define DISPMODE_EDIT	 	3
#define DISPMODE_NONE		99

#define EDITMODE_TIMEOUT	75

#define ENC_PRESCALE 		2

#define VOUT_TARGET_MAX		15.0
#define VOUT_TARGET_MIN		0.01

#define IOUT_TARGET_MAX		3.5
#define IOUT_TARGET_MIN		0.01

#define EDITMODE_FLASHRATE	4

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define digit2Char(v) 			((char)(v % 10) + '0')

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

DAC_HandleTypeDef hdac1;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim16;

/* USER CODE BEGIN PV */
uint16_t adc[ADC_BUF_LEN];

float VOUT_TARGET = 5.0;
float IOUT_TARGET = 0.5;

float NEW_VOUT_TARGET = 0;
float NEW_IOUT_TARGET = 0;


int displayMode = DISPMODE_DEFAULT, oldDisplayMode = DISPMODE_NONE, exitEditMode = DISPMODE_DEFAULT;
int editModeDigit = 0;
int editModeTime = 0;

int graphVOUT[32];
int graphIOUT[32];

extern float ISENSE, VOUT, POUT;

extern unsigned long adc_VOUT, adc_ISENS1, adc_ISENS2;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_DAC1_Init(void);
static void MX_SPI1_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM16_Init(void);
/* USER CODE BEGIN PFP */
void prepareFormattedFloatString(char *strbuf, float val, int format);
void prepareFormattedIntString(char *strbuf, int val, int format);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
bool UIBTN_Pressed = false, ENCBTN_Pressed = false, firstRun = true;
int16_t ENC_Position = 0;
int16_t lastENCPosition = 0;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	char strbuf[80];

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
  MX_DMA_Init();
  MX_DAC1_Init();
  MX_SPI1_Init();
  MX_ADC1_Init();
  MX_TIM16_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
  HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2000);

  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc, ADC_BUF_LEN);

  HAL_TIM_Base_Start_IT(&htim16);

  ST7735_Init(1);
  fillScreen(BLACK);

  // Clear graph mode history to minimum values (in Y offset)
  for (int i=0; i<32; i++)
  {
	  graphVOUT[i] = 58;
	  graphIOUT[i] = 123;
  }

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  // {{ HIGH PRIORITY }}
	  // None.

	  // {{ LOW PRIORITY }}
	  // TODO: Dynamic scaling for graph vertical ranges.
	  // TODO: Dynamic redraw over minor grid-lines to avoid flicker in graph mode.
	  // TODO: Multi-stage KP/KI values to provide better stability at low ISENSE + high VOUT


	  // Whenever the button ISR detects a keypress on SW1, change display mode or digit (when in EDIT mode)
	  if (UIBTN_Pressed)
	  {
		  UIBTN_Pressed = false;
		  switch (displayMode)
		  {
		  	  case DISPMODE_DEFAULT:
		  		  displayMode = DISPMODE_GRAPH;
		  		  break;
		  	  case DISPMODE_GRAPH:
		  		  displayMode = DISPMODE_ADC;
		  		  break;
		  	  case DISPMODE_ADC:
		  		  displayMode = DISPMODE_DEFAULT;
		  		  break;
		  	  case DISPMODE_EDIT:
		  	  	  editModeTime = 0;
		  	  	  editModeDigit++;
		  	  	  if (editModeDigit >= 7)
		  	  	  {
		  	  		  editModeDigit = 0;
		  	  	  }
		  	  	  break;
		  }
	  }

	  // If the encoder button is pressed and we are in EDIT mode, commit the VOUT and IOUT target changes, then return to
	  // the default view.
	  if (ENCBTN_Pressed)
	  {
		  ENCBTN_Pressed = false;
		  // In edit mode we edit a copy of the current targets, but only commit on actual press of commit button, not timeout
		  if (displayMode == DISPMODE_EDIT)
		  {
			  // Commit changes to targets upon press of encoder button
			  VOUT_TARGET = NEW_VOUT_TARGET;
			  IOUT_TARGET = NEW_IOUT_TARGET;
			  if (exitEditMode != DISPMODE_EDIT)
				  displayMode = exitEditMode;
			  ENCBTN_Pressed = false;
		  }
		  else
		  {
			  // Switch to EDIT mode from any other mode
	  		  NEW_VOUT_TARGET = VOUT_TARGET;
			  NEW_IOUT_TARGET = IOUT_TARGET;
	  		  editModeDigit = 0;
	  		  exitEditMode = displayMode;
	  		  editModeTime = 0;
	  		  displayMode = DISPMODE_EDIT;
		  }
	  }

	  // Whenever we change display modes, clear the old fixed text and redraw any new fixed text - this way, we only draw
	  // the elements below once on screen change and not every display update.
	  if (displayMode != oldDisplayMode)
	  {
		  oldDisplayMode = displayMode;
		  // Different screen modality; clear old text
		  fillScreen(BLACK);
		  switch (displayMode)
		  {
		  case DISPMODE_DEFAULT:
			  ST7735_WriteString(100, 10, "V", Font_16x26, GREEN, BLACK);
			  ST7735_WriteString(100, 55, "A", Font_16x26, RED, BLACK);
			  ST7735_WriteString(100, 100, "W", Font_16x26, color565(255,127,39), BLACK);
			  ST7735_WriteString(20, 37, "TARGET: ", Font_7x10, WHITE, BLACK);
			  ST7735_WriteString(110, 37, "V", Font_7x10, WHITE, BLACK);
			  ST7735_WriteString(20, 82, "LIMIT: ", Font_7x10, WHITE, BLACK);
			  ST7735_WriteString(110, 82, "A", Font_7x10, WHITE, BLACK);
			  break;
		  case DISPMODE_GRAPH:
			  ST7735_WriteString(28, 0,  "REAL-TIME VOUT", Font_7x10, YELLOW, BLACK);
			  ST7735_WriteString(28, 64, "REAL-TIME IOUT", Font_7x10, YELLOW, BLACK);

			  ST7735_WriteString(0, 6, "16.0", Font_7x10, BLUE, BLACK);
			  ST7735_WriteString(0, 18, "12.0", Font_7x10, BLUE, BLACK);
			  ST7735_WriteString(7, 30, "8.0", Font_7x10, BLUE, BLACK);
			  ST7735_WriteString(7, 42, "4.0", Font_7x10, BLUE, BLACK);
			  ST7735_WriteString(7, 54, "0.0", Font_7x10, BLUE, BLACK);

			  ST7735_WriteString(7, 79, "3.0", Font_7x10, BLUE, BLACK);
			  ST7735_WriteString(7, 93, "2.0", Font_7x10, BLUE, BLACK);
			  ST7735_WriteString(7, 107, "1.0", Font_7x10, BLUE, BLACK);
			  ST7735_WriteString(7, 118, "0.0", Font_7x10, BLUE, BLACK);
			  break;
		  case DISPMODE_ADC:
			  ST7735_WriteString(60,  1, "Raw ADC Values", Font_7x10, GREEN, BLACK);
  			  ST7735_WriteString(5,  1, "VOUT", Font_7x10, GREEN, BLACK);
  			  ST7735_WriteString(5,  40, "ISENSE (Direct)", Font_7x10, GREEN, BLACK);
  			  ST7735_WriteString(5,  80, "ISENSE (Filtered)", Font_7x10, GREEN, BLACK);
			  break;
		  case DISPMODE_EDIT:
			  ST7735_WriteString(30, 20, "TARGET VOUT: ", Font_7x10, WHITE, BLACK);
			  ST7735_WriteString(110, 30, "V", Font_16x26, GREEN, BLACK);
			  ST7735_WriteString(30, 60, "CURRENT LIMIT: ", Font_7x10, WHITE, BLACK);
			  ST7735_WriteString(110, 70, "A", Font_16x26, RED, BLACK);
		  }
	  }



	  int16_t encDelta = (ENC_Position-lastENCPosition);
	  if (encDelta != 0 && displayMode == DISPMODE_EDIT)
	  {
		  if (editModeTime % 2 == 0)
			  editModeTime = 0;
		  else
			  editModeTime = 1;

		  float totalDelta = 0.01;
		  if (editModeDigit <= 3)
		  {
			  for (int i=0; i<(3-editModeDigit); i++)
				  totalDelta *= 10;
			  totalDelta *= (encDelta/ENC_PRESCALE);
			  NEW_VOUT_TARGET += totalDelta;
			  if (NEW_VOUT_TARGET > VOUT_TARGET_MAX)
				  NEW_VOUT_TARGET = VOUT_TARGET_MAX;
			  if (NEW_VOUT_TARGET < VOUT_TARGET_MIN)
				  NEW_VOUT_TARGET = VOUT_TARGET_MIN;
		  }
		  else
		  {
			  for (int i=0; i<(6-editModeDigit); i++)
				  totalDelta *= 10;
			  totalDelta *= (encDelta/ENC_PRESCALE);
			  NEW_IOUT_TARGET += totalDelta;
			  if (NEW_IOUT_TARGET > IOUT_TARGET_MAX)
				  NEW_IOUT_TARGET = IOUT_TARGET_MAX;
			  if (NEW_IOUT_TARGET < IOUT_TARGET_MIN)
				  NEW_IOUT_TARGET = IOUT_TARGET_MIN;
		  }
	  }

	  lastENCPosition = ENC_Position;

	  // Exit limit editing mode if there is no user input for EDITMOD_TIMEOUT refreshes
	  // New VOUT_TARGET and IOUT_TARGET values are lost in this case
	  if (displayMode == DISPMODE_EDIT)
	  {
		  editModeTime++;
		  if (editModeTime >= EDITMODE_TIMEOUT)
		  {
			  editModeTime = 0;
			  displayMode = exitEditMode;
			  fillScreen(BLACK);
		  }
	  }

	  // Active display refresh code
	  switch (displayMode)
	  {
	  case DISPMODE_DEFAULT:
		  // Display Actual VOUT
		  prepareFormattedFloatString(strbuf, VOUT, FSFORMAT_FIXED2D2);
		  ST7735_WriteString(10,  10, strbuf, Font_16x26, GREEN, BLACK);

		  // Display Sensed IOUT
		  prepareFormattedFloatString(strbuf, ISENSE, FSFORMAT_FIXED2D2);
		  ST7735_WriteString(10,  55, strbuf, Font_16x26, RED, BLACK);

		  // Display Sensed POUT
		  prepareFormattedFloatString(strbuf, POUT, FSFORMAT_FIXED2D2);
		  ST7735_WriteString(10,  100, strbuf, Font_16x26, color565(255,127,39), BLACK);

		  // Display Target VOUT
		  prepareFormattedFloatString(strbuf, VOUT_TARGET, FSFORMAT_FIXED2D2);
		  ST7735_WriteString(72, 37, strbuf, Font_7x10, WHITE, BLACK);

		  // Display Target IOUT
		  prepareFormattedFloatString(strbuf, IOUT_TARGET, FSFORMAT_FIXED2D2);
		  ST7735_WriteString(72, 82, strbuf, Font_7x10, WHITE, BLACK);

		  // Control Mode Indication
		  if ((VOUT_TARGET - VOUT) < 0.1)
		  {
			  ST7735_WriteString(125, 20, "CV", Font_16x26, color565(0,255,0), BLACK);
			  ST7735_WriteString(125, 44, "CC", Font_16x26, color565(50,0,0), BLACK);
		  }
		  else
		  {
			  ST7735_WriteString(125, 20, "CV", Font_16x26, color565(0,50,0), BLACK);
			  ST7735_WriteString(125, 44, "CC", Font_16x26, color565(255,0,0), BLACK);
		  }
		  break;
	  case DISPMODE_GRAPH:
		  // Redraw outside squares of graph area
		  prepareFormattedFloatString(strbuf, VOUT, FSFORMAT_FIXED2D2);
		  ST7735_WriteString(124, 0,  strbuf, Font_7x10, GREEN, BLACK);

		  prepareFormattedFloatString(strbuf, ISENSE, FSFORMAT_FIXED2D2);
		  ST7735_WriteString(124, 64,  strbuf, Font_7x10, GREEN, BLACK);

		  drawRect(29, 11, 127, 48, WHITE);
		  drawRect(29, 75, 127, 49, WHITE);

		  // Redraw major gridlines
		  for (int i = 1; i < 8; i++)
		  {
			  if (i % 2 == 0)
				  drawLine(30, 12+i*6, 154, 12+i*6, LIGHTGREY);
			  else
				  drawLine(30, 12+i*6, 154, 12+i*6, DARKGREY);
		  }

		  for (int i = 1; i < 7; i++)
		  {
			  if (i % 2 == 1)
				  drawLine(30, 76+i*7, 154, 76+i*7, LIGHTGREY);
			  else
				  drawLine(30, 76+i*7, 154, 76+i*7, DARKGREY);
		  }

		  int newVOUT_Y = 58 - (int)(48*(VOUT / 16.0f));
		  int newIOUT_Y = 123 - (int)(49*(ISENSE / 3.5f));

		  if (newVOUT_Y < 10)
			  newVOUT_Y = 10;

		  if (newIOUT_Y < 74)
			  newIOUT_Y = 74;

		  if (newVOUT_Y != graphVOUT[0] || graphVOUT[0] != graphVOUT[1])
			  drawLine(30,graphVOUT[0],34,graphVOUT[1],BLACK);

		  if (newIOUT_Y != graphIOUT[0] || graphIOUT[0] != graphIOUT[1])
			  drawLine(30,graphIOUT[0],34,graphIOUT[1],BLACK);

		  for (int i=1; i<31; i++)
		  {
			  int x1 = 30+i*4;
			  int x2 = 30+(i+1)*4;

			  int y0 = graphVOUT[i-1];
			  int y1 = graphVOUT[i];
			  int y2 = graphVOUT[i+1];

			  if (y0 != y1 || y1 != y2)
				  drawLine(x1,y1,x2,y2,BLACK);

			  y0 = graphIOUT[i-1];
			  y1 = graphIOUT[i];
			  y2 = graphIOUT[i+1];

			  if (y0 != y1 || y1 != y2)
				  drawLine(x1,y1,x2,y2,BLACK);
		  }

		  for (int i=30; i>=0; i--)
		  {
			  graphVOUT[i+1] = graphVOUT[i];
			  graphIOUT[i+1] = graphIOUT[i];
		  }
		  graphVOUT[0] = newVOUT_Y;
		  graphIOUT[0] = newIOUT_Y;

		  for (int i=0; i<31; i++)
		  {
			  int x1 = 30+i*4;
			  int x2 = 30+(i+1)*4;

			  int y1 = graphVOUT[i];
			  int y2 = graphVOUT[i+1];
			  drawLine(x1,y1,x2,y2,RED);

			  y1 = graphIOUT[i];
			  y2 = graphIOUT[i+1];
			  drawLine(x1,y1,x2,y2,RED);
		  }

		  break;
	  case DISPMODE_ADC:
		  // Raw VOUT ADC Value
		  prepareFormattedFloatString(strbuf, ((float)(adc_VOUT) / ADC_RES) * ADC_REF * ADC_VOUT_VDIV, FSFORMAT_FIXED2D2);
		  ST7735_WriteString(5,  12, strbuf, Font_16x26, WHITE, BLACK);
		  // Raw ISENSE (unfiltered) ADC value
		  prepareFormattedIntString(strbuf, adc_ISENS1, ISFORMAT_4096);
		  ST7735_WriteString(5,  52, strbuf, Font_16x26, WHITE, BLACK);
		  // Raw ISENSE (filtered) ADC value
		  prepareFormattedIntString(strbuf, adc_ISENS2, ISFORMAT_4096);
		  ST7735_WriteString(5,  92, strbuf, Font_16x26, WHITE, BLACK);
		  break;
	  case DISPMODE_EDIT:
		  // Display target VOUT
		  prepareFormattedFloatString(strbuf, NEW_VOUT_TARGET, FSFORMAT_FIXED2D2);
		  if (strbuf[0] == ' ')
			  strbuf[0] = '0';

		  if (editModeTime % EDITMODE_FLASHRATE == 0)
		  {
			  if (editModeDigit <= 1)
				  strbuf[editModeDigit] = ' ';
			  else if (editModeDigit <= 3)
				  strbuf[editModeDigit+1] = ' ';
		  }
		  ST7735_WriteString(20,  30, strbuf, Font_16x26, GREEN, BLACK);

		  // Display maximum IOUT
		  prepareFormattedFloatString(strbuf, NEW_IOUT_TARGET, FSFORMAT_FIXED2D2);
		  if (editModeTime % EDITMODE_FLASHRATE == 0)
		  {
			  if (editModeDigit == 4)
				  strbuf[1] = ' ';
			  else if (editModeDigit >= 5)
				  strbuf[editModeDigit-2] = ' ';

		  }
		  ST7735_WriteString(20,  70, strbuf, Font_16x26, RED, BLACK);
		  break;
	  }
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
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.LowPowerAutoPowerOff = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = 4;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_3CYCLES_5;
  hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_3CYCLES_5;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief DAC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC1_Init(void)
{

  /* USER CODE BEGIN DAC1_Init 0 */

  /* USER CODE END DAC1_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC1_Init 1 */

  /* USER CODE END DAC1_Init 1 */

  /** DAC Initialization
  */
  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC1_Init 2 */

  /* USER CODE END DAC1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_1LINE;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4; //SPI_BAUDRATEPRESCALER_4
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM16 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM16_Init(void)
{

  /* USER CODE BEGIN TIM16_Init 0 */

  /* USER CODE END TIM16_Init 0 */

  /* USER CODE BEGIN TIM16_Init 1 */

  /* USER CODE END TIM16_Init 1 */
  htim16.Instance = TIM16;
  htim16.Init.Prescaler = 0;
  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim16.Init.Period = 39999;  //39999
  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim16.Init.RepetitionCounter = 0;
  htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM16_Init 2 */

  /* USER CODE END TIM16_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_CS_Pin|LCD_DC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LCD_CS_Pin LCD_DC_Pin */
  GPIO_InitStruct.Pin = LCD_CS_Pin|LCD_DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_RS_Pin */
  GPIO_InitStruct.Pin = LCD_RS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_RS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : UI_BTN_Pin */
  GPIO_InitStruct.Pin = UI_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(UI_BTN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ENCSW_Pin */
  GPIO_InitStruct.Pin = ENCSW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(ENCSW_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ENCB_Pin ENCA_Pin */
  GPIO_InitStruct.Pin = ENCB_Pin|ENCA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */


void prepareFormattedFloatString(char *strbuf, float val, int format)
{
	switch (format)
	{
	case FSFORMAT_FIXED2D2:
		val *= 100;
		uint16_t val_i = (uint16_t)val;

		strbuf[4] = digit2Char(val_i); val_i /= 10;
		strbuf[3] = digit2Char(val_i); val_i /= 10;
		strbuf[2] = '.';
		strbuf[1] = digit2Char(val_i); val_i /= 10;
		strbuf[0] = digit2Char(val_i);

		if (strbuf[0] == '0')
			strbuf[0] = ' ';

		strbuf[5] = 0x00;

		break;
	}
}


void prepareFormattedIntString(char *strbuf, int val, int format)
{
	switch (format)
	{
	case ISFORMAT_4096:
		strbuf[3] = digit2Char(val); val /= 10;
		strbuf[2] = digit2Char(val); val /= 10;
		strbuf[1] = digit2Char(val); val /= 10;
		strbuf[0] = digit2Char(val);

		strbuf[4] = 0x00;
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
