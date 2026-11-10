/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    dac.c
  * @brief   This file provides code for the configuration
  *          of the DAC instances.
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
#include "dac.h"

/* USER CODE BEGIN 0 */
static TIM_HandleTypeDef sineTimer;
static DMA_HandleTypeDef sineDma;

/* One cycle, centered at code 2048 with a peak amplitude of 1600 codes. */
static uint32_t sineSamples[] = {
  2048, 2271, 2489, 2699, 2896, 3076, 3237, 3374, 3486,
  3570, 3624, 3647, 3639, 3600, 3531, 3434, 3309, 3159,
  2988, 2799, 2595, 2381, 2160, 1936, 1715, 1501, 1297,
  1108, 937, 787, 662, 565, 496, 457, 449, 472,
  526, 610, 722, 859, 1020, 1200, 1397, 1607, 1825,
};

/* USER CODE END 0 */

DAC_HandleTypeDef hdac;

/* DAC init function */
void MX_DAC_Init(void)
{

  /* USER CODE BEGIN DAC_Init 0 */

  /* USER CODE END DAC_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC_Init 1 */

  /* USER CODE END DAC_Init 1 */

  /** DAC Initialization
  */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT2 config
  */
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC_Init 2 */

  /* USER CODE END DAC_Init 2 */

}

void HAL_DAC_MspInit(DAC_HandleTypeDef* dacHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(dacHandle->Instance==DAC)
  {
  /* USER CODE BEGIN DAC_MspInit 0 */

  /* USER CODE END DAC_MspInit 0 */
    /* DAC clock enable */
    __HAL_RCC_DAC_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**DAC GPIO Configuration
    PA4     ------> DAC_OUT1
    PA5     ------> DAC_OUT2
    */
    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN DAC_MspInit 1 */

  /* USER CODE END DAC_MspInit 1 */
  }
}

void HAL_DAC_MspDeInit(DAC_HandleTypeDef* dacHandle)
{

  if(dacHandle->Instance==DAC)
  {
  /* USER CODE BEGIN DAC_MspDeInit 0 */

  /* USER CODE END DAC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_DAC_CLK_DISABLE();

    /**DAC GPIO Configuration
    PA4     ------> DAC_OUT1
    PA5     ------> DAC_OUT2
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4|GPIO_PIN_5);

  /* USER CODE BEGIN DAC_MspDeInit 1 */

  /* USER CODE END DAC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void DAC_StartSine10kHz(void)
{
  DAC_ChannelConfTypeDef channel = {0};
  TIM_MasterConfigTypeDef trigger = {0};

  __HAL_RCC_TIM6_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* TIM6 runs at 90 MHz. 200 ticks/sample and 45 samples/cycle give 10 kHz. */
  sineTimer.Instance = TIM6;
  sineTimer.Init.Prescaler = 0;
  sineTimer.Init.Period = 199;
  if (HAL_TIM_Base_Init(&sineTimer) != HAL_OK)
  {
    Error_Handler();
  }

  trigger.MasterOutputTrigger = TIM_TRGO_UPDATE;
  trigger.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&sineTimer, &trigger) != HAL_OK)
  {
    Error_Handler();
  }

  sineDma.Instance = DMA1_Stream5;
  sineDma.Init.Channel = DMA_CHANNEL_7;
  sineDma.Init.Direction = DMA_MEMORY_TO_PERIPH;
  sineDma.Init.PeriphInc = DMA_PINC_DISABLE;
  sineDma.Init.MemInc = DMA_MINC_ENABLE;
  sineDma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  sineDma.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  sineDma.Init.Mode = DMA_CIRCULAR;
  sineDma.Init.Priority = DMA_PRIORITY_HIGH;
  sineDma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  if (HAL_DMA_Init(&sineDma) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_LINKDMA(&hdac, DMA_Handle1, sineDma);

  channel.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
  channel.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &channel, DAC_CHANNEL_1) != HAL_OK ||
      HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, sineSamples[0]) != HAL_OK ||
      HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, sineSamples,
                        sizeof(sineSamples) / sizeof(sineSamples[0]), DAC_ALIGN_12B_R) != HAL_OK)
  {
    Error_Handler();
  }

  /* Circular playback only needs interrupts for errors. */
  __HAL_DMA_DISABLE_IT(&sineDma, DMA_IT_HT | DMA_IT_TC);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);

  if (HAL_TIM_Base_Start(&sineTimer) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE END 1 */

