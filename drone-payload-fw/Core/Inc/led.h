/**
  ******************************************************************************
	WIFI_MNGR
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __led_H
#define __led_H
#ifdef __cplusplus
 extern "C" {
#endif

#define LED_R_Pin GPIO_PIN_0
#define LED_R_GPIO_Port GPIOC
#define LED_G_Pin GPIO_PIN_1
#define LED_G_GPIO_Port GPIOC
#define LED_B_Pin GPIO_PIN_2
#define LED_B_GPIO_Port GPIOC

typedef enum eLedStates_name
{
	eLedStates_all_off = 0,
	eLedStates_red,
	eLedStates_green,
	eLedStates_red_green,
	eLedStates_blue,
	eLedStates_red_blue,
	eLedStates_blue_green,
	eLedStates_all_on,
}eLedStates;


void LED_Init(void);
void LED_SetState( eLedStates a_eLedStates);
eLedStates LED_GetState(void);



#ifdef __cplusplus
}
#endif
#endif /*__ led_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
