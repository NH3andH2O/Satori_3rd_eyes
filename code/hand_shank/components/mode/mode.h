#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

namespace Mode
{
	enum Status
	{
		IDLE,
		CALIBRATING,
	};

	enum QueueList
	{
		QUEUE_MODE,
		QUEUE_LED_BLINK,
	};

	struct BlinkSet
	{
		uint8_t times;	// LED閃爍次數
		uint16_t delay; // 閃爍時間間隔
	};

} // namespace Mode

void initMode();
QueueHandle_t getQueuedMode(Mode::QueueList queue);