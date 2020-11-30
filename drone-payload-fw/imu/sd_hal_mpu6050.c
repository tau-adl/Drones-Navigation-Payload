/*
 * sd_hal_mpu6050.c
 *
 *  Created on: Feb 19, 2016
 *      Author: Sina Darvishi
 */

/**
 * |----------------------------------------------------------------------
 * | Copyright (C) Sina Darvishi,2016
 * |
 * | This program is free software: you can redistribute it and/or modify
 * | it under the terms of the GNU General Public License as published by
 * | the Free Software Foundation, either version 3 of the License, or
 * | any later version.
 * |
 * | This program is distributed in the hope that it will be useful,
 * | but WITHOUT ANY WARRANTY; without even the implied warranty of
 * | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * | GNU General Public License for more details.
 * |
 * | You should have received a copy of the GNU General Public License
 * | along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * |----------------------------------------------------------------------
 */

#include "i2c.h"
#include "sd_hal_mpu6050.h"


/* Default I2C address */
#define MPU6050_I2C_ADDR			0xD0

/* Who I am register value */
#define MPU6050_I_AM				0x68

/* MPU6050 registers */
#define MPU6050_AUX_VDDIO			0x01
#define MPU6050_SMPLRT_DIV			0x19
#define MPU6050_CONFIG				0x1A
#define MPU6050_GYRO_CONFIG			0x1B
#define MPU6050_ACCEL_CONFIG		0x1C
#define MPU6050_MOTION_THRESH		0x1F
#define MPU6050_INT_PIN_CFG			0x37
#define MPU6050_INT_ENABLE			0x38
#define MPU6050_INT_STATUS			0x3A
#define MPU6050_ACCEL_XOUT_H		0x3B
#define MPU6050_ACCEL_XOUT_L		0x3C
#define MPU6050_ACCEL_YOUT_H		0x3D
#define MPU6050_ACCEL_YOUT_L		0x3E
#define MPU6050_ACCEL_ZOUT_H		0x3F
#define MPU6050_ACCEL_ZOUT_L		0x40
#define MPU6050_TEMP_OUT_H			0x41
#define MPU6050_TEMP_OUT_L			0x42
#define MPU6050_GYRO_XOUT_H			0x43
#define MPU6050_GYRO_XOUT_L			0x44
#define MPU6050_GYRO_YOUT_H			0x45
#define MPU6050_GYRO_YOUT_L			0x46
#define MPU6050_GYRO_ZOUT_H			0x47
#define MPU6050_GYRO_ZOUT_L			0x48
#define MPU6050_MOT_DETECT_STATUS	0x61
#define MPU6050_SIGNAL_PATH_RESET	0x68
#define MPU6050_MOT_DETECT_CTRL		0x69
#define MPU6050_USER_CTRL			0x6A
#define MPU6050_PWR_MGMT_1			0x6B
#define MPU6050_PWR_MGMT_2			0x6C
#define MPU6050_FIFO_COUNTH			0x72
#define MPU6050_FIFO_COUNTL			0x73
#define MPU6050_FIFO_R_W			0x74
#define MPU6050_WHO_AM_I			0x75

/* Gyro sensitivities in degrees/s */
#define MPU6050_GYRO_SENS_250		((float) 131)
#define MPU6050_GYRO_SENS_500		((float) 65.5)
#define MPU6050_GYRO_SENS_1000		((float) 32.8)
#define MPU6050_GYRO_SENS_2000		((float) 16.4)

/* Acce sensitivities in g/s */
#define MPU6050_ACCE_SENS_2			((float) 16384)
#define MPU6050_ACCE_SENS_4			((float) 8192)
#define MPU6050_ACCE_SENS_8			((float) 4096)
#define MPU6050_ACCE_SENS_16		((float) 2048)



static SD_MPU6050 g_mpu1 = {NULL};




