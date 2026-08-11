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
	config.max_upper_eyelid_pin = (uint8_t)(this->prefs.getUShort("max_upper_eyelid_pin", 150));
	config.mid_upper_eyelid_pin = (uint8_t)(this->prefs.getUShort("mid_upper_eyelid_pin", 85));
	config.min_upper_eyelid_pin = (uint8_t)(this->prefs.getUShort("min_upper_eyelid_pin", 50));
	config.max_lower_eyelid_pin = (uint8_t)(this->prefs.getUShort("max_lower_eyelid_pin", 65));
	config.mid_lower_eyelid_pin = (uint8_t)(this->prefs.getUShort("mid_lower_eyelid_pin", 50));
	config.min_lower_eyelid_pin = (uint8_t)(this->prefs.getUShort("min_lower_eyelid_pin", 22));
	config.max_eyeball_pin = (uint8_t)(this->prefs.getUShort("max_eyeball_pin", 110));
	config.mid_eyeball_pin = (uint8_t)(this->prefs.getUShort("mid_eyeball_pin", 75));
	config.min_eyeball_pin = (uint8_t)(this->prefs.getUShort("min_eyeball_pin", 40));
	this->prefs.end();
	return config;
}

void Config::setServoConfig(const ServoConfig &config)
{
	this->prefs.begin("Servo", false);
	this->prefs.putUShort("max_upper_eyelid_pin", (uint16_t)(config.max_upper_eyelid_pin));
	this->prefs.putUShort("mid_upper_eyelid_pin", (uint16_t)(config.mid_upper_eyelid_pin));
	this->prefs.putUShort("min_upper_eyelid_pin", (uint16_t)(config.min_upper_eyelid_pin));
	this->prefs.putUShort("max_lower_eyelid_pin", (uint16_t)(config.max_lower_eyelid_pin));
	this->prefs.putUShort("mid_lower_eyelid_pin", (uint16_t)(config.mid_lower_eyelid_pin));
	this->prefs.putUShort("min_lower_eyelid_pin", (uint16_t)(config.min_lower_eyelid_pin));
	this->prefs.putUShort("max_eyeball_pin", (uint16_t)(config.max_eyeball_pin));
	this->prefs.putUShort("mid_eyeball_pin", (uint16_t)(config.mid_eyeball_pin));
	this->prefs.putUShort("min_eyeball_pin", (uint16_t)(config.min_eyeball_pin));
	this->prefs.end();
}