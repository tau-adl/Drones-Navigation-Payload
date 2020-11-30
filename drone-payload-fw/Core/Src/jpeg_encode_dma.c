/**
 ******************************************************************************
 * @file    JPEG/JPEG_EncodingFromFLASH_DMA/Src/encode_dma.c
 * @author  MCD Application Team
 * @brief   This file provides routines for JPEG Encoding from memory with
 *          DMA method.
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

/* Includes ------------------------------------------------------------------*/
#include "fatfs.h"
#include "camera_mngr.h" // frame size defs
#include "jpeg.h"
#include "jpeg_utils_conf.h"
#include "jpeg_utils.h"
#include <string.h>
#include <stdio.h>

#include "jpeg_encode_dma.h"


/** @addtogroup STM32H7xx_HAL_Examples
 * @{
 */

/** @addtogroup JPEG_EncodingFromFLASH_DMA
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  __IO  uint8_t State;  
  uint8_t *DataBuffer;
  __IO uint32_t DataBufferSize;

}JPEG_Data_BufferTypeDef;

/* Private define ------------------------------------------------------------*/
#if (JPEG_RGB_FORMAT == JPEG_ARGB8888)
#define BYTES_PER_PIXEL    4
#elif (JPEG_RGB_FORMAT == JPEG_RGB888)
#define BYTES_PER_PIXEL    3
#elif (JPEG_RGB_FORMAT == JPEG_RGB565)
#define BYTES_PER_PIXEL    2
#endif

#define CHUNK_SIZE_IN   ((uint32_t)(MAX_INPUT_WIDTH * BYTES_PER_PIXEL * MAX_INPUT_LINES))
#define CHUNK_SIZE_OUT  ((uint32_t) (4096))

#define JPEG_BUFFER_EMPTY       0
#define JPEG_BUFFER_FULL        1

#define BMP_HEADER_SIZE  ((uint32_t)(54))

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
JPEG_RGBToYCbCr_Convert_Function pRGBToYCbCr_Convert_Function;

uint8_t Input_Data_Buffer[CHUNK_SIZE_IN];

uint8_t MCU_Data_IntBuffer0[CHUNK_SIZE_IN];
uint8_t MCU_Data_InBuffer1[CHUNK_SIZE_IN];

uint8_t JPEG_Data_OutBuffer0[CHUNK_SIZE_OUT];
uint8_t JPEG_Data_OutBuffer1[CHUNK_SIZE_OUT];

JPEG_Data_BufferTypeDef Jpeg_OUT_BufferTab = {JPEG_BUFFER_EMPTY , JPEG_Data_OutBuffer0 , 0};

JPEG_Data_BufferTypeDef Jpeg_IN_BufferTab = {JPEG_BUFFER_EMPTY , MCU_Data_IntBuffer0, 0};

uint32_t MCU_TotalNb                = 0;
uint32_t MCU_BlockIndex             = 0;
__IO uint32_t Jpeg_HWEncodingEnd    = 0;


__IO uint32_t Output_Is_Paused      = 0;
__IO uint32_t Input_Is_Paused       = 0;
uint32_t CurrentLine                = 1;

uint32_t OutBuffSize				= 0;
// TODO: this is a twick. fix it!
JPEG_ConfTypeDef Conf = {JPEG_COLOR_SPACE, JPEG_CHROMA_SAMPLING, IMG_H + 16, IMG_W, JPEG_IMAGE_QUALITY};
FIL *pBmpFile;
FIL *pJpegFile;

uint8_t* pFrameJpegBuff;
uint8_t* pFrameBuffOnSram;

/* Private function prototypes -----------------------------------------------*/
static void JpegEncodeDma_ReadBmpRgbLines(FIL *file, JPEG_ConfTypeDef Conf, uint8_t * pDataBuffer, uint32_t *BufferSize);
static void JpegEncodeDma_ReadRamGrayLines(uint8_t* pSrcBuffer, JPEG_ConfTypeDef Conf, uint8_t * pDataBuffer, uint32_t *BufferSize);
static void JpegEncodeDma_ReadBmpGrayLines(FIL *file, JPEG_ConfTypeDef Conf, uint8_t * pDataBuffer, uint32_t *BufferSize);

/* Private functions ---------------------------------------------------------*/