SD_MPU6050_Result SD_MPU6050_Init(SD_MPU6050_Device DeviceNumber, SD_MPU6050_Accelerometer AccelerometerSensitivity, SD_MPU6050_Gyroscope GyroscopeSensitivity)
{
	SD_MPU6050* pThis 	= &g_mpu1;
	pThis->m_pI2Cx		= &hi2c2;

	uint8_t WHO_AM_I = (uint8_t)MPU6050_WHO_AM_I;
	uint8_t temp;
	uint8_t d[2];


	/* Format I2C address */
	pThis->Address = MPU6050_I2C_ADDR | (uint8_t)DeviceNumber;


	/* Check if device is connected */
	if(HAL_I2C_IsDeviceReady(pThis->m_pI2Cx,(uint16_t)pThis->Address,2,5)!=HAL_OK)
	{
				return SD_MPU6050_Result_Error;
	}
	/* Check who am I */
	//------------------
		/* Send address */
		if(HAL_I2C_Master_Transmit(pThis->m_pI2Cx,(uint16_t)pThis->Address, &WHO_AM_I, 1, MPU6050_I2C_TIMEOUT_MSEC) != HAL_OK)
		{
			return SD_MPU6050_Result_Error;
		}

		/* Receive multiple byte */
		if(HAL_I2C_Master_Receive(pThis->m_pI2Cx,(uint16_t)pThis->Address, &temp, 1, MPU6050_I2C_TIMEOUT_MSEC) != HAL_OK)
		{
			return SD_MPU6050_Result_Error;
		}

		/* Checking */
		while(temp != MPU6050_I_AM)
		{
				/* Return error */
				return SD_MPU6050_Result_DeviceInvalid;
		}
	//------------------

	/* Wakeup MPU6050 */
	//------------------
		/* Format array to send */
		d[0] = MPU6050_PWR_MGMT_1;
		d[1] = 0x00;

		/* Try to transmit via I2C */
		if(HAL_I2C_Master_Transmit(pThis->m_pI2Cx,(uint16_t)pThis->Address, (uint8_t *)d, 2, MPU6050_I2C_TIMEOUT_MSEC) != HAL_OK)
		{
					return SD_MPU6050_Result_Error;
		}
	//------------------

	/* Set sample rate to 1kHz */
	SD_MPU6050_SetDataRate(SD_MPU6050_DataRate_200Hz); //SO: every 5 msec

	/* Config accelerometer */
	SD_MPU6050_SetAccelerometer(AccelerometerSensitivity);

	/* Config Gyroscope */
	SD_MPU6050_SetGyroscope(GyroscopeSensitivity);

	/* Return OK */
	return SD_MPU6050_Result_Ok;
}

SD_MPU6050_Result SD_MPU6050_SetDataRate(uint8_t rate)
{
	SD_MPU6050* pThis 	= &g_mpu1;
	uint8_t d[2];

	/* Format array to send */
	d[0] = MPU6050_SMPLRT_DIV;
	d[1] = rate;

	/* Set data sample rate */
	while(HAL_I2C_Master_Transmit(pThis->m_pI2Cx,(uint16_t)pThis->Address,(uint8_t *)d,2,MPU6050_I2C_TIMEOUT_MSEC)!=HAL_OK);
	/*{
				return SD_MPU6050_Result_Error;
	}*/

	/* Return OK */
	return SD_MPU6050_Result_Ok;
}

