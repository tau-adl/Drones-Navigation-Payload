/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
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

#include "fatfs.h"

uint8_t retSD;    /* Return value for SD */
char SDPath[4];   /* SD logical drive path */
FATFS SDFatFS;    /* File system object for SD logical drive */
FIL SDFile;       /* File object for SD */

/* USER CODE BEGIN Variables */
static uint8_t isInitialized = 0;

void FS_Init(void);
/* USER CODE END Variables */

void MX_FATFS_Init(void)
{
  /*## FatFS: Link the SD driver ###########################*/
  retSD = FATFS_LinkDriver(&SD_Driver, SDPath);

  /* USER CODE BEGIN Init */
  /* additional user code for init */     

  FS_Init();
  /* USER CODE END Init */
}

/**
  * @brief  Gets Time from RTC
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{
  /* USER CODE BEGIN get_fattime */
  return 0;
  /* USER CODE END get_fattime */
}

/* USER CODE BEGIN Application */
uint8_t Fatfs_GetRetSD(void)
{
	return retSD;
}

/**
 * @brief  FS init and creating of filesystem
 * @retval None
 */
void FS_Init(void)
{
	if(BSP_SD_IsDetected())
	{
		FRESULT res;

		if (IS_NEED_SD_FORAMT)
		{
			printf("formatting SD\r\n");
			res = f_mkfs(SDPath, FM_FAT32, 0, workBuffer, sizeof(workBuffer));
			if (res != FR_OK)
			{
				Error_Handler();
			}

		}
		else
		{
			printf("no SD format\r\n");
		}
	}


	if (Fatfs_GetRetSD() == 0)
	{
		printf("FATFS Link Driver OK\r\n");
	}
	else
	{
		printf("FATFS Link Driver ERR\r\n");
	}
}


/**
 * @brief  SD init with BSP
 * @retval None
 */
void SD_Initialize(void)
{
	if (isInitialized == 0)
	{
		BSP_SD_Init();

		if(BSP_SD_IsDetected())
		{
			isInitialized = 1;
		}
	}
}


/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
