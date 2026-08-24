#include "wit.h"

wit::wit(uint8_t serialPort, uint8_t rxPin, uint8_t txPin, uint32_t baudRate, uint8_t axis, uint8_t orient)
{
	this->serialPort = serialPort;
	this->rxPin = rxPin;
	this->txPin = txPin;
	this->baudRate = baudRate;
	this->axis = axis;
	this->orient = orient;
}

const uint8_t *wit::GetWitBaudCommand(uint32_t baud)
{
	switch (baud)
	{
		case 4800:
			return WIT_SET_BAUD_4800;
		case 9600:
			return WIT_SET_BAUD_9600;
		case 19200:
			return WIT_SET_BAUD_19200;
		case 38400:
			return WIT_SET_BAUD_38400;
		case 57600:
			return WIT_SET_BAUD_57600;
		case 115200:
			return WIT_SET_BAUD_115200;
		case 230400:
			return WIT_SET_BAUD_230400;
		default:
			return nullptr; // 不支援的波特率
	}
}

uint8_t wit::wit_check_baudrate(uint32_t baudRate)
{
	/* 檢查波特率 */
	this->hwSerial->begin(baudRate, SERIAL_8N1, this->rxPin, this->txPin); // Serial初始化
	vTaskDelay(100);													   // 等待Serial初始化完成
	this->hwSerial->flush(false);
	wit_send_command(WIT_UNLOCK, 5); // 解鎖指令發送
	ESP_LOGV("WIT", "(rx:%d, tx:%d) Checking WIT module baud rate: %d", this->rxPin, this->txPin, baudRate);
	uint64_t startTime = xTaskGetTickCount();			  // 開始時間
	while (xTaskGetTickCount() - startTime < initTimeout) // 設置波特率
	{
		/* wit感測器安裝檢測 */
		wit_send_command(WIT_READ_VERSION, 5); // 讀取版本指令發送
		if (this->hwSerial->available() >= 11) // 資料讀取
		{
			if (this->hwSerial->read() == 0x55) // 協議頭檢測
			{
				if (this->hwSerial->read() == 0x5F)
				{
					uint8_t version[9];
					this->hwSerial->readBytes(version, 9); // 讀取版本號
					if (version[8] == (uint8_t)(0x55 + 0x5F + version[0] + version[1] + version[2] + version[3] + version[4] + version[5] +
												version[6] + version[7])) // 校驗和檢測
					{
						ESP_LOGV("WIT", "(rx:%d, tx:%d) module baud rate %d check successful.", this->rxPin, this->txPin, baudRate);
						return 1;
					}
				}
			}
		}
	}
	this->hwSerial->end(); // Serial結束
	ESP_LOGD("WIT", "(rx:%d, tx:%d) module baud rate %d check failed.", this->rxPin, this->txPin, baudRate);
	return 0;
}