/**
 * @brief  JpegEncodeDMA_FromSdToSd
 * @param hjpeg: JPEG handle pointer
 * @param  bmpfile    : bmp file path for encode.
 * @param  jpgfile    : jpg file path for encode.
 * @retval None
 */
uint32_t JpegEncodeDMA_FromSdToSd(JPEG_HandleTypeDef *hjpeg, FIL *bmpfile, FIL *jpgfile)
{
	pBmpFile = bmpfile;
	pJpegFile = jpgfile;
	uint32_t dataBufferSize = 0;

	/* Reset all Global variables */
	MCU_TotalNb                = 0;
	MCU_BlockIndex             = 0;
	Jpeg_HWEncodingEnd         = 0;
	Output_Is_Paused           = 0;
	Input_Is_Paused            = 0;
	CurrentLine                = 1;

	/* BMP Header Parsing */
	BMP_GetInfo(pBmpFile, &Conf);
	JPEG_GetEncodeColorConvertFunc(&Conf, &pRGBToYCbCr_Convert_Function, &MCU_TotalNb);

	/* Clear Output Buffer */
	Jpeg_OUT_BufferTab.DataBufferSize = 0;
	Jpeg_OUT_BufferTab.State = JPEG_BUFFER_EMPTY;


	/* Fill input Buffer */
	/* Read and reorder MAX_INPUT_LINES lines from BMP file and fill data buffer */
	//ReadBmpRgbLines(pBmpFile, Conf, Input_Data_Buffer ,&dataBufferSize);
	JpegEncodeDma_ReadBmpGrayLines(pBmpFile, Conf, Input_Data_Buffer ,&dataBufferSize);

	/* RGB to YCbCr Pre-Processing */
	MCU_BlockIndex += pRGBToYCbCr_Convert_Function(Input_Data_Buffer, Jpeg_IN_BufferTab.DataBuffer, 0, dataBufferSize,(uint32_t*)(&Jpeg_IN_BufferTab.DataBufferSize));
	Jpeg_IN_BufferTab.State = JPEG_BUFFER_FULL;

	/* Fill Encoding Params */
	HAL_JPEG_ConfigEncoding(hjpeg, &Conf);

	/* Start JPEG encoding with DMA method */
	HAL_JPEG_Encode_DMA(hjpeg ,Jpeg_IN_BufferTab.DataBuffer ,Jpeg_IN_BufferTab.DataBufferSize ,Jpeg_OUT_BufferTab.DataBuffer ,CHUNK_SIZE_OUT);

	return 0;
}

/**
 * @brief  JpegEncodeDMA_FromRamToSd
 * @param hjpeg: JPEG handle pointer
 * @param  FrameBuff    : FrameBuff path for encode.
 * @param  jpgfile    : jpg file path for encode.
 * @retval None
 */
uint32_t JpegEncodeDMA_FromRamToSd(JPEG_HandleTypeDef *hjpeg, uint8_t* FrameBuff, FIL *jpgfile)
{
	pFrameBuffOnSram = FrameBuff;
	pJpegFile = jpgfile;
	uint32_t dataBufferSize = 0;

	/* Reset all Global variables */
	MCU_TotalNb                = 0;
	MCU_BlockIndex             = 0;
	Jpeg_HWEncodingEnd         = 0;
	Output_Is_Paused           = 0;
	Input_Is_Paused            = 0;
	CurrentLine                = 1;

	/* BMP Header Parsing */
	Conf.ImageHeight 		= IMG_H;
	Conf.ImageWidth 		= IMG_W;
	Conf.ColorSpace 		= JPEG_COLOR_SPACE;
	Conf.ChromaSubsampling  = JPEG_CHROMA_SAMPLING;
	Conf.ImageQuality       = JPEG_IMAGE_QUALITY;

	JPEG_GetEncodeColorConvertFunc(&Conf, &pRGBToYCbCr_Convert_Function, &MCU_TotalNb);


	/* Clear Output Buffer */
	Jpeg_OUT_BufferTab.DataBufferSize = 0;
	Jpeg_OUT_BufferTab.State = JPEG_BUFFER_EMPTY;


	/* Fill input Buffer */
	/* Read and reorder MAX_INPUT_LINES lines from BMP file and fill data buffer */
//	ReadBmpRgbLines(pBmpFile, Conf, Input_Data_Buffer ,&dataBufferSize);
//	ReadBayerLines(pBmpFile, Conf, FrameBuff /*Input_Data_Buffer */, Conf.ImageHeight*Conf.ImageWidth/*&dataBufferSize*/);

	//JpegEncodeDma_ReadRamGrayLines(pFrameBuffOnSram, Conf, Input_Data_Buffer ,&dataBufferSize);



	/* RGB to YCbCr Pre-Processing */
	MCU_BlockIndex += pRGBToYCbCr_Convert_Function(pFrameBuffOnSram , Jpeg_IN_BufferTab.DataBuffer, 0, dataBufferSize,(uint32_t*)(&Jpeg_IN_BufferTab.DataBufferSize));
	Jpeg_IN_BufferTab.State = JPEG_BUFFER_FULL;

	/* Fill Encoding Params */
	HAL_JPEG_ConfigEncoding(hjpeg, &Conf);

	/* Start JPEG encoding with DMA method */
	HAL_JPEG_Encode_DMA(hjpeg ,Jpeg_IN_BufferTab.DataBuffer ,Jpeg_IN_BufferTab.DataBufferSize ,Jpeg_OUT_BufferTab.DataBuffer ,CHUNK_SIZE_OUT);

	return 0;
}


