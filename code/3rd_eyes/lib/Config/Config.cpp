#include "Config.h"
using namespace AppConfig;

WiFiConfig Config::getWiFiConfig()
{
	WiFiConfig config;
	this->prefs.begin("WiFi", true);
	config.is_enabled = this->prefs.getBool("wifi_enabled", false);
	config.ssid = this->prefs.getString("wifi_ssid", "");
	config.password = this->prefs.getString("wifi_password", "");
	this->prefs.end();
	return config;
}

void Config::setWiFiConfig(const WiFiConfig &config)
{
	this->prefs.begin("WiFi", false);
	this->prefs.putBool("wifi_enabled", config.is_enabled);
	this->prefs.putString("wifi_ssid", config.ssid);
	this->prefs.putString("wifi_password", config.password);
	this->prefs.end();
}

SoftAPConfig Config::getSoftAPConfig()
{
	SoftAPConfig config;
	this->prefs.begin("SoftAP", true);
	config.ssid = this->prefs.getString("softap_ssid", "3rdEyes");
	config.password = this->prefs.getString("softap_password", "");
	this->prefs.end();
	return config;
}

void Config::setSoftAPConfig(const SoftAPConfig &config)
{
	this->prefs.begin("SoftAP", false);
	this->prefs.putString("softap_ssid", config.ssid);
	this->prefs.putString("softap_password", config.password);
	this->prefs.end();
}

ModeConfig Config::getModeConfig()
{
	ModeConfig config;
	this->prefs.begin("Mode", true);
	config.mode = (uint8_t)(this->prefs.getUShort("mode", 0));
	this->prefs.end();
	return config;
}

void Config::setModeConfig(const ModeConfig &config)
{
	this->prefs.begin("Mode", false);
	this->prefs.putUShort("mode", (uint16_t)(config.mode));
	this->prefs.end();
}

AdvancedConfig Config::getAdvancedConfig()
{
	AdvancedConfig config;
	this->prefs.begin("Advanced", true);
	config.correction_timer = this->prefs.getUShort("correction", 2000);
	this->prefs.end();
	return config;
}

void Config::setAdvancedConfig(const AdvancedConfig &config)
{
	this->prefs.begin("Advanced", false);
	this->prefs.putUShort("correction", config.correction_timer);
	this->prefs.end();
}

ServoConfig Config::getServoConfig()
{
	ServoConfig config;
	this->prefs.begin("Servo", true);
	config.is_setup = this->prefs.getBool("is_setup", false);
	config.max_upper_eyelid = (uint8_t)(this->prefs.getUShort("upper_max", 150));
	config.mid_upper_eyelid = (uint8_t)(this->prefs.getUShort("upper_mid", 85));
	config.min_upper_eyelid = (uint8_t)(this->prefs.getUShort("upper_min", 50));
	config.max_lower_eyelid = (uint8_t)(this->prefs.getUShort("lower_max", 65));
	config.mid_lower_eyelid = (uint8_t)(this->prefs.getUShort("lower_mid", 50));
	config.min_lower_eyelid = (uint8_t)(this->prefs.getUShort("lower_min", 22));
	config.max_eyeball = (uint8_t)(this->prefs.getUShort("max_eyeball", 110));
	config.mid_eyeball = (uint8_t)(this->prefs.getUShort("mid_eyeball", 75));
	config.min_eyeball = (uint8_t)(this->prefs.getUShort("min_eyeball", 40));
	this->prefs.end();
	return config;
}

void Config::setServoConfig(const ServoConfig &config)
{
	this->prefs.begin("Servo", false);
	this->prefs.putBool("is_setup", config.is_setup);
	this->prefs.putUShort("upper_max", (uint16_t)(config.max_upper_eyelid));
	this->prefs.putUShort("upper_mid", (uint16_t)(config.mid_upper_eyelid));
	this->prefs.putUShort("upper_min", (uint16_t)(config.min_upper_eyelid));
	this->prefs.putUShort("lower_max", (uint16_t)(config.max_lower_eyelid));
	this->prefs.putUShort("lower_mid", (uint16_t)(config.mid_lower_eyelid));
	this->prefs.putUShort("lower_min", (uint16_t)(config.min_lower_eyelid));
	this->prefs.putUShort("max_eyeball", (uint16_t)(config.max_eyeball));
	this->prefs.putUShort("mid_eyeball", (uint16_t)(config.mid_eyeball));
	this->prefs.putUShort("min_eyeball", (uint16_t)(config.min_eyeball));
	this->prefs.end();
}

void Config::clearServoConfig()
{
	this->prefs.begin("Servo", false);
	this->prefs.clear();
	this->prefs.end();
}

uint8_t Config::getLogConfig()
{
	this->prefs.begin("Log", true);
	uint8_t log_level = (uint8_t)(this->prefs.getUShort("log_level", 3));
	this->prefs.end();
	return log_level;
}

void Config::setLogConfig(uint8_t log_level)
{
	this->prefs.begin("Log", false);
	this->prefs.putUShort("log_level", (uint16_t)(log_level));
	this->prefs.end();
}