void wit::wit_send_command(const uint8_t *command, size_t length)
{
	if (!this->hwSerial)
	{
		return;
	}
	this->hwSerial->write(command, length); // 發送指令
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
	if (!this->hwSerial)
	{
		ESP_LOGE("WIT", "(rx:%d, tx:%d) module initialization failed; Serial initialization error.", this->rxPin, this->txPin);
		return SERIAL_INIT_ERROR; // Serial初始化錯誤
	}

	/* 波特率檢查 */
	if (!wit_check_baudrate(this->baudRate)) // 檢查目標波特率失敗
	{
		/* 自動波特率檢查 */
		for (auto baud : BAUD_RATES)
		{
			if (wit_check_baudrate(baud)) // 找到非目標波特率
			{
				wit_send_command(this->WIT_UNLOCK, 5); // 解鎖
				vTaskDelay(300);
				wit_send_command(GetWitBaudCommand(this->baudRate), 5); // 設置波特率
				vTaskDelay(300);
				wit_send_command(this->WIT_SAVE, 5); // 保存
				vTaskDelay(300);
				break;
			}
		}
		this->hwSerial->end(); // Serial結束

		/* 波特率設置後檢查 */
		if (!wit_check_baudrate(baudRate))
		{
			ESP_LOGE("WIT", "(rx:%d, tx:%d) module initialization failed; baud rate check error.", this->rxPin, this->txPin);
			return WIT_INIT_ERROR; // 初始化检查錯誤
		}
	}

	/* 其他參數設置初始化 */
	vTaskDelay(300);
	wit_send_command(this->WIT_UNLOCK, 5); // 解鎖
	vTaskDelay(300);
	wit_send_command(this->WIT_SET_RRATE_200HZ, 5); // 設置速率200Hz
	vTaskDelay(300);
	wit_send_command(this->WIT_SET_RSW, 5); // 設置输出内容
	vTaskDelay(300);
	if (this->orient == 0)
	{
		wit_send_command(this->WIT_SET_ORIENT_H, 5); // 設置水平安裝
	}
	else
	{
		wit_send_command(this->WIT_SET_ORIENT_V, 5); // 設置垂直安裝
	}
	vTaskDelay(300);
	if (this->axis == 6)
	{
		wit_send_command(this->WIT_SET_AXIS_6, 5); // 設置6軸
	}
	else if (this->axis == 9)
	{
		wit_send_command(this->WIT_SET_AXIS_9, 5); // 設置9軸
	}
	vTaskDelay(300);
	wit_send_command(this->WIT_SAVE, 5); // 保存

	/* 獲取wit版本 */

	uint64_t startTime = xTaskGetTickCount(); // 開始時間
	uint16_t version;
	bool versionCheck = false;
	while (xTaskGetTickCount() - startTime < initTimeout) // 設置波特率
	{
		wit_send_command(WIT_READ_VERSION, 5); // 讀取版本指令發送
		if (this->hwSerial->available() >= 11) // 資料讀取
		{
			if (this->hwSerial->read() == 0x55) // 協議頭檢測
			{
				if (this->hwSerial->read() == 0x5F)
				{
					uint8_t version_data[9];
					this->hwSerial->readBytes(version_data, 9); // 讀取版本號
					if (version_data[8] == (uint8_t)(0x55 + 0x5F + version_data[0] + version_data[1] + version_data[2] + version_data[3] +
													 version_data[4] + version_data[5] + version_data[6] + version_data[7])) // 校驗和檢測
					{
						versionCheck = true;
						version = (version_data[1] << 8) | version_data[0];
						break;
					}
				}
			}
		}
	}

	if (!versionCheck)
	{
		ESP_LOGW("WIT", "(rx:%d, tx:%d) version check error.", this->rxPin, this->txPin);
		ESP_LOGI("WIT", "(rx:%d, tx:%d) module initialized successfully; baud: %d; axes: %d; orientation: %s", this->rxPin, this->txPin,
				 this->baudRate, this->axis, (this->orient == 0) ? "horizontal" : "vertical");
	}
	else
	{
		ESP_LOGI("WIT", "(rx:%d, tx:%d) module initialized successfully; version: %d; baud: %d; axes: %d; orientation: %s", this->rxPin, this->txPin,
				 version, this->baudRate, this->axis, (this->orient == 0) ? "horizontal" : "vertical");
	}
	return 0; // 初始化成功
}

