/*
 * camera_mngr.h
 */

#ifndef INC_CAMERA_MNGR_H_
#define INC_CAMERA_MNGR_H_

#define CAMERA_BENCHMARK
//#define SAVE_INPUT_IMG_ON_SD
//#define SAVE_OUTPUT_IMG_ON_SD


//#define JPEG_444_GS_MCU_SIZE        (64)
// 640 * 480 = 307200
//#define FRAME_SIZE		FRAMESIZE_VGA
//#define IMG_W			(640)
//#define IMG_H			(480)

// 320 * 240 = 76800
#define FRAME_SIZE		FRAMESIZE_QVGA
#define IMG_W			(320)
#define IMG_H			(240)

//// 160 * 120 = 19200
//#define FRAME_SIZE		FRAMESIZE_QQVGA
//#define IMG_W			(160)
//#define IMG_H			(120)

#define DCMI_ACQ_TIMEOUT_MSEC				40

#define IMG_H_MARGIN	32

#define FRAME_BUFF_SIZE  ( (IMG_H + IMG_H_MARGIN) * IMG_W )
ALIGN_32BYTES( uint8_t CameraFrameBuff[FRAME_BUFF_SIZE] );

#define JPEG_BUFF_MAX_SIZE	15000
ALIGN_32BYTES( uint8_t JpegFrameBuff[JPEG_BUFF_MAX_SIZE	] );

ALIGN_32BYTES( uint8_t CompressedImg[JPEG_BUFF_MAX_SIZE	] );

typedef enum CamImgState
{
	eCamImgState_Init			= 0,
	eCamImgState_AcqStart,
	eCamImgState_AcqCmplt,
	eCamImgState_CompressStart,
	eCamImgState_CompressCmplt,
	eCamImgState_MaxVal
}eCamImgState;

typedef enum CompressedImgState
{
	eCompressedImgState_Init			= 0,
	eCompressedImgState_WaitForSend,
	eCompressedImgState_SendStart,
	eCompressedImgState_SendCmplt,
	eCompressedImgState_MaxVal
}eCompressedImgState;

typedef struct stCameraMngr_name {

	uint8_t*			m_pFrameBuff;	// camera work buffer
	uint32_t			m_FrameWidth;
	uint32_t			m_FrameHeight;
	uint32_t			m_FrameBuffSize;

	uint8_t*			m_pJpegFrameBuff;	// work buffer for jpeg compression
	uint32_t			m_JpegFrameBuffSize;
	uint32_t			m_JpegFrameBuffConvSize;

	uint8_t*			m_pCompressedImg; //  holds the buffer to ground station
	uint32_t			m_CompressedImgSize;
	eCompressedImgState	m_eCompressedImgState;

	eCamImgState		m_eCamImgState;

	uint32_t			m_JpegFrameBuffChecksum;

	uint32_t			m_DcmiFrameAcqStartTick;

#ifdef CAMERA_BENCHMARK
	uint32_t			m_JpegConvStartTick;
	uint32_t			m_JpegConvDuration_msec;


	uint32_t			m_DcmiFrameAcqDuration_msec;
#endif // CAMERA_BENCHMARK
} stCameraMngr;




void CameraMngr_Init(void);

void CameraMngr_WaitForFrame(void);
void CameraMngr_DcmiFrameAcqDma(void);

eCamImgState CameraMngr_GetCamImgState(void);
void CameraMngr_SetCamImgState(eCamImgState a_eCamImgState);
eCompressedImgState CameraMngr_GetCompressedImgState(void);
void CameraMngr_SetCompressedImgState(eCompressedImgState a_eCompressedImgState);

uint8_t* CameraMngr_GetFrameBuff(void);
uint8_t* CameraMngr_GetCompressedImg(void);
uint32_t CameraMngr_GetCompressedImgSize(void);

bool CameraMngr_isDcmiAcqRunning( void );
bool CameraMngr_isDcmiAcqEndded( void );

void CameraMngr_CompressStart(void);
void CameraMngr_CompressProc(void);
void CameraMngr_CompressEnd(void);
void CameraMngr_CompressBenchmark(void);

uint32_t CameraMngr_GetCompressedImgChecksum(void);

void CameraMngr_HandleEvents(void);

#endif /* INC_CAMERA_MNGR_H_ */