/**
 * @brief  JpegEncodeDMA_FromRamToRam
 * @param hjpeg: JPEG handle pointer
 * @param  InFrameBuff    : FrameBuff path for encode.
 * @param  OutFrameBuff    : jpg file path for encode.
 * @retval None
 */
uint32_t JpegEncodeDMA_FromRamToRam(JPEG_HandleTypeDef *hjpeg, uint8_t* InFrameBuff, uint8_t* OutFrameBuff)
{
	pFrameBuffOnSram = InFrameBuff; // this is a static ptr for JPEG_EncodeInputHandler
	pFrameJpegBuff = OutFrameBuff;	// this is a static ptr for JPEG_EncodeOutputHandler
	uint32_t dataBufferSize = 0;

	/* Reset all Global variables */
	MCU_TotalNb                = 0;
	MCU_BlockIndex             = 0;
	Jpeg_HWEncodingEnd         = 0;
	Output_Is_Paused           = 0;
	Input_Is_Paused            = 0;
	CurrentLine                = 1;
	OutBuffSize				   = 0;

	JPEG_GetEncodeColorConvertFunc(&Conf, &pRGBToYCbCr_Convert_Function, &MCU_TotalNb);

	/* Clear Output Buffer */
	Jpeg_OUT_BufferTab.DataBufferSize = 0;
	Jpeg_OUT_BufferTab.State = JPEG_BUFFER_EMPTY;


	/* Fill input Buffer */
	/* Read and reorder MAX_INPUT_LINES lines from BMP file and fill data buffer */
	JpegEncodeDma_ReadRamGrayLines(InFrameBuff, Conf, Input_Data_Buffer ,&dataBufferSize);


	/* RGB to YCbCr Pre-Processing */
	MCU_BlockIndex += pRGBToYCbCr_Convert_Function(InFrameBuff , Jpeg_IN_BufferTab.DataBuffer, 0, dataBufferSize, (uint32_t*)(&Jpeg_IN_BufferTab.DataBufferSize));
	Jpeg_IN_BufferTab.State = JPEG_BUFFER_FULL;

	/* Fill Encoding Params */
	HAL_JPEG_ConfigEncoding(hjpeg, &Conf);

	/* Start JPEG encoding with DMA method */
	HAL_JPEG_Encode_DMA(hjpeg ,Jpeg_IN_BufferTab.DataBuffer ,Jpeg_IN_BufferTab.DataBufferSize ,Jpeg_OUT_BufferTab.DataBuffer, CHUNK_SIZE_OUT);

	return 0;
}

uint32_t JpegEncodeDMA_GetOutBuffSize(void)
{
	return OutBuffSize;
}

/**
 * @brief JPEG Ouput Data BackGround processing .
 * @param hjpeg: JPEG handle pointer
 * @retval 1 : if JPEG processing has finiched, 0 : if JPEG processing still ongoing
 */