witData wit::wit_get_data()
{
	witData result;	  // 返回數據結構體
	uint8_t data[60]; // 數據緩存
	uint8_t length;
	result.angle_status = WIT_READING;
	result.angular_speed_status = WIT_READING;
	result.acceleration_status = WIT_READING;
	result.quaternion_status = WIT_READING;
	result.serialPort = this->serialPort; // 返回Serial端口

	/* 數據讀取 */
	u64_t startTime = xTaskGetTickCount();	 // 開始時間
	while (this->hwSerial->available() < 60) // 确保足够的数据
	{
		if (xTaskGetTickCount() - startTime > dataTimeout) // 超時計時器開始計時
		{
			result.status = WIT_TIMEOUT; // 無數據
			length = this->hwSerial->available();
			uint8_t temp[length];
			this->hwSerial->readBytes(temp, length);
			return result; // 返回數據結構體
		}
	}
	this->hwSerial->readBytes(data, 60); // 讀取數據

	/* 數據解析 */
	for (uint8_t i = 0; i < 50; i++)
	{
		if (data[i] == 0x55) // 協議頭檢測
		{
			switch (data[i + 1]) // 協議類型檢測
			{
				case 0x51: // 加速度
					if (data[i + 10] == (uint8_t)(data[i] + data[i + 1] + data[i + 2] + data[i + 3] + data[i + 4] + data[i + 5] + data[i + 6] +
												  data[i + 7] + data[i + 8] + data[i + 9])) // 校驗和檢測
					{
						result.xacceleration = (int16_t)((int16_t)(data[i + 3] << 8) | data[i + 2]) / (double)32768 * (double)16 *
											   (double)GRAVITATIONAL_ACCELERATION; // 加速度計算
						result.yacceleration =
							(int16_t)((int16_t)(data[i + 5] << 8) | data[i + 4]) / (double)32768 * (double)16 * (double)GRAVITATIONAL_ACCELERATION;
						result.zacceleration =
							(int16_t)((int16_t)(data[i + 7] << 8) | data[i + 6]) / (double)32768 * (double)16 * (double)GRAVITATIONAL_ACCELERATION;
						result.temperature = (int16_t)((int16_t)(data[i + 9] << 8) | data[i + 8]) / (double)100; // 溫度計算
						result.acceleration_status = 0;
					}
					else
					{
						result.acceleration_status = WIT_VERITY_ERROR; // 數據驗證錯誤
					}
					i += 10; // 跳過數據
					break;
				case 0x52: // 角速度
					if (data[i + 10] == (uint8_t)(data[i] + data[i + 1] + data[i + 2] + data[i + 3] + data[i + 4] + data[i + 5] + data[i + 6] +
												  data[i + 7] + data[i + 8] + data[i + 9])) // 校驗和檢測
					{
						result.xangular_speed = (int16_t)((int16_t)(data[i + 3] << 8) | data[i + 2]) / (double)32768 * (double)2000; // 角速度計算
						result.yangular_speed = (int16_t)((int16_t)(data[i + 5] << 8) | data[i + 4]) / (double)32768 * (double)2000;
						result.zangular_speed = (int16_t)((int16_t)(data[i + 7] << 8) | data[i + 6]) / (double)32768 * (double)2000;
						result.angular_speed_status = 0;
					}
					else
					{
						result.angular_speed_status = WIT_VERITY_ERROR; // 數據驗證錯誤
					}
					i += 10; // 跳過數據
					break;
				case 0x53: // 角度
					if (data[i + 10] == (uint8_t)(data[i] + data[i + 1] + data[i + 2] + data[i + 3] + data[i + 4] + data[i + 5] + data[i + 6] +
												  data[i + 7] + data[i + 8] + data[i + 9])) // 校驗和檢測
					{
						result.xangle = (int16_t)((int16_t)(data[i + 3] << 8) | data[i + 2]) / (double)32768 * (double)180; // 角度計算
						result.yangle = (int16_t)((int16_t)(data[i + 5] << 8) | data[i + 4]) / (double)32768 * (double)180;
						result.zangle = (int16_t)((int16_t)(data[i + 7] << 8) | data[i + 6]) / (double)32768 * (double)180;
						result.angle_status = 0;
					}
					else
					{
						result.angle_status = WIT_VERITY_ERROR; // 數據驗證錯誤
					}
					i += 10; // 跳過數據
					break;
				case 0x59: // 四元數
					if (data[i + 10] == (uint8_t)(data[i] + data[i + 1] + data[i + 2] + data[i + 3] + data[i + 4] + data[i + 5] + data[i + 6] +
												  data[i + 7] + data[i + 8] + data[i + 9])) // 校驗和檢測
					{
						result.wquaternion = (int16_t)((int16_t)(data[i + 3] << 8) | data[i + 2]) / (double)32768; // 四元數計算
						result.xquaternion = (int16_t)((int16_t)(data[i + 5] << 8) | data[i + 4]) / (double)32768;
						result.yquaternion = (int16_t)((int16_t)(data[i + 7] << 8) | data[i + 6]) / (double)32768;
						result.zquaternion = (int16_t)((int16_t)(data[i + 9] << 8) | data[i + 8]) / (double)32768;
						result.quaternion_status = 0;
					}
					else
					{
						result.quaternion_status = WIT_VERITY_ERROR; // 數據驗證錯誤
					}
					i += 10; // 跳過數據
					break;
				default:
					break;
			}

			/* 數據狀態檢測 */
			if (result.angle_status != WIT_READING && result.angular_speed_status != WIT_READING && result.acceleration_status != WIT_READING &&
				result.quaternion_status != WIT_READING) // 所有數據都讀取完成
			{
				if (result.angle_status == 0 && result.angular_speed_status == 0 && result.acceleration_status == 0 &&
					result.quaternion_status == 0) // 所有數據都正確
				{
					result.status = 0; // 數據正常
				}
				else
				{
					result.status = WIT_DATA_ERROR; // 數據驗證錯誤
				}
				length = this->hwSerial->available();
				uint8_t temp[length];
				this->hwSerial->readBytes(temp, length); // 清空Serial緩存
				return result;							 // 返回數據結構體
			}
		}
	}
	length = this->hwSerial->available();
	uint8_t temp[length];
	this->hwSerial->readBytes(temp, length);
	result.status = WIT_UNKNOWN_ERROR; // 數據錯誤
	return result;					   // 返回數據結構體
}

void wit::wit_flush()
{
	if (this->hwSerial)
	{
		this->hwSerial->flush(false);
	}
}

uint8_t wit::wit_serial_get()
{
	return this->serialPort; // 返回Serial端口
}