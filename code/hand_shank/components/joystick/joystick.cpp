#include "joystick.h"

#include <Arduino.h>
#include "esp_log.h"
#include "board_config.h"
#include "board_prefs.h"
#include "mode.h"

using enum JoystickCalibrationConfig;

static const char *TAG = "Joystick"; // 日志標籤

/** 構造函數 **/
/* 成員函數 */
Joystick::Joystick(gpio_num_t pin_adc1, gpio_num_t pin_adc2)
{
	this->pin_adc1 = pin_adc1;
	this->pin_adc2 = pin_adc2;
}

/* 初始化搖桿 */
void Joystick::init()
{
	ESP_LOGI(TAG, "Joystick initialized on pins ADC1: %d, ADC2: %d", pin_adc1, pin_adc2);
	analogReadResolution(12);

	/* 設置ADC引腳衰減 */
	analogSetPinAttenuation(this->pin_adc1, ADC_11db);
	analogSetPinAttenuation(this->pin_adc2, ADC_11db);
}

/* 讀取搖桿ADC值 */
JoystickLoc Joystick::readJoystickADC()
{
	JoystickLoc adc;
	adc.x = analogRead(PIN_JOY_ADC1);
	adc.y = analogRead(PIN_JOY_ADC2);

	ESP_LOGD(TAG, "Joystick ADC Readings - X: %d, Y: %d", adc.x, adc.y);
	return adc;
}

/* 讀取搖桿位置 */
JoystickLoc Joystick::readJoystickPoc()
{
	JoystickLoc loc = readJoystickADC();
	JoystickPrefs prefs = getJoystickPrefs();

	/* 根據校準數據計算位置百分比 */
	int16_t delta_x = loc.x - prefs.center_x;
	int16_t delta_y = loc.y - prefs.center_y;

	/* 應用死區 */
	if (abs(delta_x) < prefs.deadzone)
		delta_x = 0;
	if (abs(delta_y) < prefs.deadzone)
		delta_y = 0;

	/* 計算百分比位置 */
	int16_t percent_x = 50;
	int16_t percent_y = 50;
	if (delta_x > 0)
		percent_x = 50 + (delta_x * 50) / (prefs.max_x - prefs.center_x);
	else if (delta_x < 0)
		percent_x = 50 + (delta_x * 50) / (prefs.center_x - prefs.min_x);
	if (delta_y > 0)
		percent_y = 50 + (delta_y * 50) / (prefs.max_y - prefs.center_y);
	else if (delta_y < 0)
		percent_y = 50 + (delta_y * 50) / (prefs.center_y - prefs.min_y);

	return {percent_x, percent_y};
}

