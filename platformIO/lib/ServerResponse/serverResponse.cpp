#include "serverResponse.h"

String getContentType(const String &path)
{
	if (path.endsWith(".html"))
		return "text/html";
	if (path.endsWith(".css"))
		return "text/css";
	if (path.endsWith(".js"))
		return "application/javascript";
	if (path.endsWith(".mjs"))
		return "application/javascript";
	if (path.endsWith(".json"))
		return "application/json";
	if (path.endsWith(".map"))
		return "application/json";
	if (path.endsWith(".wasm"))
		return "application/wasm";
	if (path.endsWith(".png"))
		return "image/png";
	if (path.endsWith(".jpg") || path.endsWith(".jpeg"))
		return "image/jpeg";
	if (path.endsWith(".gif"))
		return "image/gif";
	if (path.endsWith(".svg"))
		return "image/svg+xml";
	if (path.endsWith(".ico"))
		return "image/x-icon";
	if (path.endsWith(".webp"))
		return "image/webp";
	if (path.endsWith(".woff"))
		return "font/woff";
	if (path.endsWith(".woff2"))
		return "font/woff2";
	if (path.endsWith(".ttf"))
		return "font/ttf";
	if (path.endsWith(".txt"))
		return "text/plain";
	return "application/octet-stream";
}

void handleRoot(AsyncWebServerRequest *req)
{
	AsyncWebServerResponse *res = req->beginResponse(LittleFS, "/www/index.html", "text/html");
	res->addHeader("Cache-Control", "no-cache");
	req->send(res);
}

void handleNotFound(AsyncWebServerRequest *request)
{
	const String urlPath = request->url();

	String fsPath = "/www" + urlPath;

	if (fsPath.endsWith("/"))
	{
		fsPath += "index.html";
	}

	if (LittleFS.exists(fsPath))
	{
		request->send(LittleFS, fsPath, getContentType(fsPath));
		return;
	}

	AsyncWebServerResponse *res = request->beginResponse(LittleFS, "/www/index.html", "text/html");
	res->addHeader("Cache-Control", "no-cache");
	request->send(res);
}

void onSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
	switch (type)
	{
		case WS_EVT_CONNECT:
			ESP_LOGI("server", "WebSocket client connected: %u", client->id());
			break;
		case WS_EVT_DISCONNECT:
			ESP_LOGI("server", "WebSocket client disconnected: %u", client->id());
			break;
		case WS_EVT_DATA:
		{
			ESP_LOGD("server", "WebSocket data received from client %u: %.*s", client->id(), len, data);
			JsonDocument data_doc;

			/* 解析JSON */
			DeserializationError error = deserializeJson(data_doc, data, len);
			if (error)
			{
				ESP_LOGE("server", "Failed to parse JSON: %s\n", error.c_str());
				return;
			}

			/**  處理不同數據消息 **/
			String type = data_doc["type"] | "";

			/* joystick數據 */
			if (type.startsWith("joystick"))
			{
				network_control_data data_send;
				String L2_type = type.substring(9);

				/* 眼球移動數據 */
				if (L2_type.startsWith("move"))
				{
					String L3_type = L2_type.substring(5);

					/* 移動 */
					if (L3_type.startsWith("move"))
					{
						data_send.type = 0;
						data_send.x = data_doc["payload"]["vx"] | 0;
						data_send.y = data_doc["payload"]["vy"] | 0;
						xQueueSend(network_control_data_quene, &data_send, 0);
					}

					/* 停止 */
					else if (L3_type.startsWith("end"))
					{
						data_send.type = 0;
						data_send.x = 0;
						data_send.y = 0;
						xQueueSend(network_control_data_quene, &data_send, 0);
					}
				}

				/* 眼皮移動數據 */
				else if (L2_type.startsWith("eyelid"))
				{
					String L3_type = L2_type.substring(7);

					/* 移動 */
					if (L3_type.startsWith("move"))
					{
						data_send.type = 1;
						data_send.x = 0;
						data_send.y = data_doc["payload"]["vy"] | 0;
						xQueueSend(network_control_data_quene, &data_send, 0);
					}

					/* 停止 */
					else if (L3_type.startsWith("end"))
					{
						data_send.type = 1;
						data_send.x = 0;
						data_send.y = 0;
						xQueueSend(network_control_data_quene, &data_send, 0);
					}
				}
			}
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
	if (request->method() != HTTP_GET)
	{
		request->send(405, "text/plain", "Method Not Allowed");
		return;
	}
	/* 構建JSON */
	JsonDocument doc;
	doc["is_wifi"] = prefs.getBool("iswifi", false);
	doc["ssid"] = prefs.getString("wifi_ssid", "");
	doc["password"] = prefs.getString("wifi_password", "");

	/* 發送響應 */
	sendJsonResponse(request, 200, true, ServerError::ERR_OK, "", &doc);
}

/* post wifi資料更改 */
void api_set_wifi_config(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
	/* 檢查請求方法 */
	if (request->method() != HTTP_POST)
	{
		request->send(405, "text/plain", "Method Not Allowed");
		return;
	}

	/* 解析JSON */
	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, data);
	if (error)
	{
		ESP_LOGE("server", "Failed to parse JSON: %s", error.c_str());
		sendJsonResponse(request, 400, false, ServerError::ERR_INVALID_JSON, "Invalid JSON format");
		return;
	}

	/* 獲取WiFi配置 */
	const char *ssid = doc["ssid"] | "";
	const char *password = doc["password"] | "";
	uint8_t is_wifi = doc["is_wifi"] | false;

	/* 檢查WiFi配置有效性 */
	if (is_wifi && ssid[0] == '\0')
	{
		ESP_LOGE("server", "WiFi enabled but SSID is empty");
		sendJsonResponse(request, 400, false, ServerError::ERR_WIFI_SSID_EMPTY, "SSID cannot be empty when WiFi is enabled");
		return;
	}

	/* 存儲WiFi配置 */
	prefs.putString("wifi_ssid", ssid);
	prefs.putString("wifi_password", password);
	prefs.putBool("iswifi", is_wifi);

	ESP_LOGI("server", "WiFi config updated: SSID=%s, Enabled=%d", ssid, is_wifi);

	/* 發送成功響應 */
	sendJsonResponse(request, 200, true, ServerError::ERR_OK);

	/* 發送WiFi更新信號 */
	uint8_t wifi_update_flag = 1;
	xQueueSend(wifiUpdate_data_quene, &wifi_update_flag, 0);
}

