// Copyright 2025 NH3andH2O
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#define VERSION "1.0.1"

#include <ESP32Servo.h>
#include <LovyanGFX.hpp>
#include <Preferences.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <FFat.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include "gc9a01.h"
#include "eyesMove.h"
#include "wit.h"
#include "IMUAngle.h"

/* wifi SoftAP 名稱密碼 */
#define SSID "3rd-Eyes" // wifi SoftAP名稱
#define PASSWORD ""		// wifi SoftAP密碼

#define UPPER_EYELID_PIN 13 // 上眼皮伺服馬達引脚
#define LOWER_EYELID_PIN 14 // 下眼皮伺服馬達引脚
#define EYEBALL_PIN 21		// 眼球伺服馬達引脚

#define witEyes_RX_PIN 18 // wit眼睛模組RX引脚
#define witEyes_TX_PIN 8  // wit眼睛模組TX引脚
#define witHead_RX_PIN 39 // wit頭部模組RX引脚
#define witHead_TX_PIN 38 // wit頭部模組TX引脚

#define GC9A01_BLK_PIN 5  // GC9A01背光引脚
#define GC9A01_RST_PIN 15 // GC9A01重置引脚
#define GC9A01_CS_PIN 6	  // GC9A01片選引脚
#define GC9A01_DC_PIN 7	  // GC9A01數據/命令引脚
#define GC9A01_SCL_PIN 17 // GC9A01時鐘引脚
#define GC9A01_SDA_PIN 16 // GC9A01數據引脚

/* 函數宣告 */
void queueCreate(QueueHandle_t *quene, uint8_t queneSize, uint8_t queneType); // 佇列創建
void listFiles(const char *dirname);										  // 遍歷目錄檔案

/* 結構體定義 */
typedef struct
{
	witDataAngle relative_angle;			   // 角度差
	witDataAngularSpeed relative_angularSpeed; // 角速度差
	witDataAcceleration witEyes_acceleration;  // 眼睛加速度
	witDataAcceleration witHead_acceleration;  // 頭部加速度
} witPProcessingData;						   // wit處理數據結構體

typedef struct
{
	uint8_t R;
	double zetaR;
	double omega_nR;
	uint8_t lightMax;
	double zetaLightMax;
	double omega_nLightMax;
} GC9A01_data; // GC9A01數據結構體

typedef struct
{
	uint8_t eyelid_angle; // 眼睛張開角度
	int8_t x_angle;		  // x角度
	int8_t y_angle;		  // y角度
} eyesMove_data;		  // 眼睛數據結構體

/* 結構體宣告 */
eyesMove eyesmove(UPPER_EYELID_PIN, LOWER_EYELID_PIN, EYEBALL_PIN);

wit witEyes(SERIAL1, witEyes_RX_PIN, witEyes_TX_PIN, 115200); // wit眼睛模組
wit witHead(SERIAL2, witHead_RX_PIN, witHead_TX_PIN, 115200); // wit頭部模組

GC9A01 gc9a01(GC9A01_SDA_PIN, GC9A01_SCL_PIN, GC9A01_CS_PIN, GC9A01_DC_PIN, GC9A01_RST_PIN, GC9A01_BLK_PIN); // GC9A01實例

Preferences prefs; // 偏好設置實例

AsyncWebServer server(80); // Web服務器
AsyncWebSocket ws("/ws");  // WebSocket服務器

witData witEyes_data; // wit眼睛數據結構體
witData witHead_data; // wit頭部數據結構體

/* 佇列宣告 */
QueueHandle_t wit_data_quene;				 // 宣告wit原始佇列
QueueHandle_t wit_data_relative_angle_quene; // 宣告wit差值佇列
QueueHandle_t eyesmove_data_quene;			 // 宣告眼睛數據佇列
QueueHandle_t gc9a01_data_quene;			 // 宣告GC9A01佇列
QueueHandle_t wifiUpdate_data_quene;		 // 宣告WiFi更新佇列

