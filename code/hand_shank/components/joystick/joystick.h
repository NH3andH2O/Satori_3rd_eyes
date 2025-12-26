#pragma once

#include <stdint.h>
#include "driver/gpio.h"

enum class JoystickCalibrationConfig : uint16_t
{
	JOY_ADC_CENTER_X_MIN = 1200,
	JOY_ADC_CENTER_X_MAX = 2000,
	JOY_ADC_CENTER_Y_MIN = 1200,
	JOY_ADC_CENTER_Y_MAX = 2000,
	JOYADC_MAX_X = 2900,
	JOYADC_MAX_Y = 2900,
	JOYADC_MIN_X = 400,
	JOYADC_MIN_Y = 400,
};

struct JoystickLoc
{
	int16_t x; // X軸位置
	int16_t y; // Y軸位置
};

struct JoystickStats
{
	bool valid;		// 資料是否有效
	float mean_x;	// X軸平均值
	float mean_y;	// Y軸平均值
	float var_x;	// X軸方差
	float var_y;	// Y軸方差
	uint16_t p5_x;	// X軸第5百分位數
	uint16_t p95_x; // X軸第95百分位數
	uint16_t p5_y;	// Y軸第5百分位數
	uint16_t p95_y; // Y軸第95百分位數
};

class Joystick
{
  private:
	gpio_num_t pin_adc1;
	gpio_num_t pin_adc2;
	JoystickLoc readJoystickADC();

  public:
	Joystick(gpio_num_t pin_adc1, gpio_num_t pin_adc2);
	void init();
	JoystickLoc readJoystickPoc();
	JoystickStats computeStats(uint8_t window = 10, bool is_ADC = false);
	int8_t JoystickCalibration();
};