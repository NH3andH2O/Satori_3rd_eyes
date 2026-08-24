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

	if (request->method() == HTTP_OPTIONS)
	{
		AsyncWebServerResponse *response = request->beginResponse(204);
		response->addHeader("Access-Control-Allow-Origin", "*");
		response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
		response->addHeader("Access-Control-Allow-Headers", "Content-Type");
		request->send(response);
		return;
	}

	if (urlPath.startsWith("/api/"))
	{
		AsyncWebServerResponse *response = request->beginResponse(404, "application/json", "{\"success\":false,\"message\":\"API not found\"}");
		response->addHeader("Access-Control-Allow-Origin", "*");
		request->send(response);
		return;
	}

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
			/* 伺服電機預覽數據 */
			else if (type.startsWith("preview"))
			{
				if (data_doc["payload"]["eyeball_angle"].isNull() || data_doc["payload"]["upper_eyelid_angle"].isNull() ||
					data_doc["payload"]["lower_eyelid_angle"].isNull()) // 檢查是否缺少必要的欄位
				{
					ESP_LOGE("server", "Missing required fields in servo preview data");
					break;
				}

				servoSet_data data_send;

				data_send.eyeball_angle = data_doc["payload"]["eyeball_angle"] | 0;
				data_send.upper_eyelid_angle = data_doc["payload"]["upper_eyelid_angle"] | 0;
				data_send.lower_eyelid_angle = data_doc["payload"]["lower_eyelid_angle"] | 0;
				xQueueSend(servoSet_data_quene, &data_send, 0);
			}
			break;
		}
		default:
			break;
	}
}

/* get後端版本獲取 */
void api_version(AsyncWebServerRequest *request)
{
	/* 檢查請求方法 */
	if (request->method() != HTTP_GET)
	{
		request->send(405, "text/plain", "Method Not Allowed");
		return;
	}

	/* 構建JSON */
	JsonDocument doc;
	doc["version"] = VERSION;

	/* 發送響應 */
	sendJsonResponse(request, 200, true, ServerError::ERR_OK, "", &doc);
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

	/* 獲取WiFi配置 */
	AppConfig::WiFiConfig wifiConfig = config.getWiFiConfig();

	/* 構建JSON */
	JsonDocument doc;
	doc["is_wifi"] = wifiConfig.is_enabled;
	doc["ssid"] = wifiConfig.ssid;
	doc["password"] = wifiConfig.password;

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
	AppConfig::WiFiConfig wifiConfig;
	wifiConfig.ssid = ssid;
	wifiConfig.password = password;
	wifiConfig.is_enabled = is_wifi;
	config.setWiFiConfig(wifiConfig);

	ESP_LOGI("server", "WiFi config updated: SSID=%s, Enabled=%d", ssid, is_wifi);

	/* 發送成功響應 */
	sendJsonResponse(request, 200, true, ServerError::ERR_OK);

	/* 發送WiFi更新信號 */
	const NetworkCommand command = NetworkCommand::StaConfigUpdate;
	if (xQueueSend(wifiUpdate_data_quene, &command, 0) != pdTRUE)
	{
		ESP_LOGE("server", "Failed to queue WiFi configuration update");
	}
}

