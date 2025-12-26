#include "battery_monitor.h"

#include <Arduino.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "board_config.h"

/* 任務句柄 */
static TaskHandle_t Handle_TaskBatteryMonitor = nullptr;
static TaskHandle_t Handle_TaskBatteryDisplay = nullptr;

/* 佇列宣告 */
static QueueHandle_t Queue_BatteryLevel = nullptr;

static const char *TAG = "BatteryMonitor"; // 日志標籤
static uint32_t period_ms = 2000;		   // 讀取週期

enum BatteryLevel
{
	BAT_75,
	BAT_50,
	BAT_25,
	BAT_5,
	BAT_0
};

struct VoltageStats
{
	bool valid;		 // 是否有效
	uint32_t avg_mV; // 平均電壓
	float var_mV2;	 // 電壓方差
};

/* 根據當前電池電量等級更新電池電量等級 */
static BatteryLevel updateBatteryLevel(float v, BatteryLevel cur)
{
	switch (cur)
	{
		case BAT_75:
			if (v < 3.95)
				return BAT_50;
			break;

		case BAT_50:
			if (v < 3.75)
				return BAT_25;
			if (v > 4.00)
				return BAT_75;
			break;

		case BAT_25:
			if (v < 3.55)
				return BAT_5;
			if (v > 3.80)
				return BAT_50;
			break;

		case BAT_5:
			if (v < 3.30)
				return BAT_0;
			if (v > 3.60)
				return BAT_25;
			break;

		case BAT_0:
			if (v > 3.35)
				return BAT_5;
			break;
	}
	return cur;
}

/* 多次取樣並計算平均值與方差 */
static VoltageStats readVoltageAvgWithVariance(gpio_num_t adcPin, int N = 64, uint32_t varThreshold_mV2 = 4000, uint32_t sampleIntervalMs = 2)
{
	VoltageStats out{};
	out.valid = false;
	out.avg_mV = 0;
	out.var_mV2 = 0.0f;

	/* 基本參數檢查 */
	if (N < 4 || N > 128)
	{
		return out;
	}

	uint32_t buf[128]; // 暫存取樣資料（N <= 128）
	uint64_t sumAll = 0;

	uint32_t minVal = UINT_MAX;
	uint32_t maxVal = 0;

	/* 連續取樣 */
	for (int i = 0; i < N; i++)
	{
		uint32_t mv = (uint32_t)analogReadMilliVolts(adcPin);
		buf[i] = mv;
		sumAll += mv;

		/* 記錄最大與最小值 */
		if (mv < minVal)
		{
			minVal = mv;
		}
		if (mv > maxVal)
		{
			maxVal = mv;
		}

		vTaskDelay(pdMS_TO_TICKS(sampleIntervalMs));
	}

	/* 去除 1 個最大值與 1 個最小值 */
	uint64_t sumTrim = sumAll - minVal - maxVal;
	const uint8_t M = N - 2; // 去極值後的有效樣本數

	float mean = (float)sumTrim / (float)M;

	/* 計算方差（忽略 1 個最大與 1 個最小） */
	float var = 0.0f;
	bool skippedMin = false;
	bool skippedMax = false;

	for (uint8_t i = 0; i < N; i++)
	{
		uint32_t mv = buf[i];

		/* 僅跳過一次最小與最大值 */
		if (!skippedMin && mv == minVal)
		{
			skippedMin = true;
			continue;
		}
		if (!skippedMax && mv == maxVal)
		{
			skippedMax = true;
			continue;
		}

		float d = (float)mv - mean;
		var += d * d;
	}

	var /= (float)M;
	out.var_mV2 = var;

	/* 方差檢查 */
	if (var > (float)varThreshold_mV2)
	{
		/* 波動過大，丟棄此次結果 */
		out.valid = false;
		out.avg_mV = 0;
		return out;
	}

	/* 通過檢查 */
	out.valid = true;
	out.avg_mV = (uint32_t)(mean + 0.5f); // 四捨五入
	return out;
}

