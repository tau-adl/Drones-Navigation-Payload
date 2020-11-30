/**
  ******************************************************************************
	WIFI_MNGR
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include "main.h"
#include "camera_mngr.h"
#include "wifi_mngr.h"
#include "tim.h"
#include "packet_mngr.h"


static stPacketMngr	g_stPacketMngr;

static void PacketMngr_GetNewImuCall(void);
static eImgStates PacketMngr_IterateImg(int8_t a_Socket);
static void PacketMngr_IterateImu();
static void PacketMngr_SendImu(int8_t a_Socket);
static void PacketMngr_SetState(ePacketMngrState a_ePacketMngrState);

static bool PacketMngr_GetIsWifiSendEvent( void );

/* ================
void PacketMngr_Init(void);
================ */
void PacketMngr_Init(void)
{
	stPacketMngr* p_stPacketMngr = &g_stPacketMngr;
	PacketMngr_SetState(ePacketMngrState_off);
	p_stPacketMngr->m_FrameEventCtr			=	0;
	p_stPacketMngr->m_p_stImg				= Img_jpg_GetStruct();
	p_stPacketMngr->m_ImuEventCtr			=	0;
	p_stPacketMngr->m_ImuCallsPerPacket		=	0;
	p_stPacketMngr->m_IsImuCallReady		= false;
	p_stPacketMngr->m_IsImuPacketReady		= false;
	p_stPacketMngr->m_IsImgTickCam			= false;
	p_stPacketMngr->m_IsWifiTick			= false;
	p_stPacketMngr->m_Tick					= 0;
	memset((&p_stPacketMngr->m_stImuPacket),0,sizeof(stImuPacket));
}

/* ================
void PacketMngr_Start(void)
================ */
void PacketMngr_Start(void)
{
	stPacketMngr* 	p_stPacketMngr 	= 	&g_stPacketMngr;
	stImg*			p_stImg			=	p_stPacketMngr->m_p_stImg;
	p_stImg->m_eCurrImgStates		= 	eImgStates_finished;
	p_stImg->m_eNextImgStates		= 	eImgStates_finished;

	TIM_StartImuTick();
	TIM_StartWifiTick();

	printf("pm start\r\n");
}

/* ================
void PacketMngr_Stop(void)
================ */
void PacketMngr_Stop(void)
{
	stPacketMngr* 	p_stPacketMngr 	= 	&g_stPacketMngr;
	stImg*			p_stImg			=	p_stPacketMngr->m_p_stImg;
	p_stImg->m_eCurrImgStates		= 	eImgStates_finished;
	p_stImg->m_eNextImgStates		= 	eImgStates_finished;

	p_stPacketMngr->m_IsImuPacketReady = false;

	TIM_StopImuTick();
	TIM_StopWifiTick();

	printf("pm stop\r\n");
}


/* ================
void PacketMngr_TxRoutine(void)
================ */
void PacketMngr_TxRoutine(int8_t a_Socket)
{
	stPacketMngr* p_stPacketMngr = &g_stPacketMngr;
#ifdef USING_IMU
	PacketMngr_IterateImu();
#endif
	switch (p_stPacketMngr->m_ePacketMngrState) {
	case ePacketMngrState_off:
	{
#ifdef USING_FRAME


		if ( CameraMngr_GetCompressedImgState() == eCompressedImgState_WaitForSend )
		{
			CameraMngr_SetCompressedImgState(eCompressedImgState_SendStart);

			if(Img_jpg_GetCurrImgState() == eImgStates_finished)
			{
				printf("%d\tSOF\r\n", HAL_GetTick());
				PacketMngr_SetState(ePacketMngrState_Frame);
				PacketMngr_GetNewImg();
				PacketMngr_IterateImg(a_Socket);
			}
			else
			{
				printf("##M\r\n");
			}
		}

#endif
#ifdef USING_IMU
			if(p_stPacketMngr->m_IsImuPacketReady == true)
			{
				PacketMngr_SendImu(a_Socket);
			}
#endif
#ifdef USING_FRAME

#endif
	}
	break;
	case ePacketMngrState_Frame:
	{
		if(p_stPacketMngr->m_IsImuPacketReady == true)
		{
			PacketMngr_IterateImg( a_Socket);
		}
	}
	break;
	case ePacketMngrState_IMU:
	{
		printf("V\r\n"); // SO: it never going to get here probably
	}
	break;
	default:
	{
		printf("U\r\n"); // SO: it never going to get here probably
	}
		break;
	}
}

