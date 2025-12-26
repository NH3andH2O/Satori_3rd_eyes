#include "board_prefs.h"

#include <Arduino.h>
#include <Preferences.h>

static Preferences prefs;

void setJoystickPrefs(const JoystickPrefs &configs)
{
	prefs.begin("joystick", false);
	prefs.putInt("center_x", configs.center_x);
	prefs.putInt("center_y", configs.center_y);
	prefs.putInt("min_x", configs.min_x);
	prefs.putInt("max_x", configs.max_x);
	prefs.putInt("min_y", configs.min_y);
	prefs.putInt("max_y", configs.max_y);
	prefs.putInt("deadzone", configs.deadzone);

	/* 標記為有效配置 */
	prefs.putBool("valid", true);
	prefs.end();
}

JoystickPrefs getJoystickPrefs()
{
	JoystickPrefs configs;
	prefs.begin("joystick", true);
	configs.valid = prefs.getBool("valid", false);

	/* 檢查是否有數據 */
	if (!configs.valid)
	{
		prefs.end();
		return configs;
	}
	configs.center_x = prefs.getInt("center_x", 0);
	configs.center_y = prefs.getInt("center_y", 0);
	configs.min_x = prefs.getInt("min_x", 0);
	configs.max_x = prefs.getInt("max_x", 0);
	configs.min_y = prefs.getInt("min_y", 0);
	configs.max_y = prefs.getInt("max_y", 0);
	configs.deadzone = prefs.getInt("deadzone", 0);
	prefs.end();
	return configs;
}