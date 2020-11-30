/**
  ******************************************************************************
	WIFI_MNGR
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#include <string.h>
#include <stdbool.h>
#include "main.h"
#include "led.h"
#include "driver/include/m2m_types.h"
#include "common/include/nm_common.h"
#include "driver/include/m2m_wifi.h"

#include "wifi_mngr.h"
// debug stuff

#include "packet_mngr.h"
#include "usart.h"

// ============= statics & consts =============

#define IMG_JPG_SIZE_B	sizeof(img_jpg_file_a)

#define SIZE_OF_START_CMD_B		7
static char g_Start_cmd[SIZE_OF_START_CMD_B] 	= "Start\r\n";


static uint8_t g_WifiTxBuff[MAIN_WIFI_M2M_BUFFER_SIZE] = DEFAULT_TX_ARR;
/** Receive buffer definition. */
static uint8_t g_WifiRxBuff[MAIN_WIFI_M2M_BUFFER_SIZE];

static stWifiMngr g_stWifiMngr = {NULL};

/** Wi-Fi connection state */
//static uint8_t wifi_connected;
stSockAdd g_stSockAdd;
stAccPoint g_stAccPoint;

#ifdef CALC_TX_AVG_TIME
	static uint32_t ctr = 0;
	static uint32_t Tock = 0;
	static uint32_t Delta_a[NUM_OF_TX_TIMES_TO_AVG] = {0};
#endif // CALC_TX_AVG_TIME

// ============= private functions declaration =============

static void WifiMngr_WifiCB(uint8_t u8MsgType, void *pvMsg);
static void WifiMngr_SocketCB(socketIdx_t sock, uint8_t u8Msg, void *pvMsg);

static void WifiMngr_InitStruct(stWifiMngr* a_p_stWifiMngr);
//static void	WifiMngr_SendImage( stWifiMngr* a_p_stWifiMngr);

/*  ============= public functions implementation section  =============*/
/* ================
int8_t WifiMngr_Init(void)
================ */
int8_t WifiMngr_Init(void)
{
	tstrWifiInitParam param;
	int8_t ret;

	nm_bsp_init();

	/* Initialize Wi-Fi parameters structure. */
	memset((uint8_t *)&param, 0, sizeof(tstrWifiInitParam));

	/* ===== MAX PA GAIN ====== */ // SO: see if it changes something... ?
	m2m_wifi_set_tx_power(TX_PWR_HIGH);

	/* =================  Initialize Wi-Fi driver with data and status callbacks. ================= */
	//param.pfAppWifiCb = wifi_cb;
	param.pfAppWifiCb = (tpfAppWifiCb)WifiMngr_WifiCB;
	ret = m2m_wifi_init(&param);
	if (M2M_SUCCESS != ret)
	{
		Error_Handler();
	}

	/* Initialize socket address structure. */
	g_stSockAdd.sin_family = AF_INET;
	g_stSockAdd.sin_port = _htons((MAIN_WIFI_M2M_SERVER_PORT));
	g_stSockAdd.sin_addr.s_addr = 0;
	/* Initialize Socket module */
	socketInit();
	registerSocketCallback(WifiMngr_SocketCB, NULL);

	/* Initialize AP mode parameters structure with SSID, channel and OPEN security type. */
	memset(&g_stAccPoint, 0x00, sizeof(tstrM2MAPConfig));
	strcpy((char *)&g_stAccPoint.au8SSID, MAIN_WLAN_SSID);
	g_stAccPoint.u8ListenChannel = MAIN_WLAN_CHANNEL;
	g_stAccPoint.u8SecType = MAIN_WLAN_AUTH;
	g_stAccPoint.au8DHCPServerIP[0] = ATWINC_AP_IP_BYTE0;
	g_stAccPoint.au8DHCPServerIP[1] = ATWINC_AP_IP_BYTE1;
	g_stAccPoint.au8DHCPServerIP[2] = ATWINC_AP_IP_BYTE2;
	g_stAccPoint.au8DHCPServerIP[3] = ATWINC_AP_IP_BYTE3;

	/* Bring up AP mode with parameters structure. */
	ret = m2m_wifi_enable_ap(&g_stAccPoint);
	if (M2M_SUCCESS != ret) {
		printf("main: m2m_wifi_enable_ap call error!\r\n");
		Error_Handler();
	}

	stWifiMngr* p_stWifiMngr 		= &g_stWifiMngr;
	WifiMngr_InitStruct(p_stWifiMngr);

	// TODO: SO: remove later
	PacketMngr_GetNewImg();

	return ret;
}

