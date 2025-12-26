#include "control.h"
#include "mode.h"
#include "joystick.h"
#include "board_config.h"
#include "board_prefs.h"
#include <Arduino.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

static const char *TAG = "Control"; // 日志標籤

static Joystick joystick(PIN_JOY_ADC1, PIN_JOY_ADC2);
static JoystickPrefs joystick_prefs;

static void TaskControl(void *pvParameters)
{
	joystick_prefs = getJoystickPrefs();
	if (!joystick_prefs.valid)
	{
		ESP_LOGW(TAG, "Joystick preferences are invalid, calibrating...");

		/* 切換到校準模式 */
		QueueHandle_t Queue_Mode = getQueuedMode(Mode::QueueList::QUEUE_MODE);
		Mode::Status calibrate_mode = Mode::Status::CALIBRATING;
		xQueueOverwrite(Queue_Mode, &calibrate_mode);

		/* 進行搖桿校準 */
		int8_t calib_result = joystick.JoystickCalibration();
	}

	while (true)
	{
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

void initControl()
{
	ESP_LOGI(TAG, "Initializing Control Module...");

	joystick.init();

	xTaskCreate(TaskControl, "TaskControl", 4096, NULL, 1, NULL);
}