/* ================
void PacketMngr_Update(void);
================ */
void PacketMngr_Update(void)
{
	stPacketMngr* p_stPacketMngr = &g_stPacketMngr;

#ifdef USING_IMU
	UPDATE_IMU_EVENT_CTR(p_stPacketMngr->m_ImuEventCtr);

	if(p_stPacketMngr->m_ImuEventCtr == COUNTING_ENDED_VAL) // SO: new IMU call Event
	{

//		printf("new imu call\r\n");
		p_stPacketMngr->m_IsImuCallReady	=	true;

		if(p_stPacketMngr->m_ImuCallsPerPacket == COUNTING_ENDED_VAL) // SO: new IMU packet
		{
			p_stPacketMngr->m_IsImuPacketReady = true;
//			printf("new imu packet\r\n");
		}
	}
#endif
#ifdef USING_FRAME


	UPDATE_FRAME_EVENT_CTR(p_stPacketMngr->m_FrameEventCtr);
	if(p_stPacketMngr->m_FrameEventCtr == COUNTING_ENDED_VAL)	// SO: new frame packet
	{
		p_stPacketMngr->m_IsImgTickCam	= true;

		p_stPacketMngr->m_Tick = HAL_GetTick();
//		printf("%d\tTick\r\n", p_stPacketMngr->m_Tick); // SO:  printf inside timer callback is not recommended at all!!!

//		printf("", p_stPa)
	}
#endif
}

/* ================
void PacketMngr_UpdateWifiTick(void);
================ */
void PacketMngr_UpdateWifiTick(void)// each cycle is 250usec
{
	UPDATE_WIFI_TICK_CTR(g_stPacketMngr.m_IsWifiTick);
}


/* ================
void PacketMngr_IterateImg(void);
================ */
eImgStates PacketMngr_IterateImg(int8_t a_Socket)
{
	if( PacketMngr_GetIsWifiSendEvent() == true)
	{

		stPacketMngr* 	p_stPacketMngr 	= 	&g_stPacketMngr;
		stImg*			p_stImg			=	p_stPacketMngr->m_p_stImg;
		stFrame* 		p_stFrame 		= (stFrame*)(p_stPacketMngr->m_PacketBytes);
		uint16_t 	packet_data_size_b 	= 	0;
		uint8_t*	pData				=	p_stImg->m_pImg;

		int8_t Result = SOCK_ERR_NO_ERROR;

		switch (p_stImg->m_eCurrImgStates)
		{
		case eImgStates_start:
		{
			p_stImg->m_SysTick 			= 	p_stPacketMngr->m_Tick;
			p_stImg->m_eCurrImgStates	= eImgStates_sending;
			PacketMngr_SetState(ePacketMngrState_Frame);
		}
		case eImgStates_sending:
		{
			p_stFrame->m_FrameSOF	=	FRAME_SOF;
			p_stFrame->m_SysTick	=	p_stImg->m_SysTick;
			p_stFrame->m_FrameSize	=	p_stImg->m_SizeB;
			//			p_stFrame->m_PacketIdx	=	(p_stFrame->m_FrameSize)%FRAME_DATA_SIZE_B? (p_stFrame->m_FrameSize)/FRAME_DATA_SIZE_B: ((p_stFrame->m_FrameSize)/FRAME_DATA_SIZE_B)+1;


			if( p_stImg->m_SizeB > FRAME_DATA_SIZE_B)
			{
				packet_data_size_b  += FRAME_DATA_SIZE_B;
				p_stImg->m_SizeB 	-= FRAME_DATA_SIZE_B;
				p_stImg->m_pImg		+= FRAME_DATA_SIZE_B;
				p_stImg->m_eNextImgStates = eImgStates_sending;
			}
			else
			{
				packet_data_size_b  += p_stImg->m_SizeB;
				p_stImg->m_SizeB 	-= p_stImg->m_SizeB;
				p_stImg->m_pImg		+= p_stImg->m_SizeB;
				p_stImg->m_eNextImgStates = eImgStates_finished;
				PacketMngr_SetState(ePacketMngrState_off);

				CameraMngr_SetCompressedImgState(eCompressedImgState_SendCmplt);
			}

			memcpy(p_stFrame->m_Data, pData, packet_data_size_b);
//			printf("%d\ts=%d\r\n", HAL_GetTick(), packet_data_size_b);
			Result = send((socketIdx_t) a_Socket, (uint8_t*)p_stFrame , NEW_FRAME_HEADER_SIZE_B + packet_data_size_b, 0);
//			printf("%d\tas\r\n",HAL_GetTick());
		}
		break;
		case eImgStates_finished:
		{
			printf("F\t%d\r\n",HAL_GetTick());
		}
		break;
		default:
			break;
		}

		if(Result != SOCK_ERR_NO_ERROR)
		{

			printf("%d\t***F***%d\r\n",HAL_GetTick(),Result);
//			Error_Handler();
		}
	}
	return Img_jpg_UpdateImgState();
}