uint32_t JPEG_EncodeOutputHandler(JPEG_HandleTypeDef *hjpeg)
{
	uint32_t bytesWritefile = 0;

	if(Jpeg_OUT_BufferTab.State == JPEG_BUFFER_FULL)
	{
		// if saving to fatfs
		//f_write (pJpegFile, Jpeg_OUT_BufferTab.DataBuffer ,Jpeg_OUT_BufferTab.DataBufferSize, (UINT*)(&bytesWritefile)) ;

		// if saving to ram
		memcpy(pFrameJpegBuff, Jpeg_OUT_BufferTab.DataBuffer ,Jpeg_OUT_BufferTab.DataBufferSize);
		pFrameJpegBuff += Jpeg_OUT_BufferTab.DataBufferSize;
		OutBuffSize += Jpeg_OUT_BufferTab.DataBufferSize;

		Jpeg_OUT_BufferTab.State = JPEG_BUFFER_EMPTY;
		Jpeg_OUT_BufferTab.DataBufferSize = 0;

		if(Jpeg_HWEncodingEnd != 0)
		{
			return 1;
		}
		else if(Output_Is_Paused == 1)
		{
			Output_Is_Paused = 0;
			HAL_JPEG_Resume(hjpeg, JPEG_PAUSE_RESUME_OUTPUT);
		}
	}


	return 0;
}

/**
 * @brief JPEG Input Data BackGround Preprocessing .
 * @param hjpeg: JPEG handle pointer
 * @retval None
 */
void JPEG_EncodeInputHandler(JPEG_HandleTypeDef *hjpeg)
{
  uint32_t dataBufferSize = 0;

  if((Jpeg_IN_BufferTab.State == JPEG_BUFFER_EMPTY) && (MCU_BlockIndex <= MCU_TotalNb))  
  {
		  // TODO: this function is hard coded. make this flixble
		/* Read and reorder 16 lines from BMP file and fill data buffer */
	  	  //    ReadBmpRgbLines(pBmpFile, Conf, Input_Data_Buffer ,&dataBufferSize);
		  //ReadBmpGrayLines(pBmpFile, Conf, Input_Data_Buffer ,&dataBufferSize);
		  JpegEncodeDma_ReadRamGrayLines(pFrameBuffOnSram, Conf, Input_Data_Buffer, &dataBufferSize);


	//	  JpegEncodeDma_ReadBmpGrayLines(pBmpFile, Conf, Input_Data_Buffer ,&dataBufferSize);

		if(dataBufferSize != 0)
		{
			/* Pre-Processing */
			MCU_BlockIndex += pRGBToYCbCr_Convert_Function(Input_Data_Buffer, Jpeg_IN_BufferTab.DataBuffer, 0, dataBufferSize, (uint32_t*)(&Jpeg_IN_BufferTab.DataBufferSize));
			//MCU_BlockIndex += pRGBToYCbCr_Convert_Function(Input_Data_Buffer /*Input_Data_Buffer */, Jpeg_IN_BufferTab.DataBuffer, 0, MAX_INPUT_LINES*Conf.ImageWidth/*dataBufferSize*/,(uint32_t*)(&Jpeg_IN_BufferTab.DataBufferSize));
			Jpeg_IN_BufferTab.State = JPEG_BUFFER_FULL;

			if(Input_Is_Paused == 1)
			{
				Input_Is_Paused = 0;
				HAL_JPEG_ConfigInputBuffer(hjpeg,Jpeg_IN_BufferTab.DataBuffer, Jpeg_IN_BufferTab.DataBufferSize);

				HAL_JPEG_Resume(hjpeg, JPEG_PAUSE_RESUME_INPUT);
			}
		}
		else
		{
			MCU_BlockIndex++;
		}
	}

  //printf("MCU_BlockIndex\t%d\r\n", MCU_BlockIndex);
}

/**
 * @brief JPEG Get Data callback
 * @param hjpeg: JPEG handle pointer
 * @param NbEncodedData: Number of encoded (consummed) bytes from input buffer
 * @retval None
 */
void HAL_JPEG_GetDataCallback(JPEG_HandleTypeDef *hjpeg, uint32_t NbEncodedData)
{
	if(NbEncodedData == Jpeg_IN_BufferTab.DataBufferSize)
	{
		Jpeg_IN_BufferTab.State = JPEG_BUFFER_EMPTY;
		Jpeg_IN_BufferTab.DataBufferSize = 0;

		HAL_JPEG_Pause(hjpeg, JPEG_PAUSE_RESUME_INPUT);
		Input_Is_Paused = 1;
	}
	else
	{
		HAL_JPEG_ConfigInputBuffer(hjpeg,Jpeg_IN_BufferTab.DataBuffer + NbEncodedData, Jpeg_IN_BufferTab.DataBufferSize - NbEncodedData);
	}
}

