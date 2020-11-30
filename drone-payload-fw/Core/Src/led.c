/**
  ******************************************************************************
	WIFI_MNGR
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#include <stdbool.h>
#include "main.h"
#include "led.h"

#define RGB_TO_ENUM(R,G,B)	((!(R)) + ((!(G))<<1) + ((!(B))<<2) )

/* ================
void LED_Init(void)
================ */
void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/*Configure GPIO pins : PCPin PCPin PCPin */
	GPIO_InitStruct.Pin = LED_R_Pin|LED_G_Pin|LED_B_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	LED_SetState(eLedStates_all_off);
}

/* ================
void LED_SetState( eLedStates a_eLedStates)
================ */
void LED_SetState( eLedStates a_eLedStates)
{
	switch (a_eLedStates) {
		case eLedStates_all_off:
		{
			HAL_GPIO_WritePin(GPIOC, LED_R_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOC, LED_G_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOC, LED_B_Pin, GPIO_PIN_SET);
		}
			break;
		case eLedStates_red:
		{
			HAL_GPIO_WritePin(GPIOC, LED_R_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, LED_G_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOC, LED_B_Pin, GPIO_PIN_SET);
		}
			break;
		case eLedStates_green:
		{
			HAL_GPIO_WritePin(GPIOC, LED_R_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOC, LED_G_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, LED_B_Pin, GPIO_PIN_SET);
		}
			break;
		case eLedStates_blue:
		{
			HAL_GPIO_WritePin(GPIOC, LED_R_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOC, LED_G_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOC, LED_B_Pin, GPIO_PIN_RESET);
		}
			break;
		case eLedStates_red_green:
		{
			HAL_GPIO_WritePin(GPIOC, LED_R_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, LED_G_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, LED_B_Pin, GPIO_PIN_SET);
		}
			break;
		case eLedStates_red_blue:
		{
			HAL_GPIO_WritePin(GPIOC, LED_R_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, LED_G_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOC, LED_B_Pin, GPIO_PIN_RESET);
		}
			break;
		case eLedStates_blue_green:
		{
			HAL_GPIO_WritePin(GPIOC, LED_R_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOC, LED_G_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, LED_B_Pin, GPIO_PIN_RESET);
		}
			break;
		case eLedStates_all_on:
		{
			HAL_GPIO_WritePin(GPIOC, LED_R_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, LED_G_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, LED_B_Pin, GPIO_PIN_RESET);
		}
			break;
		default: // all off
			HAL_GPIO_WritePin(GPIOC, LED_R_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOC, LED_G_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOC, LED_B_Pin, GPIO_PIN_SET);
			break;
	}
}

/* ================
eLedStates LED_GetState(void)
================ */
eLedStates LED_GetState(void)
{
	return (eLedStates)(RGB_TO_ENUM(HAL_GPIO_ReadPin(GPIOC, LED_R_Pin),\
									HAL_GPIO_ReadPin(GPIOC, LED_G_Pin),\
									HAL_GPIO_ReadPin(GPIOC, LED_B_Pin)));
}

// ================= TEST SECTION ================
//uint8_t res = true;
//for(eLedStates LedStates = eLedStates_all_off; LedStates<= eLedStates_all_on; LedStates++)
//{
//	HAL_Delay(200);
//	LED_SetState(LedStates);
//	if(LedStates != LED_GetState())
//	{
//		res = false;
//		break;
//	}
//}


/*****END OF FILE****/
