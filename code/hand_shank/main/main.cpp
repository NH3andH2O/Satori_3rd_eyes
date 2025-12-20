#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "board_config.h"
#include "battery_monitor.h"

extern "C" void app_main(void)
{
	initArduino();
	battery_monitor_start();

	while (true)
	{
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}