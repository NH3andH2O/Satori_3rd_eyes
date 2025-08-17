#include "serverResponse.h"

void onSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
	JsonDocument doc_get;
	switch (type)
	{
		case WS_EVT_CONNECT:
			Serial.printf("WebSocket client connected: %u\n", client->id());
			break;
		case WS_EVT_DISCONNECT:
			Serial.printf("WebSocket client disconnected: %u\n", client->id());
			break;
		case WS_EVT_DATA:
		{
			Serial.printf("WebSocket data received from client %u: %.*s\n", client->id(), len, data);
			break;
		}
		default:
			break;
	}
}

/* get wifi設置獲取 */
void api_wifi_config(AsyncWebServerRequest *request)
{
	/* 檢查請求方法 */
	if (request->method() == HTTP_GET)
	{
		JsonDocument data_doc;
		data_doc["is_wifi"] = prefs.getBool("iswifi", false);
		data_doc["ssid"] = prefs.getString("wifi_ssid", "");
		data_doc["password"] = prefs.getString("wifi_password", "");

		String jsonStr;
		serializeJson(data_doc, jsonStr);

		request->send(200, "application/json", jsonStr);
	}
	else
	{
		request->send(405, "text/plain", "Method Not Allowed");
	}
}

/* post wifi資料更改 */
void api_set_wifi_config(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{

	/* 檢查請求方法 */
	if (request->method() == HTTP_POST)
	{
		JsonDocument data_doc;

		/* 獲取JSON */
		DeserializationError error = deserializeJson(data_doc, data);
		if (error)
		{
			Serial.printf("Failed to parse JSON: %s\n", error.c_str());
			request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
			return;
		}

		/* 獲取WiFi配置 */
		const char *ssid = data_doc["ssid"] | "";
		const char *password = data_doc["password"] | "";
		uint8_t iswifi = data_doc["is_wifi"] | false;

		/* 檢查WiFi配置 */
		if (iswifi == false || (iswifi == true && ssid[0] != '\0'))
		{
			/* 存儲WiFi配置 */
			prefs.putString("wifi_ssid", ssid);
			prefs.putString("wifi_password", password);
			prefs.putBool("iswifi", iswifi);

			/* 發送返回值 */
			request->send(200, "application/json", "{\"success\":true}");

			/* wifi更新 */
			uint8_t is_wifiUpdate = 1;							  // WiFi更新標誌
			xQueueSend(wifiUpdate_data_quene, &is_wifiUpdate, 0); // 發送WiFi更新信號
			return;
		}
		else
		{
			/* 發送錯誤返回值 */
			request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
			return;
		}
	}
}

/* get softAP設置獲取 */
void api_softAP_config(AsyncWebServerRequest *request)
{
	/* 檢查請求方法 */
	if (request->method() == HTTP_GET)
	{
		JsonDocument data_doc;
		data_doc["ssid"] = prefs.getString("ssid", DEFAULT_SSID);
		data_doc.shrinkToFit();

		String jsonStr;
		serializeJson(data_doc, jsonStr);

		request->send(200, "application/json", jsonStr);
	}
	else
	{
		request->send(405, "text/plain", "Method Not Allowed");
	}
}

/* post softAP資料更改 */
void api_set_softAP_config(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
	/* 檢查請求方法 */
	if (request->method() == HTTP_POST)
	{
		JsonDocument data_json;

		/* 獲取JSON */
		DeserializationError error = deserializeJson(data_json, data);
		if (error)
		{
			Serial.printf("Failed to parse JSON: %s\n", error.c_str());
			request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
			return;
		}

		/* 獲取SoftAP配置 */
		const char *ssid = data_json["ssid"] | DEFAULT_SSID;
		const uint8_t is_change_password = data_json["is_change_password"] | false;
		const char *password = data_json["password"] | "";
		const char *password_confirm = data_json["password_confirm"] | "";

		/* 檢查SoftAP配置 */
		if (is_change_password && strcmp(password, password_confirm) != 0)
		{
			request->send(400, "application/json", "{\"error\":\"Passwords do not match\"}");
			return;
		}
		else if (is_change_password && !is_valid_wifi_password(password))
		{
			request->send(400, "application/json", "{\"error\":\"Invalid password\"}");
			return;
		}

		/* 存儲SoftAP配置 */
		if (ssid[0] == '\0')
		{
			prefs.putString("ssid", DEFAULT_SSID);
		}
		else
		{
			prefs.putString("ssid", ssid);
		}
		if (is_change_password)
		{
			if (strlen(password) > 0)
			{
				prefs.putString("password", password);
			}
			else
			{
				prefs.remove("password"); // 如果密碼為空，則刪除密碼
			}
		}
		request->send(200, "application/json", "{\"success\":true}");

		/* 發送SoftAP配置更新信號 */
		uint8_t is_wifiUpdate = 2; // WiFi更新標誌
		xQueueSend(wifiUpdate_data_quene, &is_wifiUpdate, 0);
		return;
	}
	else
	{
		request->send(405, "text/plain", "Method Not Allowed");
	}
}

/* get模式獲取 */
void api_mode_config(AsyncWebServerRequest *request)
{
	/* 檢查請求方法 */
	if (request->method() == HTTP_GET)
	{
		JsonDocument data_doc;
		data_doc["mode"] = prefs.getInt("mode", 1);
		data_doc["correction_timer"] = prefs.getUInt("correction", 2000);

		String jsonStr;
		serializeJson(data_doc, jsonStr);

		request->send(200, "application/json", jsonStr);
	}
	else
	{
		request->send(405, "text/plain", "Method Not Allowed");
	}
}

/* post模式修改 */
void api_set_mode_config(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
	/* 檢查請求方法 */
	if (request->method() == HTTP_POST)
	{
		JsonDocument data_json;

		/* 獲取JSON */
		DeserializationError error = deserializeJson(data_json, data);
		if (error)
		{
			Serial.printf("Failed to parse JSON: %s\n", error.c_str());
			request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
			return;
		}

		/* 獲取模式配置 */
		int8_t mode = data_json["mode"] | 0;
		if (mode < 1 || mode > 1) // 非法模式檢測
		{
			request->send(400, "application/json", "{\"error\":\"Invalid mode\"}");
			return;
		}

		uint16_t correction_timer = data_json["correction_timer"] | 2000;

		prefs.putInt("mode", mode);
		prefs.putUInt("correction", correction_timer);

		request->send(200, "application/json", "{\"success\":true}");
		return;
	}
	else
	{
		request->send(405, "text/plain", "Method Not Allowed");
	}
}