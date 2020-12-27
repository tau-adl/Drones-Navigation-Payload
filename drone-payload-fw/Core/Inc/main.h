/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdbool.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */


/** Wi-Fi Settings */

#define PROJ_SHA			"0x53a63a68b7373bf06e1bc09205d579fdcc81de3e"




//#define MAIN_WIFI_M2M_PRODUCT_NAME        "NMCTemp"
//#define MAIN_WIFI_M2M_SERVER_IP           0xC0A80079 /* 255.255.255.255 */
#define ATWINC_AP_IP_BYTE0	0xC0 /* 192 */
#define ATWINC_AP_IP_BYTE1	0xA8 /* 168 */
#define ATWINC_AP_IP_BYTE2	0x01 /* 1 */
#define ATWINC_AP_IP_BYTE3	0x01 /* 1 */



#define MAIN_WIFI_M2M_SERVER_PORT         (6666)
//#define MAIN_WIFI_M2M_REPORT_INTERVAL     (1000)

#define MAIN_WIFI_M2M_BUFFER_SIZE      1024 //SOCKET_BUFFER_MAX_LENGTH

#define IPV4_BYTE(val, index)          ((val >> (index * 8)) & 0xFF)

/** AP mode Settings */
#define MAIN_WLAN_SSID                 "WINC1500_AP" /* < SSID */
#define MAIN_WLAN_AUTH                 M2M_WIFI_SEC_OPEN /* < Security manner */
#define MAIN_WLAN_CHANNEL              (6) /* < Channel number */
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_R_Pin GPIO_PIN_0
#define LED_R_GPIO_Port GPIOC
#define LED_G_Pin GPIO_PIN_1
#define LED_G_GPIO_Port GPIOC
#define LED_B_Pin GPIO_PIN_2
#define LED_B_GPIO_Port GPIOC
#define WIFI_CHIP_EN_Pin GPIO_PIN_5
#define WIFI_CHIP_EN_GPIO_Port GPIOA
#define SPI2_NSS_Pin GPIO_PIN_12
#define SPI2_NSS_GPIO_Port GPIOB
#define WIFI_RESET_N_Pin GPIO_PIN_12
#define WIFI_RESET_N_GPIO_Port GPIOD
#define WIFI_INT_N_Pin GPIO_PIN_13
#define WIFI_INT_N_GPIO_Port GPIOD
#define WIFI_INT_N_EXTI_IRQn EXTI15_10_IRQn
#define DCMI_PCLK_Pin GPIO_PIN_8
#define DCMI_PCLK_GPIO_Port GPIOA
#define DCMI_RESET_Pin GPIO_PIN_10
#define DCMI_RESET_GPIO_Port GPIOA
#define SD_CD_Pin GPIO_PIN_0
#define SD_CD_GPIO_Port GPIOD
#define DCMI_PWDN_Pin GPIO_PIN_7
#define DCMI_PWDN_GPIO_Port GPIOD
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