/* get softAP設置獲取 */
void api_softAP_config(AsyncWebServerRequest *request)
{
	/* 檢查請求方法 */
	if (request->method() != HTTP_GET)
	{
		request->send(405, "text/plain", "Method Not Allowed");
	}

	/* 構建JSON */
	JsonDocument doc;
	doc["ssid"] = prefs.getString("ssid", DEFAULT_SSID);
	doc["password"] = prefs.getString("password", "");

	/* 發送響應 */
	sendJsonResponse(request, 200, true, ServerError::ERR_OK, "", &doc);
}

/* post softAP資料更改 */
void api_set_softAP_config(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
	/* 檢查請求方法 */
	if (request->method() != HTTP_POST)
	{
		request->send(405, "text/plain", "Method Not Allowed");
		return;
	}

	/* 解析JSON */
	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, data);
	if (error)
	{
		ESP_LOGE("server", "Failed to parse JSON: %s\n", error.c_str());
		sendJsonResponse(request, 400, false, ServerError::ERR_INVALID_JSON, "Invalid JSON format");
		return;
	}

	/* 獲取SoftAP配置 */
	const char *ssid = doc["ssid"] | DEFAULT_SSID;
	const uint8_t is_change_password = doc["is_change_password"] | false;
	const char *password = doc["password"] | "";
	const char *password_confirm = doc["password_confirm"] | "";

	/* 檢查SoftAP配置 */
	if (is_change_password && strcmp(password, password_confirm) != 0)
	{
		sendJsonResponse(request, 400, false, ServerError::ERR_SOFTAP_PASSWORD_MISMATCH, "Passwords do not match");
		return;
	}
	else if (is_change_password && !is_valid_wifi_password(password))
	{
		sendJsonResponse(request, 400, false, ServerError::ERR_SOFTAP_INVALID_PASSWORD, "Invalid password");
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

	/* 發送成功響應 */
	sendJsonResponse(request, 200, true, ServerError::ERR_OK);

	/* 發送SoftAP配置更新信號 */
	uint8_t is_wifiUpdate = 2; // WiFi更新標誌
	xQueueSend(wifiUpdate_data_quene, &is_wifiUpdate, 0);
	return;
}

/* get模式獲取 */
void api_mode_config(AsyncWebServerRequest *request)
{
	/* 檢查請求方法 */
	if (request->method() != HTTP_GET)
	{
		request->send(405, "text/plain", "Method Not Allowed");
	}

	/* 構建JSON */
	JsonDocument doc;
	doc["mode"] = prefs.getInt("mode", 1);

	/* 發送響應 */
	sendJsonResponse(request, 200, true, ServerError::ERR_OK, "", &doc);
}

/* post模式修改 */
void api_set_mode_config(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
	/* 檢查請求方法 */
	if (request->method() != HTTP_POST)
	{
		request->send(405, "text/plain", "Method Not Allowed");
		return;
	}

	/* 解析JSON */
	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, data);
	if (error)
	{
		ESP_LOGE("server", "Failed to parse JSON: %s\n", error.c_str());
		sendJsonResponse(request, 400, false, ServerError::ERR_INVALID_JSON, "Invalid JSON format");
		return;
	}

	/* 獲取模式配置 */
	int8_t mode = doc["mode"] | 0;
	if (mode < 1 || mode > 2) // 非法模式檢測
	{
		sendJsonResponse(request, 400, false, ServerError::ERR_MODE_INVALID, "Invalid mode value");
		return;
	}

	prefs.putInt("mode", mode);
	xQueueSend(mode_data_quene, &mode, 0); // 發送模式數據

	sendJsonResponse(request, 200, true, ServerError::ERR_OK);
	return;
}

/* get高級設置獲取 */
void api_advanced_config(AsyncWebServerRequest *request)
{
	/* 檢查請求方法 */
	if (request->method() != HTTP_GET)
	{
		request->send(405, "text/plain", "Method Not Allowed");
	}

	/* 構建JSON */
	JsonDocument doc;
	doc["correction_timer"] = prefs.getUInt("correction", 2000);

	/* 發送響應 */
	sendJsonResponse(request, 200, true, ServerError::ERR_OK, "", &doc);
}

/* post高級設置修改 */
void api_set_advanced_config(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
	/* 檢查請求方法 */
	if (request->method() == HTTP_POST)
	{
		request->send(405, "text/plain", "Method Not Allowed");
		return;
	}

	/* 解析JSON */
	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, data);
	if (error)
	{
		ESP_LOGE("server", "Failed to parse JSON: %s\n", error.c_str());
		sendJsonResponse(request, 400, false, ServerError::ERR_INVALID_JSON, "Invalid JSON format");
		return;
	}

	/* 獲取配置 */
	uint16_t correction_timer = doc["correction_timer"] | 2000;

	prefs.putUInt("correction", correction_timer);

	request->send(200, "application/json", "{\"success\":true}");
	return;
}