#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void TaskBatteryMonitor(void *pvParameters)
{

	/* 電阻配置 */
	const float R_UP = 100000.0;					 // 上拉電阻
	const float R_DOWN = 360000.0;					 // 下拉電阻
	const float BAT_GAIN = (R_UP + R_DOWN) / R_DOWN; // 電池電壓增益

	/* ADC配置 */
	analogReadResolution(12);					   // 設置ADC解析度為12位
	analogSetPinAttenuation(GPIO_NUM_2, ADC_11db); // 設置GPIO2的ADC衰減為11dB
	analogReadMilliVolts(GPIO_NUM_2);			   // 預讀取以穩定ADC

	while (true)
	{
		/* 讀取電壓值 */
		uint32_t rawMilliVolts = analogReadMilliVolts(GPIO_NUM_2);
		float batteryVoltage = (rawMilliVolts / 1000.0) * BAT_GAIN;

		/* 輸出電壓值 */
		ESP_LOGI("BatteryMonitor", "Battery Voltage: %.2f V", batteryVoltage);
		vTaskDelay(pdMS_TO_TICKS(2000)); // 每2秒讀取一次
	}
}

extern "C" void app_main(void)
{
	initArduino();

	/* 創建任務 */
	xTaskCreate(TaskBatteryMonitor, "BatteryMonitorTask", 4096, NULL, 1, NULL);

	while (true)
	{
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}