/* get softAP設置獲取 */
void api_softAP_config(AsyncWebServerRequest *request)
{
	/* 檢查請求方法 */
	if (request->method() != HTTP_GET)
	{
		request->send(405, "text/plain", "Method Not Allowed");
	}

	/* 獲取SoftAP配置 */
	AppConfig::SoftAPConfig softAPConfig = config.getSoftAPConfig();

	/* 構建JSON */
	JsonDocument doc;
	doc["ssid"] = softAPConfig.ssid;
	doc["password"] = softAPConfig.password;

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
	AppConfig::SoftAPConfig softAPConfig = config.getSoftAPConfig();
	if (ssid[0] == '\0')
	{
		softAPConfig.ssid = DEFAULT_SSID;
	}
	else
	{
		softAPConfig.ssid = ssid;
	}
	if (is_change_password)
	{
		if (strlen(password) > 0)
		{
			softAPConfig.password = password;
		}
		else
		{
			softAPConfig.password = "";
		}
	}
	config.setSoftAPConfig(softAPConfig);

	/* 發送成功響應 */
	sendJsonResponse(request, 200, true, ServerError::ERR_OK);

	/* 發送SoftAP配置更新信號 */
	const NetworkCommand command = NetworkCommand::SoftAPConfigUpdate;
	if (xQueueSend(wifiUpdate_data_quene, &command, 0) != pdTRUE)
	{
		ESP_LOGE("server", "Failed to queue SoftAP configuration update");
	}
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

	/* 獲取模式配置 */
	AppConfig::ModeConfig modeConfig = config.getModeConfig();

	/* 構建JSON */
	JsonDocument doc;
	doc["mode"] = modeConfig.mode;

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

	/* 存儲模式配置 */
	AppConfig::ModeConfig modeConfig;
	modeConfig.mode = mode;
	config.setModeConfig(modeConfig);
	xQueueOverwrite(mode_data_quene, &mode); // 快速切換時只保留最新模式

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

	/* 獲取高級設置 */
	AppConfig::AdvancedConfig advancedConfig = config.getAdvancedConfig();

	/* 構建JSON */
	JsonDocument doc;
	doc["correction_timer"] = advancedConfig.correction_timer;
	doc["gyroscope_eyelid_angle"] = advancedConfig.gyroscope_eyelid_angle;

	/* 發送響應 */
	sendJsonResponse(request, 200, true, ServerError::ERR_OK, "", &doc);
}

/* post高級設置修改 */
void api_set_advanced_config(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
	/* 檢查請求方法 */
	if (request->method() != HTTP_POST)
	{
		request->send(405, "text/plain", "Method Not Allowed");
		return;
	}

	/* 解析JSON */
	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, data, len);
	if (error)
	{
		ESP_LOGE("server", "Failed to parse JSON: %s\n", error.c_str());
		sendJsonResponse(request, 400, false, ServerError::ERR_INVALID_JSON, "Invalid JSON format");
		return;
	}

	// 部分更新需區分欄位缺失與顯式null
	bool has_correction_timer = false;
	bool has_gyroscope_eyelid_angle = false;
	for (JsonPairConst entry : doc.as<JsonObjectConst>())
	{
		if (strcmp(entry.key().c_str(), "correction_timer") == 0)
		{
			has_correction_timer = true;
		}
		else if (strcmp(entry.key().c_str(), "gyroscope_eyelid_angle") == 0)
		{
			has_gyroscope_eyelid_angle = true;
		}
	}
	if (!has_correction_timer && !has_gyroscope_eyelid_angle)
	{
		sendJsonResponse(request, 400, false, ServerError::ERR_ADVANCED_CONFIG_MISSING, "Missing advanced configuration field");
		return;
	}

	// 未提交欄位保留現值以相容舊版前端
	AppConfig::AdvancedConfig advancedConfig = config.getAdvancedConfig();
	if (has_correction_timer)
	{
		if (!doc["correction_timer"].is<int>() || doc["correction_timer"].as<int>() < 0 || doc["correction_timer"].as<int>() > UINT16_MAX)
		{
			sendJsonResponse(request, 400, false, ServerError::ERR_ADVANCED_CONFIG_INVALID,
							 "correction_timer must be an integer between 0 and 65535");
			return;
		}
		advancedConfig.correction_timer = doc["correction_timer"].as<uint16_t>();
	}

	if (has_gyroscope_eyelid_angle)
	{
		if (!doc["gyroscope_eyelid_angle"].is<int>() || doc["gyroscope_eyelid_angle"].as<int>() < 0 || doc["gyroscope_eyelid_angle"].as<int>() > 80)
		{
			sendJsonResponse(request, 400, false, ServerError::ERR_ADVANCED_CONFIG_INVALID,
							 "gyroscope_eyelid_angle must be an integer between 0 and 80");
			return;
		}
		advancedConfig.gyroscope_eyelid_angle = doc["gyroscope_eyelid_angle"].as<uint8_t>();
	}

	config.setAdvancedConfig(advancedConfig);
	// 各任務保留一份最新設定，避免consumer競爭同一訊息
	xQueueOverwrite(wit_advanced_config_update_quene, &advancedConfig);
	xQueueOverwrite(gyroscope_advanced_config_update_quene, &advancedConfig);

	/* 發送成功響應 */
	sendJsonResponse(request, 200, true, ServerError::ERR_OK);
	return;
}

