/**
  ******************************************************************************
	WIFI_MNGR
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#include <stdbool.h>
#include "main.h"
#include "usart.h"
#include "img_jpg_file.h"
#include "camera_mngr.h"



static stImg g_stImg	= {.m_eNextImgStates = eImgStates_finished, .m_eCurrImgStates = eImgStates_finished};

/* ================
stImg* Img_jpg_GetStruct(void)
================ */
stImg* Img_jpg_GetStruct(void)
{
	return &g_stImg;
}

/* ================
eImgStates Img_jpg_GetNextImgState(void)
================ */
eImgStates Img_jpg_GetNextImgState(void)
{
	return g_stImg.m_eNextImgStates;
}

/* ================
eImgStates Img_jpg_GetCurrImgState(void)
================ */
eImgStates Img_jpg_GetCurrImgState(void)
{
	return g_stImg.m_eCurrImgStates;
}

/* ================
eImgStates  Img_jpg_UpdateImgState(void)
================ */
eImgStates  Img_jpg_UpdateImgState(void)
{
	stImg* pstImg = &g_stImg;
	pstImg->m_eCurrImgStates = pstImg->m_eNextImgStates;
	return pstImg->m_eCurrImgStates;
}

/* ================
uint8_t* Img_jpg_get_arr_ptr(void)
================ */
//uint8_t* Img_jpg_GetArrPtr(void)
//{
//	return img_jpg_file_a;
//}

///* ================
//stImg*	Img_jpg_Iterate( void)
//================ */
//stImg*	Img_jpg_Iterate( void )
//{
//	stImg* pstImg = &g_stImg
//	if( pstImg != NULL )
//	{
//		switch (pstImg->m_eImgStates)
//		{
//			case eImgStates_start:
//			{
//				if( pstImg->m_SizeB > PACKET_DATA_LEN_B)
//				{
//					pstImg->m_eImgStates = eImgStates_sending;
//				}
//				else
//				{
//					pstImg->m_eImgStates = eImgStates_last_packet;
//				}
//			}
//			break;
//			case eImgStates_sending:
//			{
//				if( pstImg->m_SizeB > PACKET_DATA_LEN_B)
//				{
//					pstImg->m_SizeB -= PACKET_DATA_LEN_B;
//					pstImg->m_pImg	+= PACKET_DATA_LEN_B;
//				}
//				else
//				{
//					pstImg->m_eImgStates = eImgStates_last_packet;
//				}
//			}
//			break;
//			case eImgStates_last_packet:
//			{
//
//				pstImg->m_eImgStates = eImgStates_finished;
//			}
//			break;
//			case eImgStates_finished:
//			{
//			}
//			break;
//			default:
//				break;
//		}
//	}
//
//	return pstImg;
//}

/* ================
void	 	Img_jpg_GetNewImg(void)
================ */
void	 	Img_jpg_GetNewImg(void)
{
	stImg* pstImg				= &g_stImg;
	pstImg->m_pImg				= CameraMngr_GetCompressedImg(); // img_jpg_file_a;
	pstImg->m_SizeB				= CameraMngr_GetCompressedImgSize(); //IMG_JPG_SIZE_B;
	pstImg->m_SysTick			= 0;
	pstImg->m_eCurrImgStates	= eImgStates_start;
	pstImg->m_eNextImgStates	= eImgStates_start;
}


/*****END OF FILE****/
