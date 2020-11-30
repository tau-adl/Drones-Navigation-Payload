/*
 * camera_mngr.c
 */
#include <stdio.h>
#include <stdbool.h>
#include <stdio.h>
#include "dcmi.h"
#include "jpeg.h"
#include "ff.h" // TODO: DB - fix this. only jpeg_encode_dma needs this
#include "sensor.h"
#include "jpeg_encode_dma.h"
#include "camera_mngr.h"

static stCameraMngr g_CameraMngr = {0x00};

void CameraMngr_DcmiAcqBenchmark(void);
uint32_t CamperMngr_CalcCompressedImgChecksum(void);


/**
 * @brief  CameraMngr_Init
 * @retval None
 */
void CameraMngr_Init(void)
{
	stCameraMngr* pThis = &g_CameraMngr;

	sensor_init();

	int CamId = sensor_get_id();

	printf("camera id = %d\r\n", CamId);
	if (CamId == MT9V034_ID)
	{
		printf("camera MT9V034 OK\r\n");
	}
	else
	{
		printf("camera MT9V034 ERR\r\n");
	}

	sensor_reset();
	sensor_set_pixformat(PIXFORMAT_GRAYSCALE); // only grayscale supported
	sensor_set_framesize(FRAME_SIZE);

	pThis->m_FrameWidth 			= IMG_W;
	pThis->m_FrameHeight 			= IMG_H;

	pThis->m_pFrameBuff				= CameraFrameBuff;
	pThis->m_FrameBuffSize			= FRAME_BUFF_SIZE;
	pThis->m_pJpegFrameBuff 		= JpegFrameBuff;
	pThis->m_JpegFrameBuffSize		= 0;
	pThis->m_JpegFrameBuffConvSize	= 0;

	pThis->m_pCompressedImg			= CompressedImg;

	pThis->m_eCamImgState			= eCamImgState_Init;
	pThis->m_eCompressedImgState	= eCompressedImgState_Init;

	pThis->m_JpegFrameBuffChecksum 		= 0;

	pThis->m_DcmiFrameAcqStartTick		= 0;

#ifdef CAMERA_BENCHMARK

	pThis->m_JpegConvStartTick			= 0;
	pThis->m_JpegConvDuration_msec		= 0;
	pThis->m_DcmiFrameAcqDuration_msec 	= 0;

//	printf( "CameraFrameBuff\t@ 0x%x  aligned with = %ld to uint32_t\r\n",
//			(uint32_t) CameraFrameBuff,
//			((uint32_t) CameraFrameBuff - D1_AXISRAM_BASE) % 32);
//
//	printf( "JpegFrameBuff\t@ 0x%x aligned with = %ld to uint32_t\r\n",
//			(uint32_t) JpegFrameBuff,
//			((uint32_t) JpegFrameBuff - D1_AXISRAM_BASE) % 32);


	printf("img dim = W=%d\tH=%d\r\n", IMG_W, IMG_H);

	if (JPEG_CHROMA_SAMPLING == JPEG_420_SUBSAMPLING)
	{
		printf("JPEG_CHROMA_SAMPLING = \tJPEG_420_SUBSAMPLING\r\n");
	}
	else if (JPEG_CHROMA_SAMPLING == JPEG_422_SUBSAMPLING)
	{
		printf("JPEG_CHROMA_SAMPLING = \tJPEG_422_SUBSAMPLING\r\n");
	}
	else if (JPEG_CHROMA_SAMPLING == JPEG_444_SUBSAMPLING)
	{
		printf("JPEG_CHROMA_SAMPLING = \tJPEG_444_SUBSAMPLING\r\n");
	}

	printf("JPEG_IMAGE_QUALITY = \t %d\r\n", JPEG_IMAGE_QUALITY);
	printf("MAX_INPUT_LINES = \t %d\r\n", MAX_INPUT_LINES);

#endif // CAMERA_BENCHMARK
}

/**
 * @brief  CameraMngr_GetImgBuff
 * @retval Frame Buff ptr
 */
