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
		u64_t startTime = millis();									//開始時間
		while (millis() - startTime < initTimeout)					//超時計時器開始計時
		{
			/* wit感測器安裝檢測 */
			if(this->hwSerial->available()) 			//資料讀取
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
		return WIT_INIT_ERROR; //初始化超時
	}
	return SERIAL_INIT_ERROR;	//Serial初始化錯誤
}