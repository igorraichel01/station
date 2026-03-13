/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           :  main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32f4xx_hal.h"

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
#define SET_Pin GPIO_PIN_0
#define SET_GPIO_Port GPIOA
#define UP_Pin GPIO_PIN_1
#define UP_GPIO_Port GPIOA
#define DHT_DATA_Pin GPIO_PIN_2
#define DHT_DATA_GPIO_Port GPIOA
#define DOWN_Pin GPIO_PIN_3
#define DOWN_GPIO_Port GPIOA
#define LEFT_Pin GPIO_PIN_4
#define LEFT_GPIO_Port GPIOA
#define RIGHT_Pin GPIO_PIN_5
#define RIGHT_GPIO_Port GPIOA
#define FLASH_CS_Pin GPIO_PIN_12
#define FLASH_CS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
/* USER CODE BEGIN Private defines */

// ✅ Bateria
#define BATTERY_ADC_CHANNEL ADC_CHANNEL_4
#define BATTERY_VREF 3.3f
#define BATTERY_ADC_MAX 4095.0f
#define BATTERY_VOLTAGE_DIVIDER 2.0f  // Divisor 10k/10k = metade

#define BATTERY_FULL 4.2f
#define BATTERY_EMPTY 3.0f

/* USER CODE END Private defines */

// ✅ TEMPOS DE AQUISIÇÃO (3 OPÇÕES)
typedef enum {
    ACQ_30M = 0,
    ACQ_1H,
    ACQ_TOTAL_OPTIONS
} AcquisitionTime_t;
typedef struct {
    AcquisitionTime_t value;
    uint32_t milliseconds;
    const char* display_text;
} AcqTimeConfig_t;

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