uint8_t* CameraMngr_GetFrameBuff(void)
{
	return g_CameraMngr.m_pFrameBuff;
}

/**
 * @brief  CameraMngr_GetCompressedImg
 * @retval Jpeg Buff ptr
 */
uint8_t* CameraMngr_GetCompressedImg(void)
{
	return g_CameraMngr.m_pCompressedImg;
}

/**
 * @brief  CameraMngr_GetJpegFrameBuffSize
 * @retval
 */
uint32_t CameraMngr_GetCompressedImgSize(void)
{
	return g_CameraMngr.m_CompressedImgSize;
}

/**
 * @brief  CameraMngr_GetCamImgState
 * @retval CamImgState
 */
eCamImgState CameraMngr_GetCamImgState(void)
{
	return g_CameraMngr.m_eCamImgState;
}

/**
 * @brief  CameraMngr_SetCamImgState
 * @retval void
 */
void CameraMngr_SetCamImgState(eCamImgState a_eCamImgState)
{
	g_CameraMngr.m_eCamImgState = a_eCamImgState;
}

/**
 * @brief  CameraMngr_GetCompressedImgState
 * @retval CompressedImgState
 */
eCompressedImgState CameraMngr_GetCompressedImgState(void)
{
	return g_CameraMngr.m_eCompressedImgState;
}

/**
 * @brief  CameraMngr_GetCompressedImgState
 * @retval none
 */
void CameraMngr_SetCompressedImgState(eCompressedImgState a_eCompressedImgState)
{
	g_CameraMngr.m_eCompressedImgState = a_eCompressedImgState;
}

/**
 * @brief  CameraMngr_WaitForFrame
 * @retval None
 */
void CameraMngr_WaitForFrame(void)
{
	uint32_t TickStart = HAL_GetTick();

	// Wait for frame
	while ( CameraMngr_isDcmiAcqRunning() )
	{
		// Wait for interrupt
		__WFI();

		if ((HAL_GetTick() - TickStart) >= DCMI_ACQ_TIMEOUT_MSEC)
		{
			printf("dcmi timeout!\r\n");

			HAL_DCMI_Stop(&hdcmi);
			break;
		}
	}

}

/**
 * @brief  CameraMngr_isDcmiAcqRunning
 * @retval isDcmiAcqRunning
 */
bool CameraMngr_isDcmiAcqRunning( void )
{
	return ( (DCMI->CR & DCMI_CR_CAPTURE) != 0 );
}

/**
 * @brief  CameraMngr_isDcmiAcqEndded
 * @retval isDcmiAcqEndded
 */
bool CameraMngr_isDcmiAcqEndded( void )
{
	stCameraMngr* pThis = &g_CameraMngr;

	bool AcqRunning = ( (DCMI->CR & DCMI_CR_CAPTURE) != 0 );
	bool AcqStarted = ( pThis->m_eCamImgState == eCamImgState_AcqStart );


	uint32_t CurrAcqTime_msec = HAL_GetTick() - pThis->m_DcmiFrameAcqStartTick;


	if ( CurrAcqTime_msec >= DCMI_ACQ_TIMEOUT_MSEC )
	{
#ifdef CAMERA_BENCHMARK
		printf("dcmi timeout, took=%d[msec]!\r\n", CurrAcqTime_msec);
#endif // CAMERA_BENCHMARK
		HAL_DCMI_Stop(&hdcmi);

		AcqRunning = false;
	}


	return ( (!AcqRunning) && AcqStarted );
}

/**
 * @brief  CameraMngr_DcmiFrameAcqDma
 * @retval None
 */
void CameraMngr_DcmiFrameAcqDma( void )
{
	stCameraMngr* pThis = &g_CameraMngr;

	printf("%d\tacq s\r\n", HAL_GetTick());

	pThis->m_eCamImgState = eCamImgState_AcqStart;


#ifdef CAMERA_BENCHMARK
	pThis->m_DcmiFrameAcqDuration_msec = 0;
#endif // CAMERA_BENCHMARK
	pThis->m_DcmiFrameAcqStartTick = HAL_GetTick();


	// start capture
	HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)g_CameraMngr.m_pFrameBuff, g_CameraMngr.m_FrameBuffSize);

