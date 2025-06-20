#ifndef WIT_H
#define WIT_H

#define SERIAL1 1
#define SERIAL2 2

/* 初始化錯誤代碼 */
#define SERIAL_INIT_ERROR 1
#define WIT_INIT_ERROR 2

/* 數據結構錯誤代碼 */
#define WIT_NO_DATA 1	//無數據
#define WIT_TIMEOUT 2	//超時
#define WIT_DATA_ERROR 3	//數據錯誤
#define WIT_UNKNOWN_ERROR 4	//未知錯誤

#define WIT_READING 1	//讀取中
#define WIT_VERITY_ERROR 2	//驗證錯誤

#define GRAVITATIONAL_ACCELERATION 9.80665	//重力加速度

#include <Arduino.h>

typedef struct 
{
	int8_t serialPort; 				//Serial端口
	uint8_t status = WIT_NO_DATA;	//狀態
	double xangle;					//角度
	double yangle;
	double zangle;
	uint8_t angle_status;
	double xangular_speed;			//角速度
	double yangular_speed;
	double zangular_speed;
	uint8_t angular_speed_status;
	double xacceleration;			//加速度
	double yacceleration;
	double zacceleration;
	uint8_t acceleration_status;
	double wquaternion;				//四元數
	double xquaternion;
	double yquaternion;
	double zquaternion;
	uint8_t quaternion_status;
	double temperature;				//溫度
} witData;

typedef struct
{
	double xangle;					//角度
	double yangle;
	double zangle;
} witDataAngle;

typedef struct
{
	double wquaternion;				//四元數
	double xquaternion;				
	double yquaternion;
	double zquaternion;
} witDataQuaternion;

typedef struct
{
	double xangular_speed;			//角速度
	double yangular_speed;
	double zangular_speed;
} witDataAngularSpeed;

typedef struct
{
	double xacceleration;			//加速度
	double yacceleration;
	double zacceleration;
} witDataAcceleration;

class wit
{
	private:

		/* 協議 */
		const uint8_t WIT_UNLOCK[5] = {0XFF, 0XAA, 0X69, 0X88, 0XB5,};	//解鎖協議

		/* 計時器 */
		const uint16_t initTimeout = 10000;	//初始化超時時間
		const uint16_t dataTimeout = 10;	//數據超時時間

		/* 參數變數 */
		uint8_t serialPort;
		uint8_t rxPin;
		uint8_t txPin;
		uint32_t baudRate;

		/* Serial端口指標 */
		HardwareSerial* hwSerial = NULL;

	public:

		wit(uint8_t serialPort, uint8_t rxPin, uint8_t txPin, uint32_t baudRate);
		int8_t wit_init();			//初始化Wit模組
		witData wit_get_data();		//獲取數據
		void wit_flush();			//清除數據
		uint8_t wit_serial_get();	//獲取Serial端口
};

#endif