/**
 * @brief JPEG Data Ready callback
 * @param hjpeg: JPEG handle pointer
 * @param pDataOut: pointer to the output data buffer
 * @param OutDataLength: length of output buffer in bytes
 * @retval None
 */
void HAL_JPEG_DataReadyCallback (JPEG_HandleTypeDef *hjpeg, uint8_t *pDataOut, uint32_t OutDataLength)
{
	Jpeg_OUT_BufferTab.State = JPEG_BUFFER_FULL;
	Jpeg_OUT_BufferTab.DataBufferSize = OutDataLength;

	HAL_JPEG_Pause(hjpeg, JPEG_PAUSE_RESUME_OUTPUT);
	Output_Is_Paused = 1;

	HAL_JPEG_ConfigOutputBuffer(hjpeg, Jpeg_OUT_BufferTab.DataBuffer, CHUNK_SIZE_OUT);
}

/**
 * @brief  JPEG Error callback
 * @param hjpeg: JPEG handle pointer
 * @retval None
 */
void HAL_JPEG_ErrorCallback(JPEG_HandleTypeDef *hjpeg)
{
	Error_Handler();
}

/*
 * @brief JPEG Decode complete callback
 * @param hjpeg: JPEG handle pointer
 * @retval None
 */
void HAL_JPEG_EncodeCpltCallback(JPEG_HandleTypeDef *hjpeg)
{
	Jpeg_HWEncodingEnd = 1;
}

/*
 * @brief Read and reorder MAX_INPUT_LINES lines from BMP file and fill data buffer
 * @param file: pointer to the source file name
 * @param file: pointer to the Data Buffer Adress
 * @param file: pointer to the Data Buffer Size
 * @retval None
 */
static void JpegEncodeDma_ReadBmpRgbLines(FIL *file, JPEG_ConfTypeDef Conf, uint8_t * pDataBuffer, uint32_t *BufferSize)
{
	uint32_t bytesReadfile    = 1;
	uint32_t CurrentBlockLine = 1;
	*BufferSize = 0;

	while((CurrentLine <= Conf.ImageHeight) && (CurrentBlockLine <= MAX_INPUT_LINES))
	{
		f_lseek (file,BMP_HEADER_SIZE + Conf.ImageWidth *(Conf.ImageHeight - CurrentLine)*3);
		f_read (file, pDataBuffer , Conf.ImageWidth*3 , (UINT*)(&bytesReadfile));

		pDataBuffer += bytesReadfile;
		*BufferSize += bytesReadfile;
		CurrentLine +=1 ;
		CurrentBlockLine += 1;
	}

}


/*
 * @brief modified func of ReadBmpRgbLines Read and reorder MAX_INPUT_LINES lines from BMP file and fill data buffer
 * @param file: pointer to the source file name
 * @param file: pointer to the Data Buffer Adress
 * @param file: pointer to the Data Buffer Size
 * @retval None
 */
static void JpegEncodeDma_ReadBmpGrayLines(FIL *file, JPEG_ConfTypeDef Conf, uint8_t * pDataBuffer, uint32_t *BufferSize)
{
	uint32_t bytesReadfile    = 1;
	uint32_t CurrentBlockLine = 1;
	*BufferSize = 0;
	uint8_t TmpBuff[MAX_INPUT_WIDTH];
    int ColIdx;

	while((CurrentLine <= Conf.ImageHeight) && (CurrentBlockLine <= MAX_INPUT_LINES))
	{
		f_lseek (file,BMP_HEADER_SIZE + Conf.ImageWidth *(Conf.ImageHeight - CurrentLine)*1);
		f_read (file, TmpBuff , Conf.ImageWidth*1 , (UINT*)(&bytesReadfile));

		// TODO: optimize!!
		for (ColIdx = 0; ColIdx < Conf.ImageWidth; ColIdx++)
		{
		   *pDataBuffer++ = TmpBuff[ColIdx];
		   *pDataBuffer++ = TmpBuff[ColIdx];
		   *pDataBuffer++ = TmpBuff[ColIdx];
		}

		*BufferSize += bytesReadfile * 3;
		CurrentLine +=1 ;
		CurrentBlockLine += 1;
	}
}