//	// dummy loop poll on dcmi capture complete
//	CameraMngr_WaitForFrame(); // TODO: DB - make this non-blocking
//	pThis->m_eCamImgState = eCamImgState_AcqCmplt;
//
//#ifdef CAMERA_BENCHMARK
//	pThis->m_DcmiFrameAcqDuration_msec = HAL_GetTick() - pThis->m_DcmiFrameAcqStartTick;
//	printf("dcmi frame acq duration %ld[msec] \r\n", pThis->m_DcmiFrameAcqDuration_msec );
//#endif // CAMERA_BENCHMARK

}

/**
 * @brief  CameraMngr_CompressStart
 * @retval None
 */
void CameraMngr_CompressStart(void)
{
	stCameraMngr* pThis = &g_CameraMngr;

	printf("%d\tcomp s\r\n", HAL_GetTick());

	pThis->m_eCamImgState = eCamImgState_CompressStart;

#ifdef CAMERA_BENCHMARK
	pThis->m_JpegConvDuration_msec = 0;
	pThis->m_JpegConvStartTick = HAL_GetTick();
#endif // CAMERA_BENCHMARK

	pThis->m_JpegFrameBuffConvSize = 0;


	JpegEncodeDMA_FromRamToRam(&hjpeg, pThis->m_pFrameBuff, pThis->m_pJpegFrameBuff);

	CameraMngr_CompressProc();
}

/**
 * @brief  CameraMngr_CompressProc
 * @retval None
 */
void CameraMngr_CompressProc(void)
{
	stCameraMngr* pThis = &g_CameraMngr;
	uint32_t JpegEncodeProcessingEnd = 0;

	JPEG_EncodeInputHandler(&hjpeg);
	JpegEncodeProcessingEnd = JPEG_EncodeOutputHandler(&hjpeg);

	if ( JpegEncodeProcessingEnd != 0 )
	{
		pThis->m_eCamImgState = eCamImgState_CompressCmplt;
	}

}


/**
 * @brief  CameraMngr_CompressEnd
 * @retval None
 */
void CameraMngr_CompressEnd(void)
{
	stCameraMngr* pThis = &g_CameraMngr;

	pThis->m_JpegFrameBuffConvSize = JpegEncodeDMA_GetOutBuffSize();


//	pThis->m_eCamImgState	= eCamImgState_CompressCmplt;
	pThis->m_JpegFrameBuffChecksum = CamperMngr_CalcCompressedImgChecksum();

	if ( pThis->m_eCompressedImgState == eCompressedImgState_SendStart )
	{
		printf("%d\timg OVR\r\n", HAL_GetTick()); // overrun

//		// TODO: decide what to do, youd miss this frame here
//		Error_Handler();
	}
	else
	{
		pThis->m_CompressedImgSize = pThis->m_JpegFrameBuffConvSize;
		pThis->m_eCompressedImgState = eCompressedImgState_WaitForSend;
		memcpy(pThis->m_pCompressedImg, pThis->m_pJpegFrameBuff, pThis->m_CompressedImgSize);

//		// TODO: remove. this is a try to reset the buffer
//		uint8_t Dummy = 0xFF;
//		memset(pThis->m_pJpegFrameBuff, Dummy, pThis->m_CompressedImgSize);
	}

}

/**
 * @brief  CameraMngr_CompressBenchmark
 * @retval None
 */
void CameraMngr_CompressBenchmark(void)
{
#ifdef CAMERA_BENCHMARK
	stCameraMngr* pThis = &g_CameraMngr;

	//printf("src size = %ld [b]\r\n", pThis->m_FrameBuffSize);

	pThis->m_JpegConvDuration_msec = HAL_GetTick() - pThis->m_JpegConvStartTick;
	printf("%d\tjout size = %ld [b]\tjcmp %ld[msec]\r\n", HAL_GetTick(), pThis->m_JpegFrameBuffConvSize, pThis->m_JpegConvDuration_msec );
//	printf("%d\r\n", HAL_GetTick());
#endif // CAMERA_BENCHMARK

}

