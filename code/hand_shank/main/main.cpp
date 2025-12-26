#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "board_config.h"
#include "battery_monitor.h"
#include "joystick.h"
#include "mode.h"
#include "control.h"

extern "C" void app_main(void)
{
	initArduino();
	battery_monitor_start();
	initMode();
	initControl();

	while (true)
	{
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}