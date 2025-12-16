/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
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
#include "stm32wbxx_hal.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define CURRENT_ALERT_Pin GPIO_PIN_13
#define CURRENT_ALERT_GPIO_Port GPIOC
#define ENC_A_Pin GPIO_PIN_0
#define ENC_A_GPIO_Port GPIOC
#define BUTTON_Pin GPIO_PIN_1
#define BUTTON_GPIO_Port GPIOC
#define ENC_B_Pin GPIO_PIN_2
#define ENC_B_GPIO_Port GPIOC
#define HEATBED_Pin GPIO_PIN_0
#define HEATBED_GPIO_Port GPIOA
#define PT_CS_Pin GPIO_PIN_4
#define PT_CS_GPIO_Port GPIOA
#define BUZZER_Pin GPIO_PIN_8
#define BUZZER_GPIO_Port GPIOA
#define PT_DRDY_Pin GPIO_PIN_4
#define PT_DRDY_GPIO_Port GPIOC
#define FIELD_DETECT_Pin GPIO_PIN_5
#define FIELD_DETECT_GPIO_Port GPIOC
#define TEMP_ALERT_Pin GPIO_PIN_2
#define TEMP_ALERT_GPIO_Port GPIOB
#define I2C_SCL_Pin GPIO_PIN_10
#define I2C_SCL_GPIO_Port GPIOB
#define I2C_SDA_Pin GPIO_PIN_11
#define I2C_SDA_GPIO_Port GPIOB
#define LED_BLUE_Pin GPIO_PIN_0
#define LED_BLUE_GPIO_Port GPIOB
#define LED_GREEN_Pin GPIO_PIN_1
#define LED_GREEN_GPIO_Port GPIOB
#define LED_RED_Pin GPIO_PIN_4
#define LED_RED_GPIO_Port GPIOE
#define THERMO_CS_Pin GPIO_PIN_12
#define THERMO_CS_GPIO_Port GPIOB
#define THERMO_SCK_Pin GPIO_PIN_13
#define THERMO_SCK_GPIO_Port GPIOB
#define THERMO_MISO_Pin GPIO_PIN_14
#define THERMO_MISO_GPIO_Port GPIOB
#define FAULT_IN_Pin GPIO_PIN_15
#define FAULT_IN_GPIO_Port GPIOB
#define SINK_EN_Pin GPIO_PIN_6
#define SINK_EN_GPIO_Port GPIOC
#define CAP_MIS_Pin GPIO_PIN_10
#define CAP_MIS_GPIO_Port GPIOA
#define LED_COOLING_Pin GPIO_PIN_10
#define LED_COOLING_GPIO_Port GPIOC
#define LED_REFLOW_Pin GPIO_PIN_11
#define LED_REFLOW_GPIO_Port GPIOC
#define LED_SOAK_Pin GPIO_PIN_12
#define LED_SOAK_GPIO_Port GPIOC
#define LED_PREHEAT_Pin GPIO_PIN_0
#define LED_PREHEAT_GPIO_Port GPIOD
#define LED_PD_Pin GPIO_PIN_1
#define LED_PD_GPIO_Port GPIOD
#define PT_SCK_Pin GPIO_PIN_3
#define PT_SCK_GPIO_Port GPIOB
#define PT_MISO_Pin GPIO_PIN_4
#define PT_MISO_GPIO_Port GPIOB
#define PT_MOSI_Pin GPIO_PIN_5
#define PT_MOSI_GPIO_Port GPIOB
#define POWER_SCL_Pin GPIO_PIN_6
#define POWER_SCL_GPIO_Port GPIOB
#define POWER_SDA_Pin GPIO_PIN_7
#define POWER_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
