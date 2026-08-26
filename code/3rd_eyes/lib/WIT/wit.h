#ifndef WIT_H
#define WIT_H

#define SERIAL1 1
#define SERIAL2 2

/* 初始化錯誤代碼 */
#define SERIAL_INIT_ERROR 1
#define WIT_INIT_ERROR 2
#define WIT_INIT_CANCELLED 3 // 模式切換取消初始化

/* 數據結構錯誤代碼 */
#define WIT_NO_DATA 1		// 無數據
#define WIT_TIMEOUT 2		// 超時
#define WIT_DATA_ERROR 3	// 數據錯誤
#define WIT_UNKNOWN_ERROR 4 // 未知錯誤
#define WIT_CANCELLED 5		// 模式切換取消讀取

#define WIT_READING 1	   // 讀取中
#define WIT_VERITY_ERROR 2 // 驗證錯誤

#define GRAVITATIONAL_ACCELERATION 9.80665 // 重力加速度

#include <Arduino.h>
#include <atomic>

typedef struct
{
	int8_t serialPort;			  // Serial端口
	uint8_t status = WIT_NO_DATA; // 狀態
	double xangle;				  // 角度
	double yangle;
	double zangle;
	uint8_t angle_status;
	double xangular_speed; // 角速度
	double yangular_speed;
	double zangular_speed;
	uint8_t angular_speed_status;
	double xacceleration; // 加速度
	double yacceleration;
	double zacceleration;
	uint8_t acceleration_status;
	double wquaternion; // 四元數
	double xquaternion;
	double yquaternion;
	double zquaternion;
	uint8_t quaternion_status;
	double temperature; // 溫度
} witData;

typedef struct
{
	double xangle; // 角度
	double yangle;
	double zangle;
} witDataAngle;

typedef struct
{
	double wquaternion; // 四元數
	double xquaternion;
	double yquaternion;
	double zquaternion;
} witDataQuaternion;

typedef struct
{
	double xangular_speed; // 角速度
	double yangular_speed;
	double zangular_speed;
} witDataAngularSpeed;

typedef struct
{
	double xacceleration; // 加速度
	double yacceleration;
	double zacceleration;
} witDataAcceleration;

class wit
{
  private:
	/* 協議 */
	static constexpr uint8_t WIT_UNLOCK[5] = {0xFF, 0xAA, 0x69, 0x88, 0xB5};		  // 解鎖
	static constexpr uint8_t WIT_SAVE[5] = {0xFF, 0xAA, 0x00, 0x00, 0x00};			  // 保存
	static constexpr uint8_t WIT_READ_VERSION[5] = {0xFF, 0xAA, 0x27, 0x2E, 0x00};	  // 讀取版本
	static constexpr uint8_t WIT_SET_RRATE_200HZ[5] = {0xFF, 0xAA, 0x03, 0x0B, 0x00}; // 設置速率200Hz
	static constexpr uint8_t WIT_SET_BAUD_4800[5] = {0xFF, 0xAA, 0x04, 0x01, 0x00};	  // 設置波特率4800
	static constexpr uint8_t WIT_SET_BAUD_9600[5] = {0xFF, 0xAA, 0x04, 0x02, 0x00};	  // 設置波特率9600
	static constexpr uint8_t WIT_SET_BAUD_19200[5] = {0xFF, 0xAA, 0x04, 0x03, 0x00};  // 設置波特率19200
	static constexpr uint8_t WIT_SET_BAUD_38400[5] = {0xFF, 0xAA, 0x04, 0x04, 0x00};  // 設置波特率38400
	static constexpr uint8_t WIT_SET_BAUD_57600[5] = {0xFF, 0xAA, 0x04, 0x05, 0x00};  // 設置波特率57600
	static constexpr uint8_t WIT_SET_BAUD_115200[5] = {0xFF, 0xAA, 0x04, 0x06, 0x00}; // 設置波特率115200
	static constexpr uint8_t WIT_SET_BAUD_230400[5] = {0xFF, 0xAA, 0x04, 0x07, 0x00}; // 設置波特率230400
	static constexpr uint8_t WIT_SET_RSW[5] = {0xFF, 0xAA, 0x02, 0x0E, 0x02};		  // 設置输出内容
	static constexpr uint8_t WIT_SET_ORIENT_H[5] = {0xFF, 0xAA, 0x23, 0x00, 0x00};	  // 設置水平安裝
	static constexpr uint8_t WIT_SET_ORIENT_V[5] = {0xFF, 0xAA, 0x23, 0x01, 0x00};	  // 設置垂直安裝
	static constexpr uint8_t WIT_SET_AXIS_6[5] = {0xFF, 0xAA, 0x24, 0x01, 0x00};	  // 設置6軸
	static constexpr uint8_t WIT_SET_AXIS_9[5] = {0xFF, 0xAA, 0x24, 0x00, 0x00};	  // 設置9軸

	static constexpr uint32_t BAUD_RATES[7] = {4800, 9600, 19200, 38400, 57600, 115200, 230400}; // 測試波特率

	/* 計時器 */
	static constexpr uint16_t initTimeout = 2000; // 初始化超時時間
	static constexpr uint16_t dataTimeout = 10;	  // 數據超時時間

	/* 參數變數 */
	uint8_t serialPort;
	uint8_t rxPin;
	uint8_t txPin;
	uint8_t axis;
	uint8_t orient;
	uint32_t baudRate;
	std::atomic<bool> stopRequested{false}; // 跨任務傳遞停止要求

	/* Serial端口指標 */
	HardwareSerial *hwSerial = NULL;

	/* 内部函數 */
	const uint8_t *GetWitBaudCommand(uint32_t baud);			  // 獲取設置波特率指令
	uint8_t wit_check_baudrate(uint32_t baudRate);				  // 檢查波特率
	void wit_send_command(const uint8_t *command, size_t length); // 發送指令
	bool wit_delay(uint32_t delayMs);							  // 可中斷等待

  public:
	wit(uint8_t serialPort, uint8_t rxPin, uint8_t txPin, uint32_t baudRate, uint8_t axis, uint8_t orient);
	int8_t wit_init();		// 初始化Wit模組
	witData wit_get_data(); // 獲取數據
	void wit_flush();		// 清除數據
	/* 任務安全停止 */
	void wit_request_stop();
	void wit_clear_stop();
	bool wit_stop_requested() const;
	void wit_end();
	uint8_t wit_serial_get(); // 獲取Serial端口
};

#endif