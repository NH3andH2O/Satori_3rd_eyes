#include "wit.h"

wit::wit(uint8_t serialPort, uint8_t rxPin, uint8_t txPin, uint32_t baudRate)
{
	this->serialPort = serialPort;
	this->rxPin = rxPin;
	this->txPin = txPin;
	this->baudRate = baudRate;
}

int8_t wit::wit_init()
{
	/* Serial選擇 */
	if (serialPort == SERIAL1)
	{
		this->hwSerial = &Serial1;
	}
	else if (serialPort == SERIAL2)
	{
		this->hwSerial = &Serial2;
	}
	
	/* Serial初始化 */
	if(hwSerial)
	{
		this->hwSerial->begin(baudRate, SERIAL_8N1, rxPin, txPin);	//Serial初始化
		u64_t startTime = xTaskGetTickCount();									//開始時間
		while (xTaskGetTickCount() - startTime < initTimeout)					//超時計時器開始計時
		{
			/* wit感測器安裝檢測 */
			if(this->hwSerial->available() >= 2) 		//資料讀取
			{
				if(this->hwSerial->read() == 0x55)		//協議頭檢測
				{
					if(this->hwSerial->read() == 0x53)	//角度數據頭檢測
					{
						return 0;						//初始化成功
					}
				}
			}
		}
		return WIT_INIT_ERROR;	//初始化超時
	}
	return SERIAL_INIT_ERROR;	//Serial初始化錯誤
}