/* 任務參照 */
TaskHandle_t taskWitEyesGetData_hamdle;		// 獲取wit眼睛數據任務
TaskHandle_t taskWitHeadGetData_hamdle;		// 獲取wit頭部數據任務
TaskHandle_t taskWitPProcessingData_hamdle; // 處理wit數據任務
TaskHandle_t taskGyroscopeTracking_hamdle;	// 陀螺儀跟蹤任務
TaskHandle_t taskGC9A01_hamdle;				// GC9A01任務
TaskHandle_t taskEyesMove_hamdle;			// 眼睛任務
TaskHandle_t taskWebServer_hamdle;			// Web服務器任務
TaskHandle_t taskNetwork_hamdle;			// 網絡任務

/** 網站相關函數 **/
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
		StaticJsonDocument<256> data_json;
		data_json["iswifi"] = prefs.getBool("iswifi", false);
		data_json["wifi_ssid"] = prefs.getString("wifi_ssid", "");
		data_json["wifi_password"] = prefs.getString("wifi_password", "");

		String jsonStr;
		serializeJson(data_json, jsonStr);

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
		JsonDocument data_json;

		/* 獲取JSON */
		DeserializationError error = deserializeJson(data_json, data);
		if (error)
		{
			Serial.printf("Failed to parse JSON: %s\n", error.c_str());
			request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
			return;
		}

		/* 獲取WiFi配置 */
		const char *ssid = data_json["wifi_ssid"] | "";
		const char *password = data_json["wifi_password"] | "";
		uint8_t iswifi = data_json["iswifi"] | false;

		/* 檢查WiFi配置 */
		if (iswifi == false || (iswifi == true && ssid[0] != '\0' && password[0] != '\0'))
		{
			/* 存儲WiFi配置 */
			prefs.putString("wifi_ssid", ssid);
			prefs.putString("wifi_password", password);
			prefs.putBool("iswifi", iswifi);

			/* 發送返回值 */
			StaticJsonDocument<256> response_doc;
			response_doc["success"] = true;
			String jsonStr;
			serializeJson(response_doc, jsonStr);
			request->send(200, "application/json", jsonStr);

			/* wifi更新 */
			uint8_t is_wifiUpdate = 1;							  // WiFi更新標誌
			xQueueSend(wifiUpdate_data_quene, &is_wifiUpdate, 0); // 發送WiFi更新信號
			return;
		}
		else
		{
			/* 發送錯誤返回值 */
			StaticJsonDocument<256> response_doc;
			response_doc["success"] = false;
			response_doc["message"] = "Invalid WiFi configuration";
			String jsonStr;
			serializeJson(response_doc, jsonStr);
			request->send(400, "application/json", jsonStr);
			return;
		}
	}
}

