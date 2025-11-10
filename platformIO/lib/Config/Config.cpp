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
	config.mode = (uint8_t)(this->prefs.getUShort("mode", 1));
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