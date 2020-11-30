/**
  ******************************************************************************
  * @file    JPEG/JPEG_EncodingFromFLASH_DMA/Inc/encode_dma.h
  * @author  MCD Application Team
  * @brief   Header for encode_dma.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef INC_JPEG_ENCODE_DMA_H_
#define INC_JPEG_ENCODE_DMA_H_

/* Includes ------------------------------------------------------------------*/
//#include "main.h"


/* Exported variables --------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

void BMP_GetInfo(FIL * Filename, JPEG_ConfTypeDef *pInfo);

uint32_t JpegEncodeDMA_FromSdToSd(JPEG_HandleTypeDef *hjpeg, FIL *bmpfile, FIL *jpgfile);
uint32_t JpegEncodeDMA_FromRamToSd(JPEG_HandleTypeDef *hjpeg, uint8_t* FrameBuff, FIL *jpgfile);
uint32_t JpegEncodeDMA_FromRamToRam(JPEG_HandleTypeDef *hjpeg, uint8_t* InFrameBuff, uint8_t* OutFrameBuff);

uint32_t JpegEncodeDMA_GetOutBuffSize(void);

uint32_t JPEG_EncodeOutputHandler(JPEG_HandleTypeDef *hjpeg);
void JPEG_EncodeInputHandler(JPEG_HandleTypeDef *hjpeg);



#endif /* INC_JPEG_ENCODE_DMA_H_ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