witData wit::wit_get_data()
{
	witData result;		//返回數據結構體
	uint8_t data[60];	//數據緩存
	uint8_t length;
	result.angle_status = WIT_READING;
	result.angular_speed_status = WIT_READING;
	result.acceleration_status = WIT_READING;
	result.quaternion_status = WIT_READING;
	result.serialPort = this->serialPort;	//返回Serial端口
	
	/* 數據讀取 */
	u64_t startTime = xTaskGetTickCount();					//開始時間
	while(this->hwSerial->available() < 60)		//确保足够的数据
	{
		if(xTaskGetTickCount() - startTime > dataTimeout)	//超時計時器開始計時
		{
			result.status = WIT_TIMEOUT;		//無數據
			length = this->hwSerial->available();
			uint8_t temp[length];
			this->hwSerial->readBytes(temp, length);
			return result;						//返回數據結構體
		}
	}
	this->hwSerial->readBytes(data, 60);	//讀取數據

	/* 數據解析 */
	for(uint8_t i = 0; i < 50; i++)
	{
		if(data[i] == 0x55)			//協議頭檢測
		{
			switch (data[i + 1])	//協議類型檢測
			{
			case 0x51:				//加速度
				if(data[i + 10] == (uint8_t)(data[i] + data[i + 1] + data[i + 2] + data[i + 3] + data[i + 4] + data[i + 5] + data[i + 6] + data[i + 7] + data[i + 8] + data[i + 9]))	//校驗和檢測
				{
					result.xacceleration = (int16_t)((int16_t)(data[i + 3] << 8) | data[i + 2]) / (double)32768 * (double)16 * (double)GRAVITATIONAL_ACCELERATION;	//加速度計算
					result.yacceleration = (int16_t)((int16_t)(data[i + 5] << 8) | data[i + 4]) / (double)32768 * (double)16 * (double)GRAVITATIONAL_ACCELERATION;
					result.zacceleration = (int16_t)((int16_t)(data[i + 7] << 8) | data[i + 6]) / (double)32768 * (double)16 * (double)GRAVITATIONAL_ACCELERATION;
					result.temperature = (int16_t)((int16_t)(data[i + 9] << 8) | data[i + 8]) / (double)100;						//溫度計算
					result.acceleration_status = 0;
				}
				else
				{
					result.acceleration_status = WIT_VERITY_ERROR;	//數據驗證錯誤
				}
				i += 10;	//跳過數據
				break;
			case 0x52:				//角速度
				if(data[i + 10] == (uint8_t)(data[i] + data[i + 1] + data[i + 2] + data[i + 3] + data[i + 4] + data[i + 5] + data[i + 6] + data[i + 7] + data[i + 8] + data[i + 9]))	//校驗和檢測
				{
					result.xangular_speed = (int16_t)((int16_t)(data[i + 3] << 8) | data[i + 2]) / (double)32768 * (double)2000;	//角速度計算
					result.yangular_speed = (int16_t)((int16_t)(data[i + 5] << 8) | data[i + 4]) / (double)32768 * (double)2000;
					result.zangular_speed = (int16_t)((int16_t)(data[i + 7] << 8) | data[i + 6]) / (double)32768 * (double)2000;
					result.angular_speed_status = 0;
				}
				else
				{
					result.angular_speed_status = WIT_VERITY_ERROR;	//數據驗證錯誤
				}
				i += 10;	//跳過數據
				break;
			case 0x53:				//角度
				if(data[i + 10] == (uint8_t)(data[i] + data[i + 1] + data[i + 2] + data[i + 3] + data[i + 4] + data[i + 5] + data[i + 6] + data[i + 7] + data[i + 8] + data[i + 9]))	//校驗和檢測
				{
					result.xangle = (int16_t)((int16_t)(data[i + 3] << 8) | data[i + 2]) / (double)32768 * (double)180;	//角度計算
					result.yangle = (int16_t)((int16_t)(data[i + 5] << 8) | data[i + 4]) / (double)32768 * (double)180;
					result.zangle = (int16_t)((int16_t)(data[i + 7] << 8) | data[i + 6]) / (double)32768 * (double)180;
					result.angle_status = 0;
				}
				else
				{
					result.angle_status = WIT_VERITY_ERROR;	//數據驗證錯誤
				}
				i += 10;	//跳過數據
				break;
			case 0x59:				//四元數
				if(data[i + 10] == (uint8_t)(data[i] + data[i + 1] + data[i + 2] + data[i + 3] + data[i + 4] + data[i + 5] + data[i + 6] + data[i + 7] + data[i + 8] + data[i + 9]))	//校驗和檢測
				{
					result.quaternion[0] = (int16_t)((int16_t)(data[i + 3] << 8) | data[i + 2]) / (double)32768;	//四元數計算
					result.quaternion[1] = (int16_t)((int16_t)(data[i + 5] << 8) | data[i + 4]) / (double)32768;
					result.quaternion[2] = (int16_t)((int16_t)(data[i + 7] << 8) | data[i + 6]) / (double)32768;
					result.quaternion[3] = (int16_t)((int16_t)(data[i + 9] << 8) | data[i + 8]) / (double)32768;
					result.quaternion_status = 0;
				}
				else
				{
					result.quaternion_status = WIT_VERITY_ERROR;	//數據驗證錯誤
				}
				i += 10;	//跳過數據
				break;
			default:
				break;
			}

			/* 數據狀態檢測 */
			if(result.angle_status != WIT_READING && result.angular_speed_status != WIT_READING && result.acceleration_status != WIT_READING)	//所有數據都讀取完成
			{
				if(result.angle_status == 0 && result.angular_speed_status == 0 && result.acceleration_status == 0)	//所有數據都正確
				{
					result.status = 0;				//數據正常
				}
				else
				{
					result.status = WIT_DATA_ERROR;	//數據驗證錯誤
				}
				length = this->hwSerial->available();
				uint8_t temp[length];
				this->hwSerial->readBytes(temp, length);		//清空Serial緩存
				return result;						//返回數據結構體
			}
		}
	}
	length = this->hwSerial->available();
	uint8_t temp[length];
	this->hwSerial->readBytes(temp, length);
	result.status = WIT_UNKNOWN_ERROR;	//數據錯誤
	return result;						//返回數據結構體
}

void wit::wit_flush()
{
	uint8_t temp[128];
	this->hwSerial->readBytes(temp, 128);	//讀取數據
}

uint8_t wit::wit_serial_get()
{
	return this->serialPort;	//返回Serial端口
}