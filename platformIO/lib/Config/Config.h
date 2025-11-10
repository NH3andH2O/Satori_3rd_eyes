#pragma once

#include <Arduino.h>
#include <Preferences.h>

namespace AppConfig
{
	struct WiFiConfig
	{
		uint8_t is_enabled;
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
	};
}