/** 任務相關函數 **/
/* 網絡相關任務 */
void taskNetwork(void *pvParameters)
{
	uint8_t is_wifiUpdate = 1;	 // WiFi更新標誌
	uint8_t wifi_old_status = 0; // 舊的WiFi連接狀態

	/* WiFi初始化 */
	WiFi.mode(WIFI_AP_STA);
	if (!WiFi.softAP(SSID, PASSWORD))
	{
		Serial.println("Failed to start SoftAP");
		vTaskDelete(NULL);
		return;
	}
	Serial.println("SoftAP Started");
	Serial.print("IP Address: ");
	Serial.println(WiFi.softAPIP());

	/* 伺服器任務創建 */
	xTaskCreatePinnedToCore(taskWebServer, "taskWebServer", 4096, NULL, 1, &taskWebServer_hamdle, 0); // 創建網絡任務

	while (1)
	{

		/* wifi更新 */
		if (is_wifiUpdate) // 如果WiFi需要更新
		{
			is_wifiUpdate = 0; // 重置WiFi更新標誌
			Serial.println("Updating WiFi connection...");
			if (prefs.getBool("iswifi", false) && prefs.getString("wifi_ssid", "").length() > 0 && prefs.getString("wifi_password", "").length() > 0)
			{
				/* 斷開當前wifi */
				if (WiFi.status() == WL_CONNECTED) // 如果WiFi已經連接
				{
					vTaskDelay(500);   // 讓前端可以收到返回消息
					WiFi.disconnect(); // 斷開WiFi連接
				}

				/* 連接新的wifi */
				WiFi.begin(prefs.getString("wifi_ssid", "").c_str(), prefs.getString("wifi_password", "").c_str()); // 連接WiFi
				uint64_t startTime = xTaskGetTickCount();															// 記錄開始時間
				while (WiFi.status() != WL_CONNECTED)																// 等待WiFi連接
				{
					if (xTaskGetTickCount() - startTime > 10000) // 如果超過10秒未連接
					{
						Serial.println("WiFi connection timed out");
						break;
					}
					vTaskDelay(100); // 延遲100毫秒
				}
				/* WiFi連接成功 */
			}
			else
			{
				/* 啓動SoftAP */
				vTaskDelay(500);   // 讓前端可以收到返回消息
				WiFi.disconnect(); // 斷開WiFi連接
			}
		}
		xQueueReceive(wifiUpdate_data_quene, &is_wifiUpdate, 1000); // 等待WiFi更新信號

		/* 檢查WiFi狀態 */
		if (WiFi.status() != wifi_old_status)
		{
			if (WiFi.status() == WL_CONNECTED) // 如果WiFi連接成功
			{
				Serial.println("WiFi connected successfully");
				Serial.print("IP Address: ");
				Serial.println(WiFi.localIP()); // 打印IP地址
			}
			else if (wifi_old_status == WL_CONNECTED)
			{
				Serial.println("WiFi disconnected");
			}
			wifi_old_status = WiFi.status(); // 更新舊的WiFi狀態
		}
	}
}