/* ================
int8_t WifiMngr_ReInit(void)
================ */
int8_t WifiMngr_ReInit(void)
{
	m2m_wifi_disable_ap();
	socketDeinit();
	m2m_wifi_deinit(NULL);
	nm_bsp_reset();

	tstrWifiInitParam param;
	int8_t ret;

	nm_bsp_init();

	/* Initialize Wi-Fi parameters structure. */
	memset((uint8_t *)&param, 0, sizeof(tstrWifiInitParam));

	/* ===== MAX PA GAIN ====== */ // SO: see if it changes something... ?
	m2m_wifi_set_tx_power(TX_PWR_HIGH);

	/* =================  Initialize Wi-Fi driver with data and status callbacks. ================= */
	//param.pfAppWifiCb = wifi_cb;
	param.pfAppWifiCb = (tpfAppWifiCb)WifiMngr_WifiCB;
	ret = m2m_wifi_init(&param);
	if (M2M_SUCCESS != ret)
	{
		Error_Handler();
	}

	socketInit();
	registerSocketCallback(WifiMngr_SocketCB, NULL);

	/* Bring up AP mode with parameters structure. */
	ret = m2m_wifi_enable_ap(&g_stAccPoint);
	if (M2M_SUCCESS != ret) {
		printf("main: m2m_wifi_enable_ap call error!\r\n");
		Error_Handler();
	}

	stWifiMngr* p_stWifiMngr 		= &g_stWifiMngr;
	WifiMngr_InitStruct(p_stWifiMngr);

	PacketMngr_GetNewImg();

	return ret;
}