/* 電池讀取任務 */
static void TaskBatteryRead(void *pvParameters)
{
	const float BAT_GAIN = (R_UP + R_DOWN) / R_DOWN;
	BatteryLevel batttery_level = BAT_75;
	BatteryLevel last_battery_level = BAT_75;
	VoltageStats voltage_stats;
	float batteryVoltage = 0.0f;

	/* 初始化 ADC */
	analogReadResolution(12);
	analogSetPinAttenuation(PIN_BATTERY, ADC_11db);
	analogReadMilliVolts(PIN_BATTERY); // 預熱讀取
	vTaskDelay(pdMS_TO_TICKS(10));

	/* 初始化電池電壓狀態 */
	do
	{
		voltage_stats = readVoltageAvgWithVariance(PIN_BATTERY);
		vTaskDelay(pdMS_TO_TICKS(50));
	} while (!voltage_stats.valid);
	batteryVoltage = (voltage_stats.avg_mV / 1000.0f) * BAT_GAIN;
	for (uint8_t i = 0; i < 5; i++)
	{
		batttery_level = updateBatteryLevel(batteryVoltage, batttery_level);
	}
	xQueueSend(Queue_BatteryLevel, &batttery_level, 0);

	/* 進入主迴圈 */
	while (true)
	{
		/* 讀取電池電壓 */
		voltage_stats = readVoltageAvgWithVariance(PIN_BATTERY);
		if (!voltage_stats.valid)
		{
			ESP_LOGW(TAG, "Voltage reading invalid due to high variance.");
			vTaskDelay(pdMS_TO_TICKS(period_ms));
			continue;
		}
		batteryVoltage = (voltage_stats.avg_mV / 1000.0f) * BAT_GAIN;
		batttery_level = updateBatteryLevel(batteryVoltage, batttery_level);

		/* 傳遞電池電壓 */
		ESP_LOGD(TAG, "Battery Voltage: %.2f V", batteryVoltage);
		if (batttery_level != last_battery_level)
		{
			xQueueSend(Queue_BatteryLevel, &batttery_level, 0);
			last_battery_level = batttery_level;
		}
		vTaskDelay(pdMS_TO_TICKS(period_ms));
	}
}

/* 電池顯示任務 */
static void TaskBatteryDisplay(void *pvParameters)
{
	/* 初始化 */
	pinMode(PIN_BAT_LED1, OUTPUT);
	pinMode(PIN_BAT_LED2, OUTPUT);
	BatteryLevel battery_level;

	/* 取得電池電量 */
	xQueueReceive(Queue_BatteryLevel, &battery_level, portMAX_DELAY);

	/* 進入主迴圈 */
	while (true)
	{
		/* 根據電池電量顯示 */
		switch (battery_level)
		{
			case BAT_75: // 75%
				digitalWrite(PIN_BAT_LED1, HIGH);
				digitalWrite(PIN_BAT_LED2, HIGH);
				xQueueReceive(Queue_BatteryLevel, &battery_level, portMAX_DELAY);
				break;
			case BAT_50: // 50%
				digitalWrite(PIN_BAT_LED1, !(digitalRead(PIN_BAT_LED1)));
				digitalWrite(PIN_BAT_LED2, HIGH);
				xQueueReceive(Queue_BatteryLevel, &battery_level, 500);
				break;
			case BAT_25: // 25%
				digitalWrite(PIN_BAT_LED1, LOW);
				digitalWrite(PIN_BAT_LED2, HIGH);
				xQueueReceive(Queue_BatteryLevel, &battery_level, portMAX_DELAY);
				break;
			case BAT_5: // 5%
				digitalWrite(PIN_BAT_LED1, LOW);
				digitalWrite(PIN_BAT_LED2, !(digitalRead(PIN_BAT_LED2)));
				xQueueReceive(Queue_BatteryLevel, &battery_level, 500);
				break;
			case BAT_0: // 0%
				digitalWrite(PIN_BAT_LED1, LOW);
				digitalWrite(PIN_BAT_LED2, !(digitalRead(PIN_BAT_LED2)));
				xQueueReceive(Queue_BatteryLevel, &battery_level, 100);
				break;
			default:
				break;
		}
	}
}

void battery_monitor_start()
{
	if (Handle_TaskBatteryMonitor != nullptr)
	{
		return; // 已經在跑
	}

	Queue_BatteryLevel = xQueueCreate(3, sizeof(BatteryLevel));
	xTaskCreate(TaskBatteryRead, "TaskBatteryRead", 4096, nullptr, 1, &Handle_TaskBatteryMonitor);
	xTaskCreate(TaskBatteryDisplay, "TaskBatteryDisplay", 2048, nullptr, 1, &Handle_TaskBatteryDisplay);
}
void battery_monitor_stop()
{
	/* 刪除任務 */
	if (Handle_TaskBatteryMonitor != nullptr)
	{
		vTaskDelete(Handle_TaskBatteryMonitor);
		Handle_TaskBatteryMonitor = nullptr;
	}
	if (Handle_TaskBatteryDisplay != nullptr)
	{
		vTaskDelete(Handle_TaskBatteryDisplay);
		Handle_TaskBatteryDisplay = nullptr;
	}
}