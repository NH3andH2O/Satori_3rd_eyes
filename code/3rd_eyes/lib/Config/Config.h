#pragma once

#include <Arduino.h>
#include <Preferences.h>

namespace AppConfig
{
	struct WiFiConfig
	{
		bool is_enabled;
		String ssid;
		String password;
	};

	struct SoftAPConfig
	{
		String ssid;
		String password;
	};

	struct ModeConfig
	{
		uint8_t mode;
	};

	struct ServoConfig
	{
		uint8_t max_upper_eyelid_pin;
		uint8_t mid_upper_eyelid_pin;
		uint8_t min_upper_eyelid_pin;
		uint8_t max_lower_eyelid_pin;
		uint8_t mid_lower_eyelid_pin;
		uint8_t min_lower_eyelid_pin;
		uint8_t max_eyeball_pin;
		uint8_t mid_eyeball_pin;
		uint8_t min_eyeball_pin;
	};

	struct AdvancedConfig
	{
		uint16_t correction_timer;
	};

	class Config
	{
	  private:
		Preferences prefs; // Preferences實例

	  public:
		WiFiConfig getWiFiConfig();
		void setWiFiConfig(const WiFiConfig &config);

		SoftAPConfig getSoftAPConfig();
		void setSoftAPConfig(const SoftAPConfig &config);

		ModeConfig getModeConfig();
		void setModeConfig(const ModeConfig &config);

		AdvancedConfig getAdvancedConfig();
		void setAdvancedConfig(const AdvancedConfig &config);

		ServoConfig getServoConfig();
		void setServoConfig(const ServoConfig &config);
	};
} // namespace AppConfig