/**
  ******************************************************************************
	WIFI_MNGR
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __packet_mngr_H
#define __packet_mngr_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "img_jpg_file.h"
#include "sd_hal_mpu6050.h"

#define USING_IMU
#define USING_FRAME

//#define IMU_EVENT_CYCLE_MSEC	10
//#define IMU_CALLS_PER_PACKET	10
//#define FRAME_EVENT_CYCLE_MSEC	100
//#define WIFI_TICK_CYCLE			4 // each cycle is 250usec


 /// SO: so far the optimal configuration
#define IMU_EVENT_CYCLE_MSEC	6
#define IMU_CALLS_PER_PACKET	10
#define FRAME_EVENT_CYCLE_MSEC	60
#define WIFI_TICK_CYCLE			4 // each cycle is 250usec

#define UPDATE_IMU_EVENT_CTR(x)			(x)++;\
										(x)=((x)%IMU_EVENT_CYCLE_MSEC)
#define UPDATE_IMU_CALLS_CTR(x)			(x)++;\
										(x)=((x)%IMU_CALLS_PER_PACKET)
#define UPDATE_FRAME_EVENT_CTR(x)		(x)++;\
										(x)=((x)%FRAME_EVENT_CYCLE_MSEC)
#define UPDATE_WIFI_TICK_CTR(x)			(x)++;\
										(x)=((x)%WIFI_TICK_CYCLE)
#define COUNTING_ENDED_VAL				0

#define FRAME_SOF	'!'

#define IMU_SOF		'i'

#define	FULL_PACKET_SIZE_B				1024
#define FRAME_DATA_SIZE_B				1017
#define NEW_FRAME_HEADER_SIZE_B			7


#pragma pack(push,1)
typedef struct stFrame_name {
	uint8_t		m_FrameSOF;
	uint32_t	m_SysTick;
	uint16_t	m_FrameSize;
	uint8_t		m_Data[FRAME_DATA_SIZE_B];
} stFrame;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct stImuPacket_name {
	uint8_t			m_ImuSOF;
	uint32_t		m_SysTick;
	stImuCall		m_ImuCalls_a[IMU_CALLS_PER_PACKET];
} stImuPacket;
#pragma pack(pop)



typedef enum ePacketMngrState
{
	ePacketMngrState_off = 0,
	ePacketMngrState_Frame,
	ePacketMngrState_IMU,
}ePacketMngrState;

typedef struct stPacketMngr_name {
	ePacketMngrState	m_ePacketMngrState;

	stImg*				m_p_stImg;
	uint32_t			m_FrameEventCtr;
	uint8_t				m_PacketBytes[FULL_PACKET_SIZE_B];

	stImuPacket			m_stImuPacket;
	uint32_t			m_ImuEventCtr;
	uint32_t			m_ImuCallsPerPacket;
	bool				m_IsImuCallReady;
	bool				m_IsImuPacketReady;

	bool				m_IsImgTickCam;
	uint32_t			m_IsWifiTick;

	uint32_t			m_Tick;

} stPacketMngr;


void PacketMngr_Init(void);
void PacketMngr_Start(void);
void PacketMngr_Stop(void);
void PacketMngr_TxRoutine(int8_t a_Socket);
void PacketMngr_Update(void);
void PacketMngr_GetNewImg( void );
bool PacketMngr_GetIsImgTickCam( void );
void PacketMngr_UpdateWifiTick(void);
#ifdef __cplusplus
}
#endif
#endif /*__packet_mngr_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