SD_MPU6050_Result SD_MPU6050_SetAccelerometer(SD_MPU6050_Accelerometer AccelerometerSensitivity)
{
	SD_MPU6050* pThis 	= &g_mpu1;
	uint8_t temp;
	uint8_t regAdd =(uint8_t )MPU6050_ACCEL_CONFIG;

	/* Config accelerometer */
	while(HAL_I2C_Master_Transmit(pThis->m_pI2Cx,(uint16_t)pThis->Address,&regAdd, 1, MPU6050_I2C_TIMEOUT_MSEC) != HAL_OK);
	/*{
				return SD_MPU6050_Result_Error;
	}*/
	while(HAL_I2C_Master_Receive(pThis->m_pI2Cx,(uint16_t)pThis->Address, &temp, 1, MPU6050_I2C_TIMEOUT_MSEC) != HAL_OK);
	/*{
				return SD_MPU6050_Result_Error;
	}*/
	temp = (temp & 0xE7) | (uint8_t)AccelerometerSensitivity << 3;
	while(HAL_I2C_Master_Transmit(pThis->m_pI2Cx,(uint16_t)pThis->Address,&temp, 1, MPU6050_I2C_TIMEOUT_MSEC) != HAL_OK);
	/*{
				return SD_MPU6050_Result_Error;
	}*/

	/* Set sensitivities for multiplying gyro and accelerometer data */
	switch (AccelerometerSensitivity) {
		case SD_MPU6050_Accelerometer_2G:
			pThis->Acce_Mult = (float)1 / MPU6050_ACCE_SENS_2;
			break;
		case SD_MPU6050_Accelerometer_4G:
			pThis->Acce_Mult = (float)1 / MPU6050_ACCE_SENS_4;
			break;
		case SD_MPU6050_Accelerometer_8G:
			pThis->Acce_Mult = (float)1 / MPU6050_ACCE_SENS_8;
			break;
		case SD_MPU6050_Accelerometer_16G:
			pThis->Acce_Mult = (float)1 / MPU6050_ACCE_SENS_16;
			break;
		default:
			break;
		}

	/* Return OK */
	return SD_MPU6050_Result_Ok;
}

SD_MPU6050_Result SD_MPU6050_SetGyroscope(SD_MPU6050_Gyroscope GyroscopeSensitivity)
{
	uint8_t temp;
	SD_MPU6050* pThis 	= &g_mpu1;
	uint8_t regAdd =(uint8_t )MPU6050_GYRO_CONFIG;

	/* Config gyroscope */
	while(HAL_I2C_Master_Transmit(pThis->m_pI2Cx,(uint16_t)pThis->Address,&regAdd, 1, MPU6050_I2C_TIMEOUT_MSEC) != HAL_OK);
	/*{
				return SD_MPU6050_Result_Error;
	}*/
	while(HAL_I2C_Master_Receive(pThis->m_pI2Cx,(uint16_t)pThis->Address, &temp, 1, MPU6050_I2C_TIMEOUT_MSEC) != HAL_OK);
	/*{
				return SD_MPU6050_Result_Error;
	}*/
	temp = (temp & 0xE7) | (uint8_t)GyroscopeSensitivity << 3;
	while(HAL_I2C_Master_Transmit(pThis->m_pI2Cx,(uint16_t)pThis->Address,&temp, 1, MPU6050_I2C_TIMEOUT_MSEC) != HAL_OK);
	/*{
				return SD_MPU6050_Result_Error;
	}*/

	switch (GyroscopeSensitivity) {
			case SD_MPU6050_Gyroscope_250s:
				pThis->Gyro_Mult = (float)1 / MPU6050_GYRO_SENS_250;
				break;
			case SD_MPU6050_Gyroscope_500s:
				pThis->Gyro_Mult = (float)1 / MPU6050_GYRO_SENS_500;
				break;
			case SD_MPU6050_Gyroscope_1000s:
				pThis->Gyro_Mult = (float)1 / MPU6050_GYRO_SENS_1000;
				break;
			case SD_MPU6050_Gyroscope_2000s:
				pThis->Gyro_Mult = (float)1 / MPU6050_GYRO_SENS_2000;
				break;
			default:
				break;
		}
	/* Return OK */
	return SD_MPU6050_Result_Ok;
}

