GLOBAL_VERBOSITY            =   0

MAIN_WIFI_M2M_BUFFER_SIZE   =   1024
FRAME_DATA_SIZE_B           =   1017
FRAME_HEADER_WO_SOF_SIZE_B  =   6
FRAME_HEADER_SIZE_B         =   7
IMU_PACkET_SIZE_B           =   125
SOF_SIZE_B                  =   1
IMU_PACKET_SIZE_WO_HEADER   =   (IMU_PACkET_SIZE_B - SOF_SIZE_B)

HOST = '192.168.1.1'  # Standard loopback interface address (localhost)
PORT = 6666  # Port to listen on (non-privileged ports are > 1023)
FRAME_SOF                   =	33 # '!'
IMU_SOF			            =   105# 'i'

IMU_SYSTICK_SHIFT_MSEC      =   50
IMU_CALLS_PER_PACKET        =   10
IMU_SOF_SIZE_B              =   1
SYSTICK_SIZE_B              =   4
IMU_DATA_SHIFT_SIZE_B       =   SYSTICK_SIZE_B
IMU_PARAMETER_SIZE_B        =   2
IMU_PARAMETERS_PER_CALL     =   6
IMU_CALL_SIZE_B             =   (IMU_PARAMETER_SIZE_B * IMU_PARAMETERS_PER_CALL)
GYRO_X_SHIFT                =   0
GYRO_Y_SHIFT                =   2
GYRO_Z_SHIFT                =   4
GYRO_BITS_TO_FLOAT          = 131.0
ACC_X_SHIFT                 =   6
ACC_Y_SHIFT                 =   8
ACC_Z_SHIFT                 =   10
ACC_BITS_TO_FLOAT           = 16384.0
IMU_CALLS_TIME_Delta_MSEC   = 6

IMG_H_TOP_CROP              = 64
IMG_H_BOTTOM_CROP           = 16
IMG_H_MARGIN                = 32
IMG_H                       = (240 + IMG_H_MARGIN)
IMG_W                       = 320

IMG_PLOT_PAUSE              = .001

TARGET_START_SEND_CMD       = b'Start\r\n'

SSID = 'WINC1500_AP'

ERR_IMG_PATH = r"err_img.jpg"

IS_DISP_ERR_IMG = False