/* ================
 void	WifiMngr_HandleEvents(void)
================ */
sint8	WifiMngr_HandleEvents(void)
{

	sint8 ret = m2m_wifi_handle_events(NULL);

	stWifiMngr* p_stWifiMngr 	= &g_stWifiMngr;

	if(p_stWifiMngr->m_IsHardFault == false)
	{
		if (p_stWifiMngr->m_tcp_server_socket < 0) {
			/* Open TCP server socket */
			if ((p_stWifiMngr->m_tcp_server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
				//			printf("main: failed to create TCP server socket error!\r\n");
				return ret;
			}

			/* Bind service*/
			bind(p_stWifiMngr->m_tcp_server_socket, (struct sockaddr *)&g_stSockAdd, sizeof(struct sockaddr_in));
		}
		else
		{
			if(p_stWifiMngr->m_IsTxPhase == true)
			{
				PacketMngr_TxRoutine(p_stWifiMngr->m_tcp_client_socket);
			}
			else
			{
				if(p_stWifiMngr->m_IsRxReady == true) // got new incoming data
				{
					p_stWifiMngr->m_IsRxReady = false;
					if( strcmp(p_stWifiMngr->m_pRxBuff,g_Start_cmd) == 0)
					{
						memset(p_stWifiMngr->m_pRxBuff,0, SIZE_OF_START_CMD_B);
						printf("Got Start\r\n");
						if( LED_GetState() != eLedStates_blue_green)
						{
							LED_SetState(eLedStates_blue_green);
						}

						p_stWifiMngr->m_IsTxPhase = true;
						PacketMngr_Start();

					}
				}
				else
				{
					recv(p_stWifiMngr->m_tcp_client_socket, g_WifiRxBuff, sizeof(g_WifiRxBuff), 0);
				}
			}
		}
	}
	else
	{
		WifiMngr_ReInit();
	}
	return ret;
}

/* ================
static void wifi_cb(uint8_t u8MsgType, void *pvMsg)
================ */
static void WifiMngr_WifiCB(uint8_t u8MsgType, void *pvMsg)
{
	stWifiMngr* p_stWifiMngr 	= &g_stWifiMngr;
	switch (u8MsgType) {
	case M2M_WIFI_RESP_CON_STATE_CHANGED:
	{
		LED_SetState(eLedStates_red_green);
		tstrM2mWifiStateChanged *pstrWifiState = (tstrM2mWifiStateChanged *)pvMsg;
		if (pstrWifiState->u8CurrState == M2M_WIFI_CONNECTED) {
			p_stWifiMngr->m_eIsWifiConnected = M2M_WIFI_CONNECTED;
			printf("wifi_cb: CONNECTED\r\n");
			m2m_wifi_request_dhcp_client();
		} else if (pstrWifiState->u8CurrState == M2M_WIFI_DISCONNECTED) {
			printf("wifi_cb: DISCONNECTED\r\n");
			p_stWifiMngr->m_eIsWifiConnected = M2M_WIFI_DISCONNECTED;
			p_stWifiMngr->m_IsHardFault		= true;

			p_stWifiMngr->m_IsTxPhase = false;
			PacketMngr_Stop();
		}
	}
	break;

	case M2M_WIFI_REQ_DHCP_CONF: // SO: a device is connected to the AP provided by ATWINC1500
	{
		LED_SetState(eLedStates_all_on);
//		uint8_t *pu8IPAddress = (uint8_t*)pvMsg;
	}
	break;

	default:
		break;
	}
}

/* ================
static void socket_cb(SOCKET sock, uint8_t u8Msg, void *pvMsg)
================ */
static void WifiMngr_SocketCB(socketIdx_t sock, uint8_t u8Msg, void *pvMsg)
{
	stWifiMngr* p_stWifiMngr 	= &g_stWifiMngr;
	p_stWifiMngr->m_eSocketCbState = (eSocketCbState) u8Msg; // SO: update last Socket Cb state

	switch (u8Msg) {
	/* Socket bind */
	case SOCKET_MSG_BIND:
	{
		LED_SetState(eLedStates_red);
		tstrSocketBindMsg* pstrBind = (tstrSocketBindMsg*) pvMsg;
		if (pstrBind && pstrBind->status == 0) {
			listen(p_stWifiMngr->m_tcp_server_socket, 0);
		} else {
			printf("socket_cb: bind error!\r\n");
		}
	}
	break;

	/* Socket listen */
	case SOCKET_MSG_LISTEN:
	{
		LED_SetState(eLedStates_green);
		tstrSocketListenMsg *pstrListen = (tstrSocketListenMsg *)pvMsg;
		if (pstrListen && pstrListen->status == 0) {
			accept(p_stWifiMngr->m_tcp_server_socket, NULL, NULL);
		} else {
			printf("socket_cb: listen error!\r\n");
		}
	}
	break;

	/* Connect accept */
	case SOCKET_MSG_ACCEPT:
	{
		LED_SetState(eLedStates_blue);
		tstrSocketAcceptMsg* pstrAccept = (tstrSocketAcceptMsg*) pvMsg;
		if (pstrAccept) {
			accept(p_stWifiMngr->m_tcp_server_socket, NULL, NULL);
			p_stWifiMngr->m_tcp_client_socket = pstrAccept->sock;
			recv(p_stWifiMngr->m_tcp_client_socket, g_WifiRxBuff, sizeof(g_WifiRxBuff), 0);

			//p_stWifiMngr->m_IsTxPhase = false;
			//PacketMngr_Stop();
		} else {
			printf("socket_cb: accept error!\r\n");
		}
	}
	break;

	/* Message receive */
	case SOCKET_MSG_RECV:
	{
		LED_SetState(eLedStates_red_green);

		tstrSocketRecvMsg* pstrRecv = (tstrSocketRecvMsg*) pvMsg;
		if (pstrRecv && pstrRecv->s16BufferSize > 0)
		{
			p_stWifiMngr->m_IsRxReady 	= true;
			p_stWifiMngr->m_pRxBuff		= pstrRecv->pu8Buffer;
		}
		else
		{
			printf("socket_cb: recv error!\r\n");
			p_stWifiMngr->m_IsHardFault		= true;
		}
	}
	break;
	case SOCKET_MSG_SEND:
	{
//		if( LED_GetState() != eLedStates_blue_green)
//		{
//			LED_SetState(eLedStates_blue_green);
//		}
//
//		p_stWifiMngr->m_IsTxPhase = true;
	}
	break;
	default:
		break;
	}
}

/*  ============= private functions implementation section  =============*/

/* ================
void WifiMngr_Calc(void)
================ */
static void WifiMngr_InitStruct(stWifiMngr* a_p_stWifiMngr)
{
	a_p_stWifiMngr->m_eIsWifiConnected	= M2M_WIFI_UNDEF;
	a_p_stWifiMngr->m_eSocketCbState	= SOCKET_MSG_BIND;
	a_p_stWifiMngr->m_IsRxReady 		= false;
	a_p_stWifiMngr->m_IsTxPhase			= false;
	memset(g_WifiRxBuff,0,sizeof(g_WifiRxBuff));
	a_p_stWifiMngr->m_pRxBuff 			= g_WifiRxBuff;
	a_p_stWifiMngr->m_pTxBuff 			= g_WifiTxBuff;
	a_p_stWifiMngr->m_tcp_server_socket	= SOCKET_IDX_NOT_INIT;
	a_p_stWifiMngr->m_tcp_client_socket	= SOCKET_IDX_NOT_INIT;
	a_p_stWifiMngr->m_IsHardFault		= false;
}

///* ================
//void	WifiMngr_SendImage( stWifiMngr* a_p_stWifiMngr)
//================ */
//void WifiMngr_SendImage( stWifiMngr* a_p_stWifiMngr)
//{
//
//
//
//	eImgStates eOldImgState =  Img_jpg_GetImgState();// SO: taking old ImgState since Iterating changes it
//
//
//	// SO: split IterateImge into two functions: 1. prepares the msg
//
//	stImg* p_stImg = PacketMngr_IterateImg();
//
//	switch (eOldImgState)
//	{
//		case eImgStates_start:
//		{
//			send(a_p_stWifiMngr->m_tcp_client_socket, p_stImg->m_pImg, PACKET_DATA_LEN_B, 0);
//		}
//		case eImgStates_sending:
//		{
//			send(a_p_stWifiMngr->m_tcp_client_socket, p_stImg->m_pImg, PACKET_DATA_LEN_B, 0);
//		}
//		break;
//		case eImgStates_last_packet:
//		{
//			send(a_p_stWifiMngr->m_tcp_client_socket, p_stImg->m_pImg, p_stImg->m_SizeB, 0);
//		}
//		break;
//		case eImgStates_finished:
//		{
//		}
//		break;
//		default:
//			break;
//	}
//}



///* ================  ATWINC API =================
///* ================
// sint8 WifiMngr_ScanReq(tenuM2mScanCh a_e_tenuM2mScanCh)
//================ */
//sint8 WifiMngr_ScanReq(tenuM2mScanCh a_e_tenuM2mScanCh)
//{
//	return m2m_wifi_request_scan(a_e_tenuM2mScanCh);
//}


/* ================
WifiMngr_Connect(tenuM2mScanCh a_e_tenuM2mScanCh) //: SO: so far it's unused
================ */
//sint8 WifiMngr_Connect(tenuM2mScanCh a_e_tenuM2mScanCh)
//{
/*	return m2m_wifi_connect((char *)MAIN_WLAN_SSID, sizeof(MAIN_WLAN_SSID),\
							MAIN_WLAN_AUTH, (char *)MAIN_WLAN_PSK,\
						a_e_tenuM2mScanCh);
*/
//}

#ifdef CALC_TX_AVG_TIME
/* ================
void WifiMngr_Calc(void)
================ */
void WifiMngr_Calc(void)
{
	uint32_t Delta = HAL_GetTick()-Tock;
	Tock = HAL_GetTick();

	Delta_a[ctr] = Delta;
	ctr++;
	if(ctr >= NUM_OF_TX_TIMES_TO_AVG)
	{
		uint32_t avg = 0;
		for(uint32_t idx = OUTLINERS_TO_OMIT; idx<NUM_OF_TX_TIMES_TO_AVG;  idx++)
		{
			avg += Delta_a[idx];
		}
		avg /= (NUM_OF_TX_TIMES_TO_AVG-OUTLINERS_TO_OMIT);
		recv(tcp_client_socket, g_WifiRxBuff, sizeof(g_WifiRxBuff), 0);
	}
}

//		uint32 Tock = HAL_GetTick();
//// ===============
//	  	uint32 Tick = HAL_GetTick();
//	  	uint32 dt = Tick - Tock;
//	  	printf("c");

#endif // CALC_TX_AVG_TIME

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
