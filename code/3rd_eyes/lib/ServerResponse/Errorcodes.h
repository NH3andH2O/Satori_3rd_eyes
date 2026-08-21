#pragma once

namespace ServerError
{
	enum Code
	{
		ERR_OK = 0,

		/* 通用錯誤 */
		ERR_INVALID_JSON = 10001, // 無效的JSON格式

		/* WiFi錯誤 */
		ERR_WIFI_SSID_EMPTY = 10101, // 啓用WiFi時SSID不能為空

		/* SoftAP錯誤 */
		ERR_SOFTAP_PASSWORD_MISMATCH = 10201, // SoftAP密碼不匹配
		ERR_SOFTAP_INVALID_PASSWORD = 10202,  // 無效的SoftAP密碼

		/* 模式錯誤 */
		ERR_MODE_INVALID = 10301, // 無效的模式

		/* 舵機錯誤 */
		ERR_SERVO_CONFIG_MISSING = 10401, // 缺少舵機配置欄位
		ERR_SERVO_CONFIG_INVALID = 10402, // 無效的舵機配置
	};
}