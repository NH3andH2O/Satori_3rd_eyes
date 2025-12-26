#include "mode.h"
#include <Arduino.h>
#include "board_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

using namespace Mode;

static constexpr const char *TAG = "MODE";
static Status current_mode = Status::IDLE;

/* 任務句柄 */
static TaskHandle_t Handle_TaskMode = nullptr;
static TaskHandle_t Handle_TaskLEDBlink = nullptr;

/* 佇列 */
static QueueHandle_t Queue_Mode = nullptr;
static QueueHandle_t Queue_LEDBlink = nullptr;

/** 任務函數 **/
/* LED閃爍任務 */
static void TaskLEDBlink(void *pvParameters)
{
	BlinkSet blink_state;
	while (true)
	{
		xQueueReceive(Queue_LEDBlink, &blink_state, portMAX_DELAY);
		for (uint8_t i = 0; i < blink_state.times; i++)
		{
			digitalWrite(PIN_MODE_LED, HIGH);
			vTaskDelay(pdMS_TO_TICKS(blink_state.delay));
			digitalWrite(PIN_MODE_LED, LOW);
			vTaskDelay(pdMS_TO_TICKS(blink_state.delay));
		}
	}
}

/* 模式管理任務 */
static void TaskMode(void *pvParameters)
{
	while (true)
	{
		switch (current_mode)
		{
			case Status::IDLE:
				digitalWrite(PIN_MODE_LED, HIGH); // 閒置模式，LED常亮
				xQueueReceive(Queue_Mode, &current_mode, portMAX_DELAY);
				break;
			case Status::CALIBRATING:
				digitalWrite(PIN_MODE_LED, LOW); // 校準模式，LED熄
				xTaskCreate(TaskLEDBlink, "TaskLEDBlink", 2048, NULL, 1, &Handle_TaskLEDBlink);
				xQueueReceive(Queue_Mode, &current_mode, portMAX_DELAY);

				/* 退出任務 */
				if (current_mode != Status::CALIBRATING)
				{
					vTaskDelete(Handle_TaskLEDBlink); // 刪除LED閃爍任務
					Handle_TaskLEDBlink = nullptr;
				}
			default:
				ESP_LOGW(TAG, "Unknown mode, switching to IDLE");
				current_mode = Mode::IDLE;
				break;
		}
	}
}

/* 模式初始化 */
void initMode()
{
	pinMode(PIN_MODE_LED, OUTPUT);
	digitalWrite(PIN_MODE_LED, HIGH);			  // 初始狀態開啓LED，表明程序啓動
	Queue_Mode = xQueueCreate(1, sizeof(Status)); // 創建佇列
	Queue_LEDBlink = xQueueCreate(1, sizeof(BlinkSet));

	vTaskDelay(pdMS_TO_TICKS(2000)); // 等待2秒，讓用戶看到LED狀態
	xTaskCreate(TaskMode, "TaskMode", 2048, NULL, 1, &Handle_TaskMode);
}

/* 獲取指定的佇列句柄 */
QueueHandle_t getQueuedMode(QueueList get_queue)
{
	switch (get_queue)
	{
		case QUEUE_MODE:
			return Queue_Mode;
			break;

		case QUEUE_LED_BLINK:
			return Queue_LEDBlink;
			break;
		default:
			return nullptr;
			break;
	}
}