/*
 * @brief Read and reorder MAX_INPUT_LINES lines from Sram buff and fill data buffer
 * @param file: pointer to the source file name
 * @param file: pointer to the Data Buffer Adress
 * @param file: pointer to the Data Buffer Size
 * @retval None
 */
static void JpegEncodeDma_ReadRamGrayLines(uint8_t* pSrcBuffer, JPEG_ConfTypeDef Conf, uint8_t * pDataBuffer, uint32_t *BufferSize)
{
	uint32_t BytesReadfile    = 1;
	uint32_t CurrentBlockLine = 1;
	*BufferSize = 0;
	uint8_t TmpBuff[MAX_INPUT_WIDTH];
    uint32_t ColIdx;
    uint32_t CurrHop = 0;
    uint8_t Pixel = 0;
	
	while ((CurrentLine <= (Conf.ImageHeight)) && (CurrentBlockLine <= MAX_INPUT_LINES))
	{
		BytesReadfile = Conf.ImageWidth;
		CurrHop = (Conf.ImageWidth * (Conf.ImageHeight - CurrentLine) * 1);
		memcpy(TmpBuff, pSrcBuffer + CurrHop, BytesReadfile);

		//pSrcBuffer	+= bytesReadfile;

		// TODO: optimize!!
		// triple for RGB
		for (ColIdx = 0; ColIdx < BytesReadfile; ColIdx++)
		{
			Pixel = TmpBuff[ColIdx];
			memset(pDataBuffer, Pixel, 3);
			pDataBuffer = pDataBuffer + 3;
//			Pixel = TmpBuff[ColIdx];
//		   *pDataBuffer++ = Pixel; // R
//		   *pDataBuffer++ = Pixel; // G
//		   *pDataBuffer++ = Pixel; // B


//		   *pDataBuffer++ = TmpBuff[ColIdx]; // R
//		   *pDataBuffer++ = TmpBuff[ColIdx]; // G
//		   *pDataBuffer++ = TmpBuff[ColIdx]; // B

		}

		*BufferSize += BytesReadfile * 3;
		CurrentLine +=1 ;
		CurrentBlockLine += 1;
	}
}

/**
 * @brief  Get the images sizes from BMP header.
 * @param  MyFile: pointer to the source file name
 * @param  pInfo: pointer to the Info structure
 * @retval None
 */
void BMP_GetInfo(FIL * MyFile, JPEG_ConfTypeDef *pInfo)
{
	uint32_t bytesReadfile = 0;
	uint32_t ImageWidth = 0;
	uint32_t ImageHeight = 0 ;
	uint32_t ImageWidthOffset = 18;
	uint32_t ImageHeightOffset = 22;

	/*Open the image with read access */
	f_lseek (MyFile,ImageWidthOffset);
	if(f_read (MyFile, (void *) &ImageWidth , 4, (UINT*)(&bytesReadfile)) == FR_OK)
	{
		pInfo->ImageWidth         = ImageWidth;
	}
	f_lseek (MyFile,ImageHeightOffset);
	if(f_read (MyFile, (void *) &ImageHeight , 4, (UINT*)(&bytesReadfile)) == FR_OK)
	{
		pInfo->ImageHeight        = ImageHeight;
	}

	/* Jpeg Encoding Setting to be setted by users */
	pInfo->ChromaSubsampling  = JPEG_CHROMA_SAMPLING;
	pInfo->ColorSpace         = JPEG_COLOR_SPACE;
	pInfo->ImageQuality       = JPEG_IMAGE_QUALITY;

	/*Check if Image Sizes meets the requirements */
	if (((pInfo->ImageWidth % 8) != 0 ) || ((pInfo->ImageHeight % 8) != 0 ) || \
			(((pInfo->ImageWidth % 16) != 0 ) && (pInfo->ColorSpace == JPEG_YCBCR_COLORSPACE) && (pInfo->ChromaSubsampling != JPEG_444_SUBSAMPLING)) || \
			(((pInfo->ImageHeight % 16) != 0 ) && (pInfo->ColorSpace == JPEG_YCBCR_COLORSPACE) && (pInfo->ChromaSubsampling == JPEG_420_SUBSAMPLING)))
	{
		Error_Handler();
	}

}
/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