/**
 * @brief  CameraMngr_GetCompressedImgChecksum
 * @retval CompressedImgChecksum
 */
uint32_t CameraMngr_GetCompressedImgChecksum(void)
{
	return g_CameraMngr.m_JpegFrameBuffChecksum;
}

/**
 * @brief  CameraMngr_HandleEvents
 * @retval None
 */
void CameraMngr_HandleEvents(void)
{
	switch (CameraMngr_GetCamImgState())
	{
	case eCamImgState_Init:
	case eCamImgState_CompressCmplt:
	{
		// start the machine
		if ( PacketMngr_GetIsImgTickCam() )
		{
			CameraMngr_DcmiFrameAcqDma();
		}
	}
	break;
	case eCamImgState_AcqStart:
	{
		if ( CameraMngr_isDcmiAcqEndded() )
		{

	        //HAL_DMA_Abort(&hdcmi);

	        // Disable DMA IRQ
	        //HAL_NVIC_DisableIRQ(DMA2_Stream7_IRQn);

			CameraMngr_SetCamImgState( eCamImgState_AcqCmplt );
			CameraMngr_DcmiAcqBenchmark();

#ifdef SAVE_INPUT_IMG_ON_SD
			char FileNameOnSd[10];
			sprintf(FileNameOnSd, "%d.bmp", g_CameraMngr.m_DcmiFrameAcqStartTick);
			FS_SaveBuffOnSdCard(CameraFrameBuff, FRAME_BUFF_SIZE, FileNameOnSd);
#endif // SAVE_INPUT_IMG_ON_SD

			CameraMngr_CompressStart();
		}
	}
	break;
	case eCamImgState_AcqCmplt:
	case eCamImgState_CompressStart:
	{
		CameraMngr_CompressProc();

		if ( CameraMngr_GetCamImgState() == eCamImgState_CompressCmplt  )
		{
			//HAL_JPEG_DeInit(&hjpeg);

			CameraMngr_CompressEnd();
			CameraMngr_CompressBenchmark();


#ifdef SAVE_OUTPUT_IMG_ON_SD
			char FileNameOnSd[10];
			sprintf(FileNameOnSd, "%d.jpg", g_CameraMngr.m_DcmiFrameAcqStartTick);
			FS_SaveBuffOnSdCard(JpegFrameBuff, g_CameraMngr.m_JpegFrameBuffConvSize, FileNameOnSd);
#endif // SAVE_OUTPUT_IMG_ON_SD

			HAL_DCMI_ReInitDMA(&hdcmi);
		}
	}
	break;
	default:
	{
		Error_Handler();
	}
	break;
	}
}

/**
 * @brief  CameraMngr_DcmqAcqBenchmark
 * @retval None
 */
void CameraMngr_DcmiAcqBenchmark(void)
{
#ifdef CAMERA_BENCHMARK
	stCameraMngr* pThis = &g_CameraMngr;

	pThis->m_DcmiFrameAcqDuration_msec = HAL_GetTick() - pThis->m_DcmiFrameAcqStartTick;
	printf("%d\tacq dur %ld[msec]\r\n", HAL_GetTick(), pThis->m_DcmiFrameAcqDuration_msec );
#endif // CAMERA_BENCHMARK
}

/**
 * @brief  CamperMngr_CalcCompressedImgChecksum
 * @retval None
 */
uint32_t CamperMngr_CalcCompressedImgChecksum(void)
{
	stCameraMngr* pThis = &g_CameraMngr;

	uint32_t idx = 0;
	unsigned char *p = (unsigned char *)pThis->m_pCompressedImg;
	uint32_t Checksum = 0;

	for(idx=0; idx < pThis->m_CompressedImgSize; idx++)
	{
		Checksum += p[idx];
	}

	return Checksum;
}
