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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h> // add 231015 kenGwon
#include <string.h> // add 231015 kenGwon
#include <stdlib.h> // add 231015 kenGwon
#include <stdbool.h> // add 231015 kenGwon
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
#define USER_Btn_Pin GPIO_PIN_13
#define USER_Btn_GPIO_Port GPIOC
#define MCO_Pin GPIO_PIN_0
#define MCO_GPIO_Port GPIOH
#define RMII_MDC_Pin GPIO_PIN_1
#define RMII_MDC_GPIO_Port GPIOC
#define DHT11_Pin GPIO_PIN_0
#define DHT11_GPIO_Port GPIOA
#define RMII_REF_CLK_Pin GPIO_PIN_1
#define RMII_REF_CLK_GPIO_Port GPIOA
#define RMII_MDIO_Pin GPIO_PIN_2
#define RMII_MDIO_GPIO_Port GPIOA
#define BUZZER_Pin GPIO_PIN_3
#define BUZZER_GPIO_Port GPIOA
#define ULTRASONIC_TIM3_CH1_Pin GPIO_PIN_6
#define ULTRASONIC_TIM3_CH1_GPIO_Port GPIOA
#define RMII_CRS_DV_Pin GPIO_PIN_7
#define RMII_CRS_DV_GPIO_Port GPIOA
#define RMII_RXD0_Pin GPIO_PIN_4
#define RMII_RXD0_GPIO_Port GPIOC
#define RMII_RXD1_Pin GPIO_PIN_5
#define RMII_RXD1_GPIO_Port GPIOC
#define LD1_Pin GPIO_PIN_0
#define LD1_GPIO_Port GPIOB
#define FND_A_Pin GPIO_PIN_1
#define FND_A_GPIO_Port GPIOB
#define FND_B_Pin GPIO_PIN_2
#define FND_B_GPIO_Port GPIOB
#define ULTRASONIC_TRIGGER_Pin GPIO_PIN_12
#define ULTRASONIC_TRIGGER_GPIO_Port GPIOF
#define IN1_DCMOTOR_Pin GPIO_PIN_8
#define IN1_DCMOTOR_GPIO_Port GPIOE
#define IN2_DCMOTOR_Pin GPIO_PIN_9
#define IN2_DCMOTOR_GPIO_Port GPIOE
#define BUTTON3_Pin GPIO_PIN_10
#define BUTTON3_GPIO_Port GPIOE
#define BUTTON2_Pin GPIO_PIN_12
#define BUTTON2_GPIO_Port GPIOE
#define FAN_ROTATE_DERECTION_LED_Pin GPIO_PIN_13
#define FAN_ROTATE_DERECTION_LED_GPIO_Port GPIOE
#define BUTTON1_Pin GPIO_PIN_14
#define BUTTON1_GPIO_Port GPIOE
#define BUTTON0_Pin GPIO_PIN_15
#define BUTTON0_GPIO_Port GPIOE
#define BUTTON4_Pin GPIO_PIN_10
#define BUTTON4_GPIO_Port GPIOB
#define FND_G_Pin GPIO_PIN_12
#define FND_G_GPIO_Port GPIOB
#define RMII_TXD1_Pin GPIO_PIN_13
#define RMII_TXD1_GPIO_Port GPIOB
#define LD3_Pin GPIO_PIN_14
#define LD3_GPIO_Port GPIOB
#define FND_DP_Pin GPIO_PIN_15
#define FND_DP_GPIO_Port GPIOB
#define STLK_RX_Pin GPIO_PIN_8
#define STLK_RX_GPIO_Port GPIOD
#define STLK_TX_Pin GPIO_PIN_9
#define STLK_TX_GPIO_Port GPIOD
#define USB_PowerSwitchOn_Pin GPIO_PIN_6
#define USB_PowerSwitchOn_GPIO_Port GPIOG
#define USB_OverCurrent_Pin GPIO_PIN_7
#define USB_OverCurrent_GPIO_Port GPIOG
#define FND_D1_Pin GPIO_PIN_8
#define FND_D1_GPIO_Port GPIOC
#define FND_D2_Pin GPIO_PIN_9
#define FND_D2_GPIO_Port GPIOC
#define USB_SOF_Pin GPIO_PIN_8
#define USB_SOF_GPIO_Port GPIOA
#define USB_VBUS_Pin GPIO_PIN_9
#define USB_VBUS_GPIO_Port GPIOA
#define USB_ID_Pin GPIO_PIN_10
#define USB_ID_GPIO_Port GPIOA
#define USB_DM_Pin GPIO_PIN_11
#define USB_DM_GPIO_Port GPIOA
#define USB_DP_Pin GPIO_PIN_12
#define USB_DP_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define FND_D3_Pin GPIO_PIN_10
#define FND_D3_GPIO_Port GPIOC
#define FND_D4_Pin GPIO_PIN_11
#define FND_D4_GPIO_Port GPIOC
#define LEDBAR0_Pin GPIO_PIN_0
#define LEDBAR0_GPIO_Port GPIOD
#define LEDBAR1_Pin GPIO_PIN_1
#define LEDBAR1_GPIO_Port GPIOD
#define LEDBAR2_Pin GPIO_PIN_2
#define LEDBAR2_GPIO_Port GPIOD
#define LEDBAR3_Pin GPIO_PIN_3
#define LEDBAR3_GPIO_Port GPIOD
#define LEDBAR4_Pin GPIO_PIN_4
#define LEDBAR4_GPIO_Port GPIOD
#define LEDBAR5_Pin GPIO_PIN_5
#define LEDBAR5_GPIO_Port GPIOD
#define LEDBAR6_Pin GPIO_PIN_6
#define LEDBAR6_GPIO_Port GPIOD
#define LEDBAR7_Pin GPIO_PIN_7
#define LEDBAR7_GPIO_Port GPIOD
#define RMII_TX_EN_Pin GPIO_PIN_11
#define RMII_TX_EN_GPIO_Port GPIOG
#define RMII_TXD0_Pin GPIO_PIN_13
#define RMII_TXD0_GPIO_Port GPIOG
#define FND_C_Pin GPIO_PIN_3
#define FND_C_GPIO_Port GPIOB
#define FND_D_Pin GPIO_PIN_4
#define FND_D_GPIO_Port GPIOB
#define FND_E_Pin GPIO_PIN_5
#define FND_E_GPIO_Port GPIOB
#define FND_F_Pin GPIO_PIN_6
#define FND_F_GPIO_Port GPIOB
#define LD2_Pin GPIO_PIN_7
#define LD2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