/* ================
void PacketMngr_IterateImu()
================ */
void PacketMngr_IterateImu()
{
	stPacketMngr* 	p_stPacketMngr 	= 	&g_stPacketMngr;
	if(p_stPacketMngr->m_IsImuCallReady == true)
	{
		p_stPacketMngr->m_IsImuCallReady = false;
		PacketMngr_GetNewImuCall();
		UPDATE_IMU_CALLS_CTR(p_stPacketMngr->m_ImuCallsPerPacket);
	}
}

/* ================
void PacketMngr_SendImu(int8_t a_Socket)
================ */
void PacketMngr_SendImu(int8_t a_Socket)
{
	stPacketMngr* 	p_stPacketMngr 	= 	&g_stPacketMngr;
	stImuPacket* p_stImuPacket = &p_stPacketMngr->m_stImuPacket;

	p_stImuPacket->m_ImuSOF = IMU_SOF;
	p_stImuPacket->m_SysTick = HAL_GetTick();

	if( (p_stPacketMngr->m_IsImuPacketReady == true)&&
			(p_stPacketMngr->m_ePacketMngrState == ePacketMngrState_off))
	{
		PacketMngr_SetState(ePacketMngrState_IMU);
		p_stPacketMngr->m_IsImuPacketReady =	false;

		printf("%d\timu\r\n",HAL_GetTick());

		int8_t Result = send((socketIdx_t) a_Socket, (uint8_t*)p_stImuPacket , sizeof(stImuPacket), 0);


		if(Result == SOCK_ERR_BUFFER_FULL) //SO: SOCK_ERR_BUFFER_FULL is received whenever user clicks on wifi connections
		{
			printf("%d\t***I***%d\r\n",HAL_GetTick(),Result);
//			Error_Handler();
		}
		PacketMngr_SetState(ePacketMngrState_off);
	}
}

/* ================
void PacketMngr_SetState( a_ePacketMngrState)
================ */
void PacketMngr_SetState(ePacketMngrState a_ePacketMngrState)
{
	g_stPacketMngr.m_ePacketMngrState = a_ePacketMngrState;
//	printf("*%d\r\n",a_ePacketMngrState);
}

/* ================
void PacketMngr_GetNewImg( void )
================ */
void PacketMngr_GetNewImg( void )
{
	Img_jpg_GetNewImg();
}

/* ================
bool PacketMngr_GetIsImgTickCam( void )
================ */
bool PacketMngr_GetIsImgTickCam( void )
{
	bool RetVal = false;

	if(g_stPacketMngr.m_IsImgTickCam == true)
	{
		RetVal = true;
		g_stPacketMngr.m_IsImgTickCam = false;
	}

	return RetVal;
}

/* ================
bool PacketMngr_GetIsWifiSendEvent( void )
================ */
bool PacketMngr_GetIsWifiSendEvent( void )
{
//	bool RetVal = false;
//
//	if(g_stPacketMngr.m_IsWifiTick == true)
//	{
//		RetVal = true;
//		g_stPacketMngr.m_IsWifiTick = false;
//	}

	return (g_stPacketMngr.m_IsWifiTick == COUNTING_ENDED_VAL);
}


/* ================
void PacketMngr_GetNewImuCall(void)
================ */
void PacketMngr_GetNewImuCall(void)
{

	stPacketMngr* 	p_stPacketMngr 		= 	&g_stPacketMngr;
	uint32_t		ImuCallsPerPacket	=	p_stPacketMngr->m_ImuCallsPerPacket;
	stImuCall*		p_stImuCall			= 	&p_stPacketMngr->m_stImuPacket.m_ImuCalls_a[ImuCallsPerPacket];

	stImuCall* pNewCall =  SD_MPU6050_GetImuCall();
	memcpy(p_stImuCall, pNewCall, sizeof(stImuCall));

}


/*****END OF FILE****/