/* 計算搖桿統計數據：連續讀取 N 次，計算平均值、方差和百分位數 */
JoystickStats Joystick::computeStats(uint8_t window, bool is_ADC)
{
	JoystickStats stats{};

	if (window < 2 || window > 100)
	{
		window = 10; // 如果窗口無效，使用預設值
		ESP_LOGW(TAG, "Invalid window size, using default: %d", window);
	}

	int16_t buf_x[100] = {0};
	int16_t buf_y[100] = {0};

	/* 連續讀取 window 次 */
	for (uint8_t i = 0; i < window; i++)
	{
		if (is_ADC)
		{
			JoystickLoc adc = readJoystickADC();
			buf_x[i] = adc.x;
			buf_y[i] = adc.y;
		}
		else
		{
			JoystickLoc loc = readJoystickPoc();
			buf_x[i] = loc.x;
			buf_y[i] = loc.y;
		}
		vTaskDelay(pdMS_TO_TICKS(2));
	}

	/* 計算平均值 */
	float sum_x = 0.0f;
	float sum_y = 0.0f;
	for (uint8_t i = 0; i < window; i++)
	{
		sum_x += buf_x[i];
		sum_y += buf_y[i];
	}

	stats.mean_x = sum_x / window;
	stats.mean_y = sum_y / window;

	/* 計算方差 */
	float var_x = 0.0f;
	float var_y = 0.0f;
	for (uint8_t i = 0; i < window; i++)
	{
		float dx = buf_x[i] - stats.mean_x;
		float dy = buf_y[i] - stats.mean_y;
		var_x += dx * dx;
		var_y += dy * dy;
	}

	stats.var_x = var_x / window;
	stats.var_y = var_y / window;

	/* 計算百分位數：對數據進行排序 */
	int16_t sorted_x[100];
	int16_t sorted_y[100];
	for (uint8_t i = 0; i < window; i++)
	{
		sorted_x[i] = buf_x[i];
		sorted_y[i] = buf_y[i];
	}

	/* 冒泡排序 X 軸 */
	for (uint8_t i = 0; i < window - 1; i++)
	{
		for (uint8_t j = 0; j < window - 1 - i; j++)
		{
			if (sorted_x[j] > sorted_x[j + 1])
			{
				int16_t temp = sorted_x[j];
				sorted_x[j] = sorted_x[j + 1];
				sorted_x[j + 1] = temp;
			}
		}
	}

	/* 冒泡排序 Y 軸 */
	for (uint8_t i = 0; i < window - 1; i++)
	{
		for (uint8_t j = 0; j < window - 1 - i; j++)
		{
			if (sorted_y[j] > sorted_y[j + 1])
			{
				int16_t temp = sorted_y[j];
				sorted_y[j] = sorted_y[j + 1];
				sorted_y[j + 1] = temp;
			}
		}
	}

	/* 計算第 5 和 95 百分位數 */
	uint8_t idx_5 = (uint8_t)((window - 1) * 0.05f);
	uint8_t idx_95 = (uint8_t)((window - 1) * 0.95f);

	stats.p5_x = sorted_x[idx_5];
	stats.p95_x = sorted_x[idx_95];
	stats.p5_y = sorted_y[idx_5];
	stats.p95_y = sorted_y[idx_95];

	stats.valid = true;

	ESP_LOGD(TAG, "Stats - Mean X: %.2f, Mean Y: %.2f, Var X: %.2f, Var Y: %.2f", stats.mean_x, stats.mean_y, stats.var_x, stats.var_y);
	ESP_LOGD(TAG, "Percentiles - P5 X: %.2f, P95 X: %.2f, P5 Y: %.2f, P95 Y: %.2f", stats.p5_x, stats.p95_x, stats.p5_y, stats.p95_y);

	return stats;
}