/* 伺服器相關任務 */
void taskWebServer(void *pvParameters)
{

	/* FFat啓動 */
	if (!FFat.begin(true))
	{
		Serial.println("FFat failed to start!");
		vTaskDelete(NULL);
	}
	Serial.println("FFat file system started");
	listFiles("/");

	/* mDNS啓動 */
	if (!MDNS.begin("3rdeyes"))
	{
		Serial.println("mDNS failed to start!");
		vTaskDelete(NULL);
	}
	Serial.println("mDNS started: http://3rdeyes.local");

	/* WebSocket服務器 */
	ws.onEvent(onSocketEvent); // 設置WebSocket事件處理函數

	/* 設置Web服務器路由 */
	server.addHandler(&ws); // 將WebSocket服務器添加到Web服務器
	server.serveStatic("/", FFat, "/www/").setDefaultFile("index.html");
	server.on("/api/wifi_config", HTTP_GET, api_wifi_config);
	server.on("/api/set_wifi_config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, api_set_wifi_config);

	/* 伺服器啓動 */
	server.begin();
	Serial.println("HTTP server started");
	while (1)
	{
		ws.cleanupClients();
		vTaskDelay(1000);
	}
}

/* 獲取wit數據任務 */
void taskWitGetData(void *arg)
{
	witData wit_data;		 // wit數據結構體
	wit *myWit = (wit *)arg; // 獲取wit數據

	uint8_t serialPort = myWit->wit_serial_get();					   // 獲取Serial端口
	String wit_name = (serialPort == SERIAL1) ? "witEyes" : "witHead"; // 獲取Serial端口名稱

	/* 初始化wit */
	int8_t wit_status = myWit->wit_init(); // 初始化wit眼睛模組
	switch (wit_status)
	{
		case 0:
			printf("%s init success\r\n", wit_name.c_str()); // 打印初始化成功
			break;
		case WIT_INIT_ERROR:
			printf("%s init error\r\n", wit_name.c_str()); // 打印初始化錯誤
			break;
		case SERIAL_INIT_ERROR:
			printf("%s serial init error\r\n", wit_name.c_str()); // 打印Serial初始化錯誤
			break;
		default:
			printf("%s init unknown error\r\n", wit_name.c_str()); // 打印未知錯誤
			break;
	}
	while (wit_status) // 失敗進入死循環
	{
		vTaskDelay(1000);
	}

	/* 清除緩存 */
	myWit->wit_flush();

	/* 獲取數據 */
	while (1)
	{
		wit_data = myWit->wit_get_data();		  // 獲取數據
		xQueueSend(wit_data_quene, &wit_data, 0); // 將數據放入佇列
		vTaskDelay(1);
	}
}

/* 處理角度差 */
void taskWitPProcessingData(void *arg)
{
	witData wit_data;		   // wit數據結構體
	witPProcessingData result; // 處理數據結構體結果

	witDataQuaternion witEyes_reference_quaternion;	   // 眼睛参考四元數
	witDataQuaternion witHead_reference_quaternion;	   // 頭部参考四元數
	witDataQuaternion *reference_quaternion = nullptr; // 參考四元數指標

	witDataQuaternion witEyes_quaternion;	 // 眼睛四元數
	witDataQuaternion witHead_quaternion;	 // 頭部四元數
	witDataQuaternion *quaternion = nullptr; // 四元數指標

	witDataQuaternion witEyes_angularSpeed_quaternion;			// 眼睛角速度四元數
	witDataQuaternion witHead_angularSpeed_quaternion;			// 頭部角速度四元數
	witDataQuaternion witEyes_angularSpeed_world_quaternion;	// 眼睛相對於世界坐標系的四元數
	witDataQuaternion witHead_angularSpeed_world_quaternion;	// 頭部相對於世界坐標系的四元數
	witDataQuaternion *angularSpeed_quaternion = nullptr;		// 角速度四元數指標
	witDataQuaternion *angularSpeed_world_quaternion = nullptr; // 角速度相對於世界坐標系的四元數

	witDataQuaternion witEyes_acceleration_quaternion;	  // 眼睛加速度四元數
	witDataQuaternion witHead_acceleration_quaternion;	  // 頭部加速度四元數
	witDataQuaternion *acceleration_quaternion = nullptr; // 加速度四元數指標

	witDataQuaternion relative_quaternion; // 眼睛四元數差值

	witDataAngle relative_angle;			   // 眼睛角度差值
	witDataAngularSpeed relative_angularSpeed; // 眼睛角速度差值

	witDataAcceleration witEyes_acceleration;	 // 眼睛加速度
	witDataAcceleration witHead_acceleration;	 // 頭部加速度
	witDataAcceleration *acceleration = nullptr; // 加速度指標

	uint8_t witEyes_angle_status = 0; // 眼睛角度狀態
	uint8_t witHead_angle_status = 0; // 頭部角度狀態
	uint8_t *angle_status = nullptr;  // 角度狀態指標

	uint8_t read_count = 0; // 讀取計數

	/* 抛棄前10次數據 */
	while (read_count < 10)
	{
		xQueueReceive(wit_data_quene, &wit_data, portMAX_DELAY); // 從佇列中獲取數據
		read_count++;
	}

	while (1)
	{
		/* 數據獲取 */
		xQueueReceive(wit_data_quene, &wit_data, portMAX_DELAY); // 從佇列中獲取數據
		String wit_name = (wit_data.serialPort == SERIAL1) ? "witEyes" : "witHead";

		/* 數據處理 */
		switch (wit_data.status) // 數據狀態
		{
			case 0: // 數據正常

				/* 眼睛 */
				if (wit_data.serialPort == SERIAL1)
				{
					reference_quaternion = &witEyes_reference_quaternion;					// 設置參考四元數
					quaternion = &witEyes_quaternion;										// 設置四元數
					angularSpeed_quaternion = &witEyes_angularSpeed_quaternion;				// 設置角速度四元數
					acceleration_quaternion = &witEyes_acceleration_quaternion;				// 設置加速度四元數
					angularSpeed_world_quaternion = &witEyes_angularSpeed_world_quaternion; // 設置角速度相對於世界坐標系的四元數
					acceleration = &witEyes_acceleration;									// 設置加速度
					angle_status = &witEyes_angle_status;									// 設置角度狀態
				}

				/* 頭部 */
				else if (wit_data.serialPort == SERIAL2)
				{
					reference_quaternion = &witHead_reference_quaternion;					// 設置參考四元數
					quaternion = &witHead_quaternion;										// 設置四元數
					angularSpeed_quaternion = &witHead_angularSpeed_quaternion;				// 設置角速度四元數
					acceleration_quaternion = &witHead_acceleration_quaternion;				// 設置加速度四元數
					angularSpeed_world_quaternion = &witHead_angularSpeed_world_quaternion; // 設置角速度相對於世界坐標系的四元數
					acceleration = &witHead_acceleration;									// 設置加速度
					angle_status = &witHead_angle_status;									// 設置角度狀態
				}

				if (reference_quaternion && quaternion && angle_status && acceleration && acceleration_quaternion && angularSpeed_quaternion &&
					angularSpeed_world_quaternion) // 防止野指標
				{
					/* 設置參考角度 */
					if (*angle_status == 0)
					{
						reference_quaternion->wquaternion = wit_data.wquaternion; // 設置參考四元數
						reference_quaternion->xquaternion = wit_data.xquaternion;
						reference_quaternion->yquaternion = wit_data.yquaternion;
						reference_quaternion->zquaternion = wit_data.zquaternion;
						*angle_status = 1; // 設置角度狀態
					}

					/* 處理參考角度 */
					quaternion->wquaternion = wit_data.wquaternion; // 設置四元數
					quaternion->xquaternion = wit_data.xquaternion;
					quaternion->yquaternion = wit_data.yquaternion;
					quaternion->zquaternion = wit_data.zquaternion;

					*quaternion = IMUAngle::quaternion_multiply(*quaternion, IMUAngle::quaternion_conjugate(*reference_quaternion)); // 處理參考四元數

					/* 處理角速度 */
					angularSpeed_quaternion->wquaternion = 0; // 設置角速度四元數
					angularSpeed_quaternion->xquaternion = wit_data.xangular_speed;
					angularSpeed_quaternion->yquaternion = wit_data.yangular_speed;
					angularSpeed_quaternion->zquaternion = wit_data.zangular_speed;

					*angularSpeed_world_quaternion =
						IMUAngle::quaternion_multiply(IMUAngle::quaternion_multiply(*reference_quaternion, *angularSpeed_quaternion),
													  IMUAngle::quaternion_conjugate(*reference_quaternion)); // 計算角速度四元數

					/* 處理加速度 */
					acceleration_quaternion->wquaternion = 0; // 設置加速度四元數
					acceleration_quaternion->xquaternion = wit_data.xacceleration;
					acceleration_quaternion->yquaternion = wit_data.yacceleration;
					acceleration_quaternion->zquaternion = wit_data.zacceleration;

					witDataQuaternion acceleration_world_quaternion =
						IMUAngle::quaternion_multiply(IMUAngle::quaternion_multiply(*reference_quaternion, *acceleration_quaternion),
													  IMUAngle::quaternion_conjugate(*reference_quaternion)); // 計算加速度四元數
					acceleration->xacceleration = acceleration_world_quaternion.xquaternion;				  // 設置加速度
					acceleration->yacceleration = acceleration_world_quaternion.yquaternion;
					acceleration->zacceleration = acceleration_world_quaternion.zquaternion;
				}
				break;
			default:
				break;
		}

		/* 差角計算 */
		if (witEyes_angle_status == 1 && witHead_angle_status == 1) // 眼睛和頭部數據都獲取完成
		{
			/* 角度差 */
			relative_quaternion =
				IMUAngle::quaternion_multiply(witEyes_quaternion, IMUAngle::quaternion_conjugate(witHead_quaternion)); // 計算眼睛和頭部的四元數差值
			relative_angle = IMUAngle::quaternion_to_euler(relative_quaternion);									   // 計算眼睛和頭部的角度差值

			/* 角速度差 */
			relative_angularSpeed.xangular_speed =
				witEyes_angularSpeed_quaternion.xquaternion - witHead_angularSpeed_quaternion.xquaternion; // 計算眼睛和頭部的角速度差值
			relative_angularSpeed.yangular_speed = witEyes_angularSpeed_quaternion.yquaternion - witHead_angularSpeed_quaternion.yquaternion;
			relative_angularSpeed.zangular_speed =
				witEyes_angularSpeed_quaternion.zquaternion - witHead_angularSpeed_quaternion.zquaternion; // 計算眼睛和頭部的角速度差值

			/* 數據推送 */
			result.relative_angle = relative_angle;				   // 設置角度差
			result.relative_angularSpeed = relative_angularSpeed;  // 設置角速度差
			result.witEyes_acceleration = witEyes_acceleration;	   // 設置眼睛加速度
			result.witHead_acceleration = witHead_acceleration;	   // 設置頭部加速度
			xQueueSend(wit_data_relative_angle_quene, &result, 0); // 從佇列中獲取數據
		}
	}
}

/* 陀螺儀跟蹤模式 */
void taskGyroscopeTracking(void *arg)
{
	witPProcessingData data_get;   // 數據接收
	eyesMove_data angle_data_send; // 角度數據發送
	GC9A01_data gc9a01_data;	   // GC9A01數據結構體

	double eyes_x = 0;			   // x角度
	double eyes_y = 0;			   // y角度
	double calculate_angularSpeed; // 角速度絕對值

	uint64_t last_move_time[2] = {0, 0}; // 上次移動時間
	while (1)
	{
		if (xQueueReceive(wit_data_relative_angle_quene, &data_get, portMAX_DELAY) == pdTRUE) // 從佇列中獲取數據
		{
			/* x角度範圍 */
			eyes_x = map(constrain(data_get.relative_angle.zangle, -60, 60), 60, -60, -55, 55); // z角度映射

			/* y角度範圍 */
			eyes_y = map(constrain(data_get.relative_angle.yangle, -30, 30), 30, -30, 35, -35); // y角度映射

			/* 傳遞眼睛角度 */
			angle_data_send.x_angle = round((int8_t)eyes_x);	  // 設置x角度
			angle_data_send.y_angle = round((int8_t)eyes_y);	  // 設置y角度
			angle_data_send.eyelid_angle = 45;					  // 設置眼睛張開角度
			xQueueSend(eyesmove_data_quene, &angle_data_send, 0); // 傳遞x角度

			/* 獲取角速度 */
			calculate_angularSpeed =
				sqrt(pow(data_get.relative_angularSpeed.xangular_speed, 2) + pow(data_get.relative_angularSpeed.yangular_speed, 2) +
					 pow(data_get.relative_angularSpeed.zangular_speed, 2)); // 計算角速度
			if (calculate_angularSpeed > 80)								 // 如果角速度大於給定值
			{
				last_move_time[1] = xTaskGetTickCount();
				last_move_time[0] = xTaskGetTickCount();
			}
			if (calculate_angularSpeed > 40)
			{
				last_move_time[0] = xTaskGetTickCount();
			}
		}

		/* GC9A01數據傳遞 */
		if (xTaskGetTickCount() - last_move_time[1] < 200)
		{
			gc9a01_data.R = 65;								// 設置GC9A01半徑
			gc9a01_data.zetaR = 0.9;						// 設置GC9A01阻尼比
			gc9a01_data.omega_nR = 20;						// 設置GC9A01自然頻率
			gc9a01_data.lightMax = 250;						// 設置GC9A01光暈最大值
			gc9a01_data.zetaLightMax = 0.9;					// 設置GC9A01光暈阻尼比
			gc9a01_data.omega_nLightMax = 20;				// 設置GC9A01光暈自然頻率
			xQueueSend(gc9a01_data_quene, &gc9a01_data, 0); // 傳遞GC9A01數據
		}
		else if (xTaskGetTickCount() - last_move_time[1] > 200 && xTaskGetTickCount() - last_move_time[0] < 200)
		{
			gc9a01_data.R = 73;								// 設置GC9A01半徑
			gc9a01_data.zetaR = 0.9;						// 設置GC9A01阻尼比
			gc9a01_data.omega_nR = 10;						// 設置GC9A01自然頻率
			gc9a01_data.lightMax = 200;						// 設置GC9A01光暈最大值
			gc9a01_data.zetaLightMax = 0.9;					// 設置GC9A01光暈阻尼比
			gc9a01_data.omega_nLightMax = 10;				// 設置GC9A01光暈自然頻率
			xQueueSend(gc9a01_data_quene, &gc9a01_data, 0); // 傳遞GC9A01數據
		}
		else
		{
			gc9a01_data.R = 80;								// 設置GC9A01半徑
			gc9a01_data.zetaR = 1;							// 設置GC9A01阻尼比
			gc9a01_data.omega_nR = 5;						// 設置GC9A01自然頻率
			gc9a01_data.lightMax = 150;						// 設置GC9A01光暈最大值
			gc9a01_data.zetaLightMax = 1;					// 設置GC9A01光暈阻尼比
			gc9a01_data.omega_nLightMax = 5;				// 設置GC9A01光暈自然頻率
			xQueueSend(gc9a01_data_quene, &gc9a01_data, 0); // 傳遞GC9A01數據
		}
		vTaskDelay(1);
	}
}

/* 畫眼睛 */
void taskGC9A01(void *arg)
{
	/* 初始化GC9A01 */
	gc9a01.GC9A01_init();				 // 初始化GC9A01
	gc9a01.GC9A01_setEyes_r(80, 1, 10);	 // 設置眼睛半徑
	uint8_t is_GC9A01_update_finish = 0; // GC9A01更新狀態
	GC9A01_data gc9a01_data;
	while (1)
	{
		/* 獲取GC9A01數據 */

		if (is_GC9A01_update_finish) // 如果更新完成
		{
			if (xQueueReceive(gc9a01_data_quene, &gc9a01_data, portMAX_DELAY) == pdTRUE)
			{
				gc9a01.GC9A01_setEyes_r(gc9a01_data.R, gc9a01_data.zetaR, gc9a01_data.omega_nR);							 // 設置眼睛半徑
				gc9a01.GC9A01_setEyes_lightMax(gc9a01_data.lightMax, gc9a01_data.zetaLightMax, gc9a01_data.omega_nLightMax); // 設置光暈最大值
			}
		}
		else // 沒有完成更新
		{
			if (xQueueReceive(gc9a01_data_quene, &gc9a01_data, 0) == pdTRUE)
			{
				gc9a01.GC9A01_setEyes_r(gc9a01_data.R, gc9a01_data.zetaR, gc9a01_data.omega_nR);							 // 設置眼睛半徑
				gc9a01.GC9A01_setEyes_lightMax(gc9a01_data.lightMax, gc9a01_data.zetaLightMax, gc9a01_data.omega_nLightMax); // 設置光暈最大值
			}
		}
		is_GC9A01_update_finish = gc9a01.GC9A01_update(); // 更新GC9A01
	}
}

/* 眼睛移動 */
void taskEyesMove(void *arg)
{
	eyesMove_data data_get;				   // 眼睛數據結構體
	uint8_t is_eyesmove_update_finish = 0; // 眼睛更新狀態

	/* 初始化眼睛 */
	eyesmove.eyesMove_init(); // 初始化眼睛
	eyesmove.eyesMove_angle_pid(4.0, 0.2, 0.25);
	while (1)
	{
		/* 获取数据 */
		if (is_eyesmove_update_finish) // 如果更新完成
		{
			if (xQueueReceive(eyesmove_data_quene, &data_get, portMAX_DELAY) == pdTRUE) // 從佇列中獲取數據
			{
				eyesmove.eyesMove_angle_set(data_get.eyelid_angle, data_get.x_angle, data_get.y_angle); // 設置眼睛角度
			}
		}
		else // 沒有完成更新
		{
			if (xQueueReceive(eyesmove_data_quene, &data_get, 0) == pdTRUE) // 從佇列中獲取數據
			{
				eyesmove.eyesMove_angle_set(45, data_get.x_angle, data_get.y_angle); // 設置眼睛角度
			}
		}
		is_eyesmove_update_finish = eyesmove.eyesMove_update(); // 更新眼睛
		vTaskDelay(1);
	}
}

void setup()
{
	Serial.begin(115200);
	prefs.begin("preferences", false);
	while (xTaskGetTickCount() < 2000)
	{
		; // 等待2秒
	}

	/* 佇列建立 */
	Serial.println("quene create..."); // 打印佇列建立狀態

	queueCreate(&wit_data_quene, 10, sizeof(witData));							 // witData結構體佇列
	queueCreate(&wit_data_relative_angle_quene, 10, sizeof(witPProcessingData)); // witPProcessingData結構體佇列
	queueCreate(&eyesmove_data_quene, 10, sizeof(eyesMove_data));				 // eyesMove_data結構體佇列
	queueCreate(&gc9a01_data_quene, 10, sizeof(GC9A01_data));					 // GC9A01_data結構體佇列
	queueCreate(&wifiUpdate_data_quene, 10, sizeof(uint8_t));					 // WiFi更新佇列

	Serial.println("quene create success"); // 打印佇列建立成功狀態

	Serial.println("wit init..."); // 打印初始化狀態

	/* 任務建立 */
	xTaskCreatePinnedToCore(taskNetwork, "taskNetwork", 4096, NULL, 1, &taskNetwork_hamdle, 0);									 // 創建網絡任務
	xTaskCreatePinnedToCore(taskWitGetData, "taskWitEyesGetData", 4096, &witEyes, 1, &taskWitEyesGetData_hamdle, 1);			 // 創建獲取數據任務
	xTaskCreatePinnedToCore(taskWitGetData, "taskWitHeadGetData", 4096, &witHead, 1, &taskWitHeadGetData_hamdle, 1);			 // 創建獲取數據任務
	xTaskCreatePinnedToCore(taskWitPProcessingData, "taskWitPProcessingData", 4096, NULL, 1, &taskWitPProcessingData_hamdle, 1); // 創建數據處理任務
	xTaskCreatePinnedToCore(taskGC9A01, "taskGC9A01", 8192, NULL, 1, &taskGC9A01_hamdle, 0);									 // 創建GC9A01任務
	xTaskCreatePinnedToCore(taskEyesMove, "taskEyesMove", 4096, NULL, 1, &taskEyesMove_hamdle, 1);								 // 創建眼睛移動任務
	xTaskCreatePinnedToCore(taskGyroscopeTracking, "taskGyroscopeTracking", 4096, NULL, 1, &taskGyroscopeTracking_hamdle, 1);	 // 創建陀螺儀跟蹤任務
}

void loop() { vTaskDelay(1000); }

void queueCreate(QueueHandle_t *quene, uint8_t queneSize, uint8_t queneType)
{
	/* 佇列建立 */
	*quene = xQueueCreate(queneSize, queneType);

	/* 佇列建立失敗 */
	if (*quene == NULL)
	{
		Serial.println("quene create error");
		while (2)
		{
			vTaskDelay(1000);
		}
	}
}

void listFiles(const char *dirname)
{
	File root = FFat.open(dirname); // 打開檔案
	if (!root)
	{
		Serial.println("Failed to open directory");
		return;
	}

	if (!root.isDirectory())
	{
		Serial.println("Not a directory");
		return;
	}

	Serial.println("Listing files:");

	/* 遍歷檔案 */
	File file = root.openNextFile();
	while (file)
	{
		if (file.isDirectory())
		{
			Serial.print("[DIR] ");
			Serial.println(file.name());
		}
		else
		{
			Serial.print("[FILE] ");
			Serial.print(file.name());
			Serial.print(" - ");
			Serial.println(file.size());
		}
		file = root.openNextFile();
	}
}