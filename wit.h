#ifndef WIT_H
#define WIT_H

#define SERIAL1 1
#define SERIAL2 2

/* 初始化錯誤代碼 */
#define SERIAL_INIT_ERROR 1
#define WIT_INIT_ERROR 2

#include <Arduino.h>

class wit
{
private:

	/* 常量變數 */
	const uint8_t WIT_UNLOCK[5] = {0XFF, 0XAA, 0X69, 0X88, 0XB5,};

	uint16_t initTimeout = 10000;	//初始化超時時間

	/* 參數變數 */
	uint8_t serialPort;
	uint8_t rxPin;
	uint8_t txPin;
	uint32_t baudRate;

	/* Serial端口指標 */
	HardwareSerial* hwSerial = NULL;
	
public:

	wit(uint8_t serialPort, uint8_t rxPin, uint8_t txPin, uint32_t baudRate);
	int8_t wit_init();
};

#endif