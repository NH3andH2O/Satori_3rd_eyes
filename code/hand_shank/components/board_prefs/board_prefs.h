#pragma once

#include <stdint.h>

/* 搖桿偏好設置 */
struct JoystickPrefs
{
	bool valid;		  // 配置是否有效
	int16_t center_x; // X軸中心值
	int16_t center_y; // Y軸中心值
	int16_t min_x;	  // X軸最小值
	int16_t max_x;	  // X軸最大值
	int16_t min_y;	  // Y軸最小值
	int16_t max_y;	  // Y軸最大值
	int16_t deadzone; // 死區範圍
};

void setJoystickPrefs(const JoystickPrefs &configs);
JoystickPrefs getJoystickPrefs();