/* get舵機配置獲取 */
void api_servo_config(AsyncWebServerRequest *request)
{
	/* 檢查請求方法 */
	if (request->method() != HTTP_GET)
	{
		request->send(405, "text/plain", "Method Not Allowed");
	}

	/* 獲取舵機配置 */
	AppConfig::ServoConfig servoConfig = config.getServoConfig();

	/* 構建JSON */
	JsonDocument doc;
	doc["is_setup"] = servoConfig.is_setup;
	doc["max_upper_eyelid"] = servoConfig.max_upper_eyelid;
	doc["mid_upper_eyelid"] = servoConfig.mid_upper_eyelid;
	doc["min_upper_eyelid"] = servoConfig.min_upper_eyelid;
	doc["max_lower_eyelid"] = servoConfig.max_lower_eyelid;
	doc["mid_lower_eyelid"] = servoConfig.mid_lower_eyelid;
	doc["min_lower_eyelid"] = servoConfig.min_lower_eyelid;
	doc["max_eyeball"] = servoConfig.max_eyeball;
	doc["mid_eyeball"] = servoConfig.mid_eyeball;
	doc["min_eyeball"] = servoConfig.min_eyeball;

	/* 發送響應 */
	sendJsonResponse(request, 200, true, ServerError::ERR_OK, "", &doc);
}

/* post舵機配置修改 */
void api_set_servo_config(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
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

	const char *requiredFields[] = {
		"max_upper_eyelid", "mid_upper_eyelid", "min_upper_eyelid", "max_lower_eyelid", "mid_lower_eyelid",
		"min_lower_eyelid", "max_eyeball",		"mid_eyeball",		"min_eyeball",
	};

	for (const char *field : requiredFields)
	{
		if (doc[field].isNull())
		{
			String message = "Missing required servo configuration field: ";
			message += field;
			sendJsonResponse(request, 400, false, ServerError::ERR_SERVO_CONFIG_MISSING, message.c_str());
			return;
		}

		if (!doc[field].is<int>() || doc[field].as<int>() < 0 || doc[field].as<int>() > 180)
		{
			String message = "Servo configuration field must be an integer between 0 and 180: ";
			message += field;
			sendJsonResponse(request, 400, false, ServerError::ERR_SERVO_CONFIG_INVALID, message.c_str());
			return;
		}
	}

	if (!(doc["min_upper_eyelid"].as<int>() < doc["mid_upper_eyelid"].as<int>() &&
		  doc["mid_upper_eyelid"].as<int>() < doc["max_upper_eyelid"].as<int>()) ||
		!(doc["min_lower_eyelid"].as<int>() < doc["mid_lower_eyelid"].as<int>() &&
		  doc["mid_lower_eyelid"].as<int>() < doc["max_lower_eyelid"].as<int>()) ||
		!(doc["min_eyeball"].as<int>() < doc["mid_eyeball"].as<int>() && doc["mid_eyeball"].as<int>() < doc["max_eyeball"].as<int>()))
	{
		sendJsonResponse(request, 400, false, ServerError::ERR_SERVO_CONFIG_INVALID, "Servo configuration must satisfy min < mid < max");
		return;
	}

	/* 獲取配置 */
	AppConfig::ServoConfig servoConfig = config.getServoConfig();
	if (!doc["is_setup"].isNull())
	{
		servoConfig.is_setup = doc["is_setup"].as<bool>();
	}
	servoConfig.max_upper_eyelid = doc["max_upper_eyelid"].as<uint8_t>();
	servoConfig.mid_upper_eyelid = doc["mid_upper_eyelid"].as<uint8_t>();
	servoConfig.min_upper_eyelid = doc["min_upper_eyelid"].as<uint8_t>();
	servoConfig.max_lower_eyelid = doc["max_lower_eyelid"].as<uint8_t>();
	servoConfig.mid_lower_eyelid = doc["mid_lower_eyelid"].as<uint8_t>();
	servoConfig.min_lower_eyelid = doc["min_lower_eyelid"].as<uint8_t>();
	servoConfig.max_eyeball = doc["max_eyeball"].as<uint8_t>();
	servoConfig.mid_eyeball = doc["mid_eyeball"].as<uint8_t>();
	servoConfig.min_eyeball = doc["min_eyeball"].as<uint8_t>();

	/* 存儲配置 */
	config.setServoConfig(servoConfig);
	eyesmove.eyesMove_servo_limit_update();

	/* 發送成功響應 */
	sendJsonResponse(request, 200, true, ServerError::ERR_OK);
	return;
}