SD_MPU6050_Result SD_MPU6050_ReadAccelerometer(void)
{
	uint8_t data[6];
	uint8_t reg = MPU6050_ACCEL_XOUT_H;
	SD_MPU6050* pThis 	= &g_mpu1;

	/* Read accelerometer data */
	while(HAL_I2C_Master_Transmit(pThis->m_pI2Cx,(uint16_t)pThis->Address, &reg, 1, MPU6050_I2C_TIMEOUT_MSEC) != HAL_OK);

	while(HAL_I2C_Master_Receive(pThis->m_pI2Cx,(uint16_t)pThis->Address, data, 6, MPU6050_I2C_TIMEOUT_MSEC) != HAL_OK);

	/* Format */
	pThis->m_stImuCall.Accelerometer_X = (int16_t)(data[0] << 8 | data[1]);
	pThis->m_stImuCall.Accelerometer_Y = (int16_t)(data[2] << 8 | data[3]);
	pThis->m_stImuCall.Accelerometer_Z = (int16_t)(data[4] << 8 | data[5]);

	/* Return OK */
	return SD_MPU6050_Result_Ok;
}
SD_MPU6050_Result SD_MPU6050_ReadGyroscope(void)
{
	uint8_t data[6];
	uint8_t reg = MPU6050_GYRO_XOUT_H;
	SD_MPU6050* pThis 	= &g_mpu1;

	/* Read gyroscope data */
	while(HAL_I2C_Master_Transmit(pThis->m_pI2Cx,(uint16_t)pThis->Address, &reg, 1, MPU6050_I2C_TIMEOUT_MSEC) != HAL_OK);

	while(HAL_I2C_Master_Receive(pThis->m_pI2Cx,(uint16_t)pThis->Address, data, 6, MPU6050_I2C_TIMEOUT_MSEC) != HAL_OK);

	/* Format */
	pThis->m_stImuCall.Gyroscope_X = (int16_t)(data[0] << 8 | data[1]);
	pThis->m_stImuCall.Gyroscope_Y = (int16_t)(data[2] << 8 | data[3]);
	pThis->m_stImuCall.Gyroscope_Z = (int16_t)(data[4] << 8 | data[5]);

	/* Return OK */
	return SD_MPU6050_Result_Ok;
}
//SD_MPU6050_Result SD_MPU6050_ReadTemperature(void)
//{
//	uint8_t data[2];
//	int16_t temp;
//	uint8_t reg = MPU6050_TEMP_OUT_H;
//	SD_MPU6050* pThis 	= &g_mpu1;
//
//	/* Read temperature */
//	while(HAL_I2C_Master_Transmit(pThis->m_pI2Cx,(uint16_t)pThis->Address, &reg, 1, 1000) != HAL_OK);
//
//	while(HAL_I2C_Master_Receive(pThis->m_pI2Cx,(uint16_t)pThis->Address, data, 2, 1000) != HAL_OK);
//
//	/* Format temperature */
//	temp = (data[0] << 8 | data[1]);
//	pThis->Temperature = (float)((int16_t)temp / (float)340.0 + (float)36.53);
//
//	/* Return OK */
//	return SD_MPU6050_Result_Ok;
//}
SD_MPU6050_Result SD_MPU6050_ReadAll(void)
{
	uint8_t data[14];
	int16_t temp;
	uint8_t reg = MPU6050_ACCEL_XOUT_H;
	SD_MPU6050* pThis 	= &g_mpu1;
	SD_MPU6050_Result Result = SD_MPU6050_Result_Ok;

	/* Read full raw data, 14bytes */
	if (HAL_I2C_Master_Transmit(pThis->m_pI2Cx,(uint16_t)pThis->Address, &reg, 1, MPU6050_I2C_TIMEOUT_MSEC) != HAL_OK)
	{
		Result = SD_MPU6050_Result_DeviceInvalid;
	}

	else if (HAL_I2C_Master_Receive(pThis->m_pI2Cx,(uint16_t)pThis->Address, data, 14, MPU6050_I2C_TIMEOUT_MSEC) != HAL_OK)
	{
		Result = SD_MPU6050_Result_DeviceInvalid;
	}

	/* Format accelerometer data */
	pThis->m_stImuCall.Accelerometer_X = (int16_t)(data[0] << 8 | data[1]);
	pThis->m_stImuCall.Accelerometer_Y = (int16_t)(data[2] << 8 | data[3]);
	pThis->m_stImuCall.Accelerometer_Z = (int16_t)(data[4] << 8 | data[5]);

//	/* Format temperature */
//	temp = (data[6] << 8 | data[7]);
//	pThis->Temperature = (float)((float)((int16_t)temp) / (float)340.0 + (float)36.53);

	/* Format gyroscope data */
	pThis->m_stImuCall.Gyroscope_X = (int16_t)(data[8] << 8 | data[9]);
	pThis->m_stImuCall.Gyroscope_Y = (int16_t)(data[10] << 8 | data[11]);
	pThis->m_stImuCall.Gyroscope_Z = (int16_t)(data[12] << 8 | data[13]);

	/* Return OK */
	return Result;
}
SD_MPU6050_Result SD_MPU6050_EnableInterrupts(void)
{
	uint8_t temp;
	uint8_t reg[2] = {MPU6050_INT_ENABLE,0x21};
	SD_MPU6050* pThis 	= &g_mpu1;

	/* Enable interrupts for data ready and motion detect */
	while(HAL_I2C_Master_Transmit(pThis->m_pI2Cx,(uint16_t)pThis->Address, reg, 2, MPU6050_I2C_TIMEOUT_MSEC) != HAL_OK);

	uint8_t mpu_reg= MPU6050_INT_PIN_CFG;
	/* Clear IRQ flag on any read operation */
	while(HAL_I2C_Master_Transmit(pThis->m_pI2Cx,(uint16_t)pThis->Address, &mpu_reg, 1, MPU6050_I2C_TIMEOUT_MSEC) != HAL_OK);

	while(HAL_I2C_Master_Receive(pThis->m_pI2Cx,(uint16_t)pThis->Address, &temp, 14, MPU6050_I2C_TIMEOUT_MSEC) != HAL_OK);
	temp |= 0x10;
	reg[0] = MPU6050_INT_PIN_CFG;
	reg[1] = temp;
	while(HAL_I2C_Master_Transmit(pThis->m_pI2Cx,(uint16_t)pThis->Address, reg, 2, MPU6050_I2C_TIMEOUT_MSEC) != HAL_OK);

	/* Return OK */
	return SD_MPU6050_Result_Ok;
}
SD_MPU6050_Result SD_MPU6050_DisableInterrupts(void)
{
	uint8_t reg[2] = {MPU6050_INT_ENABLE,0x00};
	SD_MPU6050* pThis 	= &g_mpu1;

	/* Disable interrupts */
	while(HAL_I2C_Master_Transmit(pThis->m_pI2Cx,(uint16_t)pThis->Address,reg,2,MPU6050_I2C_TIMEOUT_MSEC)!=HAL_OK);
	/* Return OK */
	return SD_MPU6050_Result_Ok;
}
SD_MPU6050_Result SD_MPU6050_ReadInterrupts(SD_MPU6050_Interrupt* InterruptsStruct)
{
	uint8_t read;

	/* Reset structure */
	InterruptsStruct->Status = 0;
	uint8_t reg = MPU6050_INT_STATUS;
	SD_MPU6050* pThis 	= &g_mpu1;

	while(HAL_I2C_Master_Transmit(pThis->m_pI2Cx,(uint16_t)pThis->Address, &reg, 1, MPU6050_I2C_TIMEOUT_MSEC) != HAL_OK);

	while(HAL_I2C_Master_Receive(pThis->m_pI2Cx,(uint16_t)pThis->Address, &read, 14, MPU6050_I2C_TIMEOUT_MSEC) != HAL_OK);

	/* Fill value */
	InterruptsStruct->Status = read;
	/* Return OK */
	return SD_MPU6050_Result_Ok;
}

stImuCall* SD_MPU6050_GetImuCall(void)
{
	if(SD_MPU6050_ReadAll() != SD_MPU6050_Result_Ok)
	{
		printf("imu err\r\n");

		g_mpu1.m_stImuCall.Gyroscope_X = 0;
		g_mpu1.m_stImuCall.Gyroscope_Y = 0;
		g_mpu1.m_stImuCall.Gyroscope_Z = 0;
		g_mpu1.m_stImuCall.Accelerometer_X = 0;
		g_mpu1.m_stImuCall.Accelerometer_Y = 0;
		g_mpu1.m_stImuCall.Accelerometer_Z = 0;
	}

	return &g_mpu1.m_stImuCall;
}