/* 搖桿校準 */
int8_t Joystick::JoystickCalibration()
{
	ESP_LOGI(TAG, "Starting joystick calibration...");

	JoystickStats stats;
	uint8_t success_count = 0;

	/* 校準參數 */
	uint16_t center_min_x = 0;
	uint16_t center_max_x = 0;
	uint16_t center_min_y = 0;
	uint16_t center_max_y = 0;
	uint16_t max_x = 0;
	uint16_t max_y = 0;
	uint16_t min_x = 0;
	uint16_t min_y = 0;

	QueueHandle_t Queue_LED_blink = getQueuedMode(Mode::QueueList::QUEUE_LED_BLINK);
	Mode::BlinkSet blink_set = {2, 75}; // 閃爍2次，每次75毫秒

	/* 中心值校準 */
	ESP_LOGI(TAG, "Calibrating center position. Please leave the joystick untouched.");
	while (success_count < 10)
	{
		stats = computeStats(64, true);
		if (stats.valid && stats.var_x < 40.0f && stats.var_y < 40.0f && stats.mean_x > (float)JOY_ADC_CENTER_X_MIN &&
			stats.mean_x < (float)JOY_ADC_CENTER_X_MAX && stats.mean_y > (float)JOY_ADC_CENTER_Y_MIN && stats.mean_y < (float)JOY_ADC_CENTER_Y_MAX)
		{
			success_count++;
			ESP_LOGV(TAG, "Center calibration step %d/10 successful.", success_count);
			vTaskDelay(pdMS_TO_TICKS(100)); // 等待一段時間再進行下一次測量
		}
		else
		{
			success_count = 0; // 重置計數器
			ESP_LOGV(TAG, "Center calibration unstable, retrying...");
		}
	}

	/* 記錄中心值範圍 */
	center_min_x = stats.p5_x;
	center_max_x = stats.p95_x;
	center_min_y = stats.p5_y;
	center_max_y = stats.p95_y;
	ESP_LOGI(TAG, "Center position calibrated: X[%d - %d], Y[%d - %d]", center_min_x, center_max_x, center_min_y, center_max_y);

	/* 發送提示 */
	xQueueOverwrite(Queue_LED_blink, &blink_set);

	/* 右邊校準 */
	success_count = 0;
	ESP_LOGI(TAG, "Calibrating right position. Please push the joystick to the right.");
	vTaskDelay(pdMS_TO_TICKS(400));
	while (success_count < 10)
	{
		stats = computeStats(64, true);
		if (stats.valid && stats.var_x < 200.0f && stats.var_y < 200.0f && stats.mean_x > (float)JOYADC_MAX_X &&
			stats.mean_y > (float)JOY_ADC_CENTER_Y_MIN && stats.mean_y < (float)JOY_ADC_CENTER_Y_MAX)
		{
			success_count++;
			ESP_LOGV(TAG, "Right calibration step %d/10 successful.", success_count);
			vTaskDelay(pdMS_TO_TICKS(100));
		}
		else
		{
			success_count = 0;
			ESP_LOGV(TAG, "Right calibration unstable, retrying...");
		}
	}

	/* 記錄最大值 */
	max_x = stats.p95_x;
	ESP_LOGI(TAG, "Right position calibrated: Max X = %d", max_x);

	/* 發送提示 */
	xQueueOverwrite(Queue_LED_blink, &blink_set);

	/* 回到中心位置校準 */
	success_count = 0;
	ESP_LOGI(TAG, "Return joystick to center position.");
	vTaskDelay(pdMS_TO_TICKS(400));
	while (success_count < 10)
	{
		stats = computeStats(64, true);
		if (stats.valid && stats.var_x < 40.0f && stats.var_y < 40.0f && stats.mean_x > (float)JOY_ADC_CENTER_X_MIN &&
			stats.mean_x < (float)JOY_ADC_CENTER_X_MAX && stats.mean_y > (float)JOY_ADC_CENTER_Y_MIN && stats.mean_y < (float)JOY_ADC_CENTER_Y_MAX)
		{
			success_count++;
			ESP_LOGV(TAG, "Return to center step %d/10 successful.", success_count);
			vTaskDelay(pdMS_TO_TICKS(100));
		}
		else
		{
			success_count = 0;
			ESP_LOGV(TAG, "Return to center unstable, retrying...");
		}
	}

	/* 更新數據 */
	center_min_x = center_min_x < stats.p5_x ? center_min_x : stats.p5_x;
	center_max_x = center_max_x > stats.p95_x ? center_max_x : stats.p95_x;
	center_min_y = center_min_y < stats.p5_y ? center_min_y : stats.p5_y;
	center_max_y = center_max_y > stats.p95_y ? center_max_y : stats.p95_y;
	ESP_LOGI(TAG, "Center position re-calibrated: X[%d - %d], Y[%d - %d]", center_min_x, center_max_x, center_min_y, center_max_y);

	/* 發送提示 */
	xQueueOverwrite(Queue_LED_blink, &blink_set);

	/* 上邊校準 */
	success_count = 0;
	ESP_LOGI(TAG, "Calibrating up position. Please push the joystick upwards.");
	vTaskDelay(pdMS_TO_TICKS(400));
	while (success_count < 10)
	{
		stats = computeStats(64, true);
		if (stats.valid && stats.var_x < 200.0f && stats.var_y < 200.0f && stats.mean_y < (float)JOYADC_MIN_Y &&
			stats.mean_x > (float)JOY_ADC_CENTER_X_MIN && stats.mean_x < (float)JOY_ADC_CENTER_X_MAX)
		{
			success_count++;
			ESP_LOGV(TAG, "Up calibration step %d/10 successful.", success_count);
			vTaskDelay(pdMS_TO_TICKS(100));
		}
		else
		{
			success_count = 0;
			ESP_LOGV(TAG, "Up calibration unstable, retrying...");
		}
	}

	/* 記錄最小值 */
	min_y = stats.p5_y;
	ESP_LOGI(TAG, "Up position calibrated: Min Y = %d", min_y);

	/* 發送提示 */
	xQueueOverwrite(Queue_LED_blink, &blink_set);

	/* 回到中心位置校準 */
	success_count = 0;
	ESP_LOGI(TAG, "Return joystick to center position.");
	vTaskDelay(pdMS_TO_TICKS(400));
	while (success_count < 10)
	{
		stats = computeStats(64, true);
		if (stats.valid && stats.var_x < 40.0f && stats.var_y < 40.0f && stats.mean_x > (float)JOY_ADC_CENTER_X_MIN &&
			stats.mean_x < (float)JOY_ADC_CENTER_X_MAX && stats.mean_y > (float)JOY_ADC_CENTER_Y_MIN && stats.mean_y < (float)JOY_ADC_CENTER_Y_MAX)
		{
			success_count++;
			ESP_LOGV(TAG, "Return to center step %d/10 successful.", success_count);
			vTaskDelay(pdMS_TO_TICKS(100));
		}
		else
		{
			success_count = 0;
			ESP_LOGV(TAG, "Return to center unstable, retrying...");
		}
	}

	/* 更新數據 */
	center_min_x = center_min_x < stats.p5_x ? center_min_x : stats.p5_x;
	center_max_x = center_max_x > stats.p95_x ? center_max_x : stats.p95_x;
	center_min_y = center_min_y < stats.p5_y ? center_min_y : stats.p5_y;
	center_max_y = center_max_y > stats.p95_y ? center_max_y : stats.p95_y;
	ESP_LOGI(TAG, "Center position re-calibrated: X[%d - %d], Y[%d - %d]", center_min_x, center_max_x, center_min_y, center_max_y);

	/* 發送提示 */
	xQueueOverwrite(Queue_LED_blink, &blink_set);

	/* 左邊校準 */
	success_count = 0;
	ESP_LOGI(TAG, "Calibrating left position. Please push the joystick to the left.");
	vTaskDelay(pdMS_TO_TICKS(400));
	while (success_count < 10)
	{
		stats = computeStats(64, true);
		if (stats.valid && stats.var_x < 200.0f && stats.var_y < 200.0f && stats.mean_x < (float)JOYADC_MIN_X &&
			stats.mean_y > (float)JOY_ADC_CENTER_Y_MIN && stats.mean_y < (float)JOY_ADC_CENTER_Y_MAX)
		{
			success_count++;
			ESP_LOGV(TAG, "Left calibration step %d/10 successful.", success_count);
			vTaskDelay(pdMS_TO_TICKS(100));
		}
		else
		{
			success_count = 0;
			ESP_LOGV(TAG, "Left calibration unstable, retrying...");
		}
	}

	/* 記錄最小值 */
	min_x = stats.p5_x;
	ESP_LOGI(TAG, "Left position calibrated: Min X = %d", min_x);

	/* 發送提示 */
	xQueueOverwrite(Queue_LED_blink, &blink_set);

	/* 回到中心位置校準 */
	success_count = 0;
	ESP_LOGI(TAG, "Return joystick to center position.");
	vTaskDelay(pdMS_TO_TICKS(400));
	while (success_count < 10)
	{
		stats = computeStats(64, true);
		if (stats.valid && stats.var_x < 40.0f && stats.var_y < 40.0f && stats.mean_x > (float)JOY_ADC_CENTER_X_MIN &&
			stats.mean_x < (float)JOY_ADC_CENTER_X_MAX && stats.mean_y > (float)JOY_ADC_CENTER_Y_MIN && stats.mean_y < (float)JOY_ADC_CENTER_Y_MAX)
		{
			success_count++;
			ESP_LOGV(TAG, "Return to center step %d/10 successful.", success_count);
			vTaskDelay(pdMS_TO_TICKS(100));
		}
		else
		{
			success_count = 0;
			ESP_LOGV(TAG, "Return to center unstable, retrying...");
		}
	}

	/* 更新數據 */
	center_min_x = center_min_x < stats.p5_x ? center_min_x : stats.p5_x;
	center_max_x = center_max_x > stats.p95_x ? center_max_x : stats.p95_x;
	center_min_y = center_min_y < stats.p5_y ? center_min_y : stats.p5_y;
	center_max_y = center_max_y > stats.p95_y ? center_max_y : stats.p95_y;
	ESP_LOGI(TAG, "Center position re-calibrated: X[%d - %d], Y[%d - %d]", center_min_x, center_max_x, center_min_y, center_max_y);

	/* 發送提示 */
	xQueueOverwrite(Queue_LED_blink, &blink_set);

	/* 下邊校準 */
	success_count = 0;
	ESP_LOGI(TAG, "Calibrating down position. Please push the joystick downwards.");
	vTaskDelay(pdMS_TO_TICKS(400));
	while (success_count < 10)
	{
		stats = computeStats(64, true);
		if (stats.valid && stats.var_x < 200.0f && stats.var_y < 200.0f && stats.mean_y > (float)JOYADC_MAX_Y &&
			stats.mean_x > (float)JOY_ADC_CENTER_X_MIN && stats.mean_x < (float)JOY_ADC_CENTER_X_MAX)
		{
			success_count++;
			ESP_LOGV(TAG, "Down calibration step %d/10 successful.", success_count);
			vTaskDelay(pdMS_TO_TICKS(100));
		}
		else
		{
			success_count = 0;
			ESP_LOGV(TAG, "Down calibration unstable, retrying...");
		}
	}

	/* 記錄最大值 */
	max_y = stats.p95_y;
	ESP_LOGI(TAG, "Down position calibrated: Max Y = %d", max_y);

	/* 發送提示 */
	xQueueOverwrite(Queue_LED_blink, &blink_set);

	/* 回到中心位置校準 */
	success_count = 0;
	ESP_LOGI(TAG, "Return joystick to center position.");
	vTaskDelay(pdMS_TO_TICKS(400));
	while (success_count < 10)
	{
		stats = computeStats(64, true);
		if (stats.valid && stats.var_x < 40.0f && stats.var_y < 40.0f && stats.mean_x > (float)JOY_ADC_CENTER_X_MIN &&
			stats.mean_x < (float)JOY_ADC_CENTER_X_MAX && stats.mean_y > (float)JOY_ADC_CENTER_Y_MIN && stats.mean_y < (float)JOY_ADC_CENTER_Y_MAX)
		{
			success_count++;
			ESP_LOGV(TAG, "Return to center step %d/10 successful.", success_count);
			vTaskDelay(pdMS_TO_TICKS(100));
		}
		else
		{
			success_count = 0;
			ESP_LOGV(TAG, "Return to center unstable, retrying...");
		}
	}

	/* 更新數據 */
	center_min_x = center_min_x < stats.p5_x ? center_min_x : stats.p5_x;
	center_max_x = center_max_x > stats.p95_x ? center_max_x : stats.p95_x;
	center_min_y = center_min_y < stats.p5_y ? center_min_y : stats.p5_y;
	center_max_y = center_max_y > stats.p95_y ? center_max_y : stats.p95_y;
	ESP_LOGI(TAG, "Center position re-calibrated: X[%d - %d], Y[%d - %d]", center_min_x, center_max_x, center_min_y, center_max_y);

	/* 發送提示 */
	xQueueOverwrite(Queue_LED_blink, &blink_set);

	/* 儲存校準數據 */
	JoystickPrefs prefs;
	prefs.valid = true;
	prefs.max_x = max_x;
	prefs.min_x = min_x;
	prefs.max_y = max_y;
	prefs.min_y = min_y;
	prefs.center_x = (center_min_x + center_max_x) / 2;
	prefs.center_y = (center_min_y + center_max_y) / 2;
	prefs.deadzone = center_max_x - center_min_x > center_max_y - center_min_y ? center_max_x - center_min_x + 20 : center_max_y - center_min_y + 20;
	setJoystickPrefs(prefs);
	ESP_LOGI(TAG, "Joystick calibration completed and preferences saved.");
	blink_set = {1, 1000};
	xQueueOverwrite(Queue_LED_blink, &blink_set);
	vTaskDelay(pdMS_TO_TICKS(2500));
	return 0;
}