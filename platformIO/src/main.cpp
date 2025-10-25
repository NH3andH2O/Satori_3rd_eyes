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

#define VERSION "1.2.0"

/* 第三方程式庫 */
#include <Arduino.h>
#include <ESP32Servo.h>
#include <LovyanGFX.hpp>
#include <Preferences.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <LittleFS.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <driver/uart.h>
/* 自定義程式庫 */
#include <gc9a01.h>
#include <eyesMove.h>
#include <wit.h>
#include <IMUAngle.h>
#include <serverResponse.h>
#include "types.h"

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

#define GYROSCOPE_TRACKS_MODE 1
#define NETWORK_CONTROL_MODE 2

/** 函數宣告 **/
/* 事件函數宣告 */
void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info); // WiFi事件處理函數

/* 回調函數宣告 */
void TimerReconnectWiFi(TimerHandle_t xTimer); // WiFi重連定時器回調函數

/* 任務函數宣告 */
void taskNetwork(void *pvParameters);		 // 網絡任務
void taskWebServer(void *pvParameters);		 // Web服務器任務
void taskModeManagement(void *pvParameters); // 模式管理任務
void taskWitGetData(void *arg);				 // 獲取wit數據任務
void taskWitPProcessingData(void *arg);		 // 處理wit數據任務
void taskGyroscopeTracking(void *arg);		 // 陀螺儀跟蹤任務
void taskNetworkControl(void *arg);			 // 網絡控制任務
void taskNetworkControlGC9A01(void *arg);	 // 網絡控制屏幕任務
void taskGC9A01(void *arg);					 // GC9A01任務
void taskEyesMove(void *arg);				 // 眼睛任務
void taskUART0Read(void *arg);				 // UART0讀取任務

/* 其他函數宣告 */
void queueCreate(QueueHandle_t *quene, uint8_t queneSize, uint8_t queneType); // 佇列創建
void TaskDeleteSafe(TaskHandle_t *pHandle, uint32_t yieldMs = 0);
String macToString(const uint8_t mac[6]); // MAC地址轉字符串

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
QueueHandle_t wit_data_quene;					// 宣告wit原始佇列
QueueHandle_t wit_data_relative_angle_quene;	// 宣告wit差值佇列
QueueHandle_t eyesmove_data_quene;				// 宣告眼睛數據佇列
QueueHandle_t gc9a01_data_quene;				// 宣告GC9A01佇列
QueueHandle_t wifiUpdate_data_quene;			// 宣告WiFi更新佇列
QueueHandle_t correction_timer_update_quene;	// 宣告角度重置時間器更新佇列
QueueHandle_t mode_data_quene;					// 宣告模式數據佇列
QueueHandle_t uart0_queue;						// UART0事件佇列
QueueHandle_t network_control_data_quene;		// 網絡數據佇列
QueueHandle_t network_control_speed_data_quene; // 網絡控制速度數據佇列

/* 任務參照 */
TaskHandle_t taskWitEyesGetData_hamdle;		  // 獲取wit眼睛數據任務
TaskHandle_t taskWitHeadGetData_hamdle;		  // 獲取wit頭部數據任務
TaskHandle_t taskWitPProcessingData_hamdle;	  // 處理wit數據任務
TaskHandle_t taskGyroscopeTracking_hamdle;	  // 陀螺儀跟蹤任務
TaskHandle_t taskNetworkControl_hamdle;		  // 網絡控制任務
TaskHandle_t taskNetworkControlGC9A01_hamdle; // 網絡控制屏幕任務
TaskHandle_t taskGC9A01_hamdle;				  // GC9A01任務
TaskHandle_t taskEyesMove_hamdle;			  // 眼睛任務
TaskHandle_t taskWebServer_hamdle;			  // Web服務器任務
TaskHandle_t taskNetwork_hamdle;			  // 網絡任務
TaskHandle_t taskModeManagement_hamdle;		  // 模式管理任務
TaskHandle_t taskUART0Read_hamdle;			  // UART0讀取任務

/* 定時器參照 */
TimerHandle_t wifiReconnectTimer; // WiFi重連定時器

void setup()
{
	/* 系統資訊列印 */
	ESP_LOGI("System", "========================================");
	ESP_LOGI("System", "3rd Eyes System Starting...");
	ESP_LOGI("System", "Version: %s", VERSION);
	ESP_LOGI("System", "========================================");
	ESP_LOGI("System", "=== ESP32-S3 System Information ===");
	ESP_LOGI("System", "SDK Version: %s", ESP.getSdkVersion());
	ESP_LOGI("System", "Chip Model: %s", ESP.getChipModel());
	ESP_LOGI("System", "Chip Revision: %d", ESP.getChipRevision());
	ESP_LOGI("System", "CPU Frequency: %d MHz", ESP.getCpuFreqMHz());
	ESP_LOGI("System", "Flash Size: %d MB", ESP.getFlashChipSize() / 1024 / 1024);
	ESP_LOGI("System", "Flash Speed: %d MHz", ESP.getFlashChipSpeed() / 1000000);
	const char *flashModeStr;
	switch (ESP.getFlashChipMode())
	{
		case 0:
			flashModeStr = "QIO";
			break;
		case 1:
			flashModeStr = "QOUT";
			break;
		case 2:
			flashModeStr = "DIO";
			break;
		case 3:
			flashModeStr = "DOUT";
			break;
		default:
			flashModeStr = "Unknown";
			break;
	}
	ESP_LOGI("System", "Flash Mode: %s", flashModeStr);
	if (psramFound())
	{
		ESP_LOGI("System", "PSRAM Size: %d MB", ESP.getPsramSize() / 1024 / 1024);
	}
	else
	{
		ESP_LOGI("System", "PSRAM Size: PSRAM Not Enabled");
	}
	ESP_LOGI("System", "========================================");

	while (xTaskGetTickCount() < 2000)
	{
		; // 等待2秒
	}

	ESP_LOGI("UART", "UART0 init...");
	prefs.begin("preferences", false);

	/* 佇列建立 */
	ESP_LOGI("quene", "quene create..."); // 打印佇列建立狀態

	queueCreate(&wit_data_quene, 10, sizeof(witData));							 // witData結構體佇列
	queueCreate(&wit_data_relative_angle_quene, 10, sizeof(witPProcessingData)); // witPProcessingData結構體佇列
	queueCreate(&eyesmove_data_quene, 10, sizeof(eyesMove_data));				 // eyesMove_data結構體佇列
	queueCreate(&gc9a01_data_quene, 10, sizeof(GC9A01_data));					 // GC9A01_data結構體佇列
	queueCreate(&wifiUpdate_data_quene, 10, sizeof(uint8_t));					 // WiFi更新佇列
	queueCreate(&correction_timer_update_quene, 10, sizeof(uint8_t));			 // 角度重置時間器更新佇列
	queueCreate(&mode_data_quene, 10, sizeof(int8_t));							 // 模式數據佇列
	queueCreate(&network_control_data_quene, 10, sizeof(network_control_data));	 // 網絡控制佇列
	queueCreate(&network_control_speed_data_quene, 10, sizeof(double));			 // 網絡控制速度佇列

	ESP_LOGI("quene", "quene create success"); // 打印佇列建立成功狀態

	/* 定時器建立 */
	wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(15000), pdFALSE, (void *)0, TimerReconnectWiFi); // WiFi重連定時器

	/* 任務建立 */
	xTaskCreatePinnedToCore(taskUART0Read, "taskUART0Read", 4096, NULL, 3, &taskUART0Read_hamdle, 1);				 // 創建UART0讀取任務
	xTaskCreatePinnedToCore(taskNetwork, "taskNetwork", 8192, NULL, 1, &taskNetwork_hamdle, 0);						 // 創建網絡任務
	xTaskCreatePinnedToCore(taskGC9A01, "taskGC9A01", 8192, NULL, 1, &taskGC9A01_hamdle, 1);						 // 創建GC9A01任務
	xTaskCreatePinnedToCore(taskEyesMove, "taskEyesMove", 4096, NULL, 1, &taskEyesMove_hamdle, 1);					 // 創建眼睛移動任務
	xTaskCreatePinnedToCore(taskModeManagement, "taskModeManagement", 4096, NULL, 2, &taskModeManagement_hamdle, 1); // 創建模式管理任務
}

void loop() { vTaskDelay(1000); }

/** 事件相關函數 **/
void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info)
{
	switch (event)
	{
		/* wifi部分 */
		case ARDUINO_EVENT_WIFI_STA_GOT_IP: // WiFi連接成功
			ESP_LOGI("wifi", "Connected to '%s' with IP %s", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
			break;
		case ARDUINO_EVENT_WIFI_STA_DISCONNECTED: // WiFi斷開
			switch (info.wifi_sta_disconnected.reason)
			{
				case WIFI_REASON_NO_AP_FOUND: // 無法找到AP
					ESP_LOGW("wifi", "reconnecting in 15 seconds...");
					xTimerStart(wifiReconnectTimer, 0);
					break;
				case WIFI_REASON_AUTH_FAIL: // 認證失敗
					break;
				case WIFI_REASON_802_1X_AUTH_FAILED: // 802.1x認證失敗
					break;
				case WIFI_REASON_HANDSHAKE_TIMEOUT: // 握手超時
					ESP_LOGW("wifi", "reconnecting in 15 seconds...");
					xTimerStart(wifiReconnectTimer, 0);
					break;
				case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT: // 4路握手超時
					ESP_LOGW("wifi", "reconnecting in 15 seconds...");
					xTimerStart(wifiReconnectTimer, 0);
					break;
				case WIFI_REASON_BEACON_TIMEOUT: // 信標超時
					ESP_LOGW("wifi", "reconnecting in 15 seconds...");
					xTimerStart(wifiReconnectTimer, 0);
					break;
				case WIFI_REASON_ASSOC_EXPIRE: // 協會過期
					ESP_LOGW("wifi", "reconnecting in 15 seconds...");
					xTimerStart(wifiReconnectTimer, 0);
					break;
				case WIFI_REASON_AUTH_EXPIRE: // 認證過期
					break;
				case WIFI_REASON_ASSOC_LEAVE: // 協會離開
					break;
				default:
					ESP_LOGW("wifi", "Disconnected from WiFi for reason: %d", info.wifi_sta_disconnected.reason);

					break;
			}
			break;
		case ARDUINO_EVENT_WIFI_STA_STOP: // WiFi停止
			ESP_LOGI("wifi", "WiFi stopped");
			break;
		case ARDUINO_EVENT_WIFI_AP_START:
			ESP_LOGI("wifi", "SoftAP \'%s\' Started with IP %s", WiFi.softAPSSID().c_str(), WiFi.softAPIP().toString().c_str());
			break;
		case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
			ESP_LOGI("wifi", "SoftAP station connected with MAC: %s, AID: %d", macToString(info.wifi_ap_staconnected.mac).c_str(),
					 info.wifi_ap_staconnected.aid);
			break;
		case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
			ESP_LOGI("wifi", "SoftAP station disconnected with MAC: %s, AID: %d", macToString(info.wifi_ap_stadisconnected.mac).c_str(),
					 info.wifi_ap_stadisconnected.aid);
			break;
		case ARDUINO_EVENT_WIFI_AP_STOP:
			ESP_LOGI("wifi", "SoftAP stopped");
			break;
		default:
			break;
	}
}

/** 回調相關函數 **/
/* WiFi重連定時器回調 */
void TimerReconnectWiFi(TimerHandle_t xTimer)
{
	if (WiFi.status() != WL_CONNECTED)
	{
		ESP_LOGI("wifi", "Attempting reconnect...");
		WiFi.begin(prefs.getString("wifi_ssid", "").c_str(), prefs.getString("wifi_password", "").c_str());
	}
}

/** 任務相關函數 **/
/* 網絡相關任務 */
void taskNetwork(void *pvParameters)
{
	uint8_t is_wifiUpdate = 1; // WiFi更新標誌

	/* WiFi初始化 */
	WiFi.onEvent(WiFiEvent); // 註冊事件回呼
	WiFi.setAutoReconnect(false);
	WiFi.mode(WIFI_AP_STA);
	WiFi.softAPdisconnect();
	if (prefs.getString("password", "").length() > 0)
	{
		if (!WiFi.softAP(prefs.getString("ssid", DEFAULT_SSID), prefs.getString("password", "")))
		{
			ESP_LOGE("wifi", "Failed to start SoftAP");
			vTaskDelete(NULL);
			return;
		}
	}
	else
	{
		if (!WiFi.softAP(prefs.getString("ssid", DEFAULT_SSID)))
		{
			ESP_LOGE("wifi", "Failed to start SoftAP");
			vTaskDelete(NULL);
			return;
		}
	}

	/* 伺服器任務創建 */
	xTaskCreatePinnedToCore(taskWebServer, "taskWebServer", 4096, NULL, 1, &taskWebServer_hamdle, 0); // 創建網絡任務

	while (1)
	{

		/* wifi更新 */
		if (is_wifiUpdate == 1) // 如果WiFi需要更新
		{
			is_wifiUpdate = 0;				// 重置WiFi更新標誌
			if (xTaskGetTickCount() > 5000) // 判斷初始化還是更新
			{
				ESP_LOGI("wifi", "Updating WiFi connection...");
			}
			if (prefs.getBool("iswifi", false) && prefs.getString("wifi_ssid", "").length() > 0 && prefs.getString("wifi_password", "").length() > 0)
			{

				/* 斷開當前wifi */
				if (WiFi.status() == WL_CONNECTED) // 如果WiFi已經連接
				{
					vTaskDelay(500);			  // 讓前端可以收到返回消息
					WiFi.disconnect(false, true); // 斷開WiFi連接
				}

				/* 連接新的wifi */
				WiFi.begin(prefs.getString("wifi_ssid", "").c_str(), prefs.getString("wifi_password", "").c_str()); // 連接WiFi
			}
			else
			{
				/* 停止WiFi */
				vTaskDelay(500);	   // 讓前端可以收到返回消息
				WiFi.disconnect(true); // 斷開WiFi連接
			}
		}

		/* softAP更新 */
		else if (is_wifiUpdate == 2)
		{
			is_wifiUpdate = 0; // 重置WiFi更新標誌
			ESP_LOGI("wifi", "Updating SoftAP configuration...");

			/* 斷開softAP */
			vTaskDelay(500);
			WiFi.softAPdisconnect();

			/* 重新啓動SoftAP */
			if (prefs.getString("password", "").length() > 0)
			{
				if (!WiFi.softAP(prefs.getString("ssid", DEFAULT_SSID), prefs.getString("password", "")))
				{
					ESP_LOGE("wifi", "Failed to start SoftAP");
					return;
				}
			}
			else
			{
				if (!WiFi.softAP(prefs.getString("ssid", DEFAULT_SSID)))
				{
					ESP_LOGE("wifi", "Failed to start SoftAP");
					return;
				}
			}
		}
		xQueueReceive(wifiUpdate_data_quene, &is_wifiUpdate, portMAX_DELAY); // 等待WiFi更新信號
	}
}

/* 伺服器相關任務 */
void taskWebServer(void *pvParameters)
{

	/* LittleFS啓動 */
	ESP_LOGI("LittleFS", "Mounting LittleFS...");
	if (!LittleFS.begin(false, "/littlefs", 10, "littlefs"))
	{
		ESP_LOGW("LittleFS", "LittleFS mount failed, formatting...");
		if (!LittleFS.begin(true, "/littlefs", 10, "littlefs"))
		{
			ESP_LOGE("LittleFS", "LittleFS format failed!");
			vTaskDelete(NULL);
			return;
		}
		ESP_LOGI("LittleFS", "LittleFS formatted successfully");
	}
	ESP_LOGI("LittleFS", "LittleFS mounted successfully");

	// 顯示檔案系統資訊
	ESP_LOGI("LittleFS", "Total: %d bytes, Used: %d bytes", LittleFS.totalBytes(), LittleFS.usedBytes());

	/* mDNS啓動 */
	if (!MDNS.begin("3rdeyes"))
	{
		ESP_LOGE("mDNS", "mDNS failed to start!");
		vTaskDelete(NULL);
	}
	ESP_LOGI("mDNS", "mDNS started: http://3rdeyes.local");

	/* WebSocket服務器 */
	ws.onEvent(onSocketEvent); // 設置WebSocket事件處理函數

	/* 設置Web服務器路由 */
	server.addHandler(&ws); // 將WebSocket服務器添加到Web服務器
	server.serveStatic("/", LittleFS, "/www/").setDefaultFile("index.html");
	server.onNotFound(handleNotFound);
	server.on("/", HTTP_GET, handleRoot);
	server.on("/api/wifi_config", HTTP_GET, api_wifi_config);
	server.on("/api/set_wifi_config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, api_set_wifi_config);
	server.on("/api/softap_config", HTTP_GET, api_softAP_config);
	server.on("/api/set_softap_config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, api_set_softAP_config);
	server.on("/api/mode_config", HTTP_GET, api_mode_config);
	server.on("/api/set_mode_config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, api_set_mode_config);
	server.on("/api/advanced_config", HTTP_GET, api_advanced_config);
	server.on("/api/set_advanced_config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, api_set_advanced_config);

	/* 伺服器啓動 */
	server.begin();
	ESP_LOGI("server", "HTTP server started");
	while (1)
	{
		ws.cleanupClients();
		vTaskDelay(1000);
	}
}

/* 模式管理任務 */
void taskModeManagement(void *pvParameters)
{
	int8_t mode = prefs.getInt("mode", 0); // 獲取當前模式
	uint8_t task_register = 0;			   // 任務寄存器
	ESP_LOGI("mode", "Current mode: %d", mode);
	/* 高 <-------> 低
	|  |  |  |  |  |  | 陀螺儀跟蹤任務 | 獲取數據任務 | */
	while (1)
	{

		/* 設置任務寄存器 */
		switch (mode)
		{
			case GYROSCOPE_TRACKS_MODE: // 陀螺儀跟蹤模式
				task_register = 0x03;
				break;
			case NETWORK_CONTROL_MODE: // 網絡控制模式
				task_register = 0x04;
				break;
			default:
				task_register = 0x00;
				break;
		}

		/* 切換任務模式 */
		if (task_register & 0x01) // 獲取數據任務
		{
			xTaskCreatePinnedToCore(taskWitGetData, "taskWitEyesGetData", 4096, &witEyes, 1, &taskWitEyesGetData_hamdle, 1); // 創建獲取數據任務
			xTaskCreatePinnedToCore(taskWitGetData, "taskWitHeadGetData", 4096, &witHead, 1, &taskWitHeadGetData_hamdle, 1); // 創建獲取數據任務
			xTaskCreatePinnedToCore(taskWitPProcessingData, "taskWitPProcessingData", 4096, NULL, 1, &taskWitPProcessingData_hamdle,
									1); // 創建數據處理任務
		}
		else
		{
			TaskDeleteSafe(&taskWitEyesGetData_hamdle);		// 刪除獲取數據任務
			TaskDeleteSafe(&taskWitHeadGetData_hamdle);		// 刪除獲取數據任務
			TaskDeleteSafe(&taskWitPProcessingData_hamdle); // 刪除數據處理任務
		}
		if (task_register & 0x02) // 陀螺儀跟蹤任務
		{
			xTaskCreatePinnedToCore(taskGyroscopeTracking, "taskGyroscopeTracking", 4096, NULL, 1, &taskGyroscopeTracking_hamdle,
									1); // 創建陀螺儀跟蹤任務
		}
		else
		{
			TaskDeleteSafe(&taskGyroscopeTracking_hamdle); // 刪除陀螺儀跟蹤任務
		}
		if (task_register & 0x04) // 網絡控制任務
		{
			xTaskCreatePinnedToCore(taskNetworkControl, "taskNetworkControl", 4096, NULL, 1, &taskNetworkControl_hamdle, 1); // 創建網絡控制任務
			xTaskCreatePinnedToCore(taskNetworkControlGC9A01, "taskNetworkControlGC9A01", 4096, NULL, 1, &taskNetworkControlGC9A01_hamdle,
									1); // 創建網絡控制屏幕任務
		}
		else
		{
			TaskDeleteSafe(&taskNetworkControl_hamdle);		  // 刪除網絡控制任務
			TaskDeleteSafe(&taskNetworkControlGC9A01_hamdle); // 刪除網絡控制屏幕任務
		}

		/* 等待模式數據 */
		xQueueReceive(mode_data_quene, &mode, portMAX_DELAY);
		ESP_LOGI("mode", "Current mode change: %d", mode);
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
	vTaskSuspendAll();
	switch (wit_status)
	{
		case 0:
			ESP_LOGI("wit", "%s init success", wit_name.c_str()); // 打印初始化成功
			break;
		case WIT_INIT_ERROR:
			ESP_LOGE("wit", "%s init error", wit_name.c_str()); // 打印初始化錯誤
			break;
		case SERIAL_INIT_ERROR:
			ESP_LOGE("wit", "%s serial init error", wit_name.c_str()); // 打印Serial初始化錯誤
			break;
		default:
			ESP_LOGE("wit", "%s init unknown error", wit_name.c_str()); // 打印未知錯誤
			break;
	}
	xTaskResumeAll();
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

/* 處理角度數據任務 */
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

	double calculate_angularSpeed; // 角速度絕對值

	uint8_t read_count = 0; // 讀取計數

	uint16_t reset_reference_timer = prefs.getUInt("correction", 2000); // 重置参考角度時間器
	uint64_t reset_reference_time = 0;									// 重置参考角度時間
	uint8_t is_reset_reference_timer_update = 0;						// 重置参考角度時間器更新標誌

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
			relative_quaternion = IMUAngle::quaternion_multiply(witEyes_quaternion,
																IMUAngle::quaternion_conjugate(witHead_quaternion)); // 計算眼睛和頭部的四元數差值
			relative_angle = IMUAngle::quaternion_to_euler(relative_quaternion);									 // 計算眼睛和頭部的角度差值

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

		/* 更新重置時間 */
		if (xQueueReceive(correction_timer_update_quene, &is_reset_reference_timer_update, 0) == pdTRUE) // 等待時間更新信號
		{
			if (is_reset_reference_timer_update == 1)
			{
				reset_reference_timer = prefs.getUInt("correction", 2000); // 更新重置参考角度時間器
				reset_reference_time = xTaskGetTickCount();				   // 更新重置参考角度時間
				is_reset_reference_timer_update = 0;					   // 重置重置参考角度時間器更新標誌
			}
		}

		/* 重置参考角度 */
		if (reset_reference_timer != 0)
		{
			uint64_t current_time = xTaskGetTickCount(); // 獲取當前時間
			calculate_angularSpeed = sqrt(pow(relative_angularSpeed.xangular_speed, 2) + pow(relative_angularSpeed.yangular_speed, 2) +
										  pow(relative_angularSpeed.zangular_speed, 2)); // 計算角速度絕對值
			if (calculate_angularSpeed > 30 || !witEyes_angle_status || !witHead_angle_status)
			{
				reset_reference_time = current_time; // 更新重置参考角度時間
			}
			if (current_time - reset_reference_time > reset_reference_timer && witEyes_angle_status && witHead_angle_status)
			{
				witEyes_angle_status = 0; // 重置眼睛角度狀態
				witHead_angle_status = 0; // 重置頭部角度狀態
			}
		}
	}
}

/* 陀螺儀跟蹤任務 */
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

/* 網絡控制任務 */
void taskNetworkControl(void *arg)
{
	network_control_data data_get;		  // 網絡控制數據接收
	eyesMove_data data_send;			  // 眼睛移動數據發送
	int8_t x = 0, y = 0;				  // 眼睛角度和上次眼睛角度
	int8_t last_x = 0, last_y = 0;		  // 上次眼睛角度
	int8_t send_x = 0, send_y = 0;		  // 發送眼睛角度
	double speed_x = 0, speed_y = 0;	  // 眼睛速度
	double velocity = 0;				  // 眼睛速度絕對值
	double eyelid_angle = 45;			  // 眼睛張開角度
	uint8_t eyelid_angle_int = 45;		  // 眼睛張開角度整數
	uint8_t send_eyelid_angle = 45;		  // 眼睛張開目標角度
	uint64_t move_last_time = 0;		  // 眼睛移動上次時間
	uint64_t eyelid_last_time = 0;		  // 眼皮角度移動上次時間
	uint8_t type = 0;					  // 數據類型
	TickType_t wait_time = portMAX_DELAY; // 等待時間

	/* 初始化數據 */
	data_get.type = 0;
	data_get.x = 0;
	data_get.y = 0;

	/* 清除佇列數據 */
	for (uint8_t i = 0; i < 10; i++)
	{
		xQueueReceive(network_control_data_quene, &data_get, 0);
	}

	ESP_LOGI("network control", "Network control task started");
	while (1)
	{
		/* 獲取網絡控制數據 */
		if (xQueueReceive(network_control_data_quene, &data_get, wait_time) == pdPASS)
		{
			type = data_get.type;
		}

		/* 處理眼皮角度移動 */
		if (type == 1)
		{
			/* 設置等待時間*/
			wait_time = pdMS_TO_TICKS(10);

			/* 獲取資料 */
			y = data_get.y;

			/* */
			if (y == 0)
			{
				wait_time = portMAX_DELAY;
			}

			/* 獲取時間差 */
			uint64_t new_time = xTaskGetTickCount();
			uint64_t diff_time = new_time - eyelid_last_time;
			eyelid_last_time = new_time;
			if (diff_time == 0)
			{
				diff_time = 1;
			}

			/* 計算眼皮角度變化 */
			eyelid_angle = eyelid_angle + (double)y * 100.0 / (1000.0 * (double)diff_time);
			eyelid_angle = constrain(eyelid_angle, 0.0, 80.0);
			eyelid_angle_int = round((uint8_t)eyelid_angle);

			/* 設置眼皮角度 */
			send_eyelid_angle = eyelid_angle_int;
		}

		/* 處理眼睛移動 */
		if (type == 0)
		{
			/* 設置等待時間*/
			wait_time = portMAX_DELAY;

			/* 計算速度 */
			x = data_get.x;
			y = data_get.y;

			uint64_t new_time = xTaskGetTickCount();
			uint64_t diff_time = new_time - move_last_time;
			move_last_time = new_time;
			if (diff_time == 0)
			{
				diff_time = 1;
			}

			int16_t delta_x = x - last_x;
			int16_t delta_y = y - last_y;
			last_x = x;
			last_y = y;

			speed_x = (double)delta_x / (double)diff_time * 1000.0;
			speed_y = (double)delta_y / (double)diff_time * 1000.0;
			velocity = sqrt(speed_x * speed_x + speed_y * speed_y);
			xQueueSend(network_control_speed_data_quene, &velocity, 0);

			/* 映射眼睛角度 */
			x = map(data_get.x, -100, 100, -55, 55);
			y = map(data_get.y, -100, 100, -35, 35);

			/* 設置眼睛角度 */
			send_x = x;
			send_y = y;
		}

		/* 傳遞數據 */
		data_send.x_angle = send_x;
		data_send.y_angle = send_y;
		data_send.eyelid_angle = send_eyelid_angle;
		xQueueSend(eyesmove_data_quene, &data_send, 0);
	}
}

/* 網絡控制屏幕任務 */
void taskNetworkControlGC9A01(void *arg)
{
	double velocity;
	int32_t velocity_sum = 0;
	int32_t velocity_array[5] = {0}; // 速度數組
	uint8_t velocity_index = 0;		 // 速度數組索引
	GC9A01_data gc9a01_data;		 // GC9A01數據結構體
	while (1)
	{
		velocity = 0;
		xQueueReceive(network_control_speed_data_quene, &velocity, 150); // 獲取速度
		velocity_array[velocity_index] = (int32_t)velocity;				 // 存儲速度
		velocity_index = (velocity_index + 1) % 5;						 // 更新索引

		/* 計算平均速度 */
		velocity_sum = 0;
		for (uint8_t i = 0; i < 5; i++)
		{
			velocity_sum += velocity_array[i];
		}

		/* 判斷瞳孔大小 */
		if (velocity_sum > 7500)
		{
			gc9a01_data.R = 65;				  // 設置GC9A01半徑
			gc9a01_data.zetaR = 0.9;		  // 設置GC9A01阻尼比
			gc9a01_data.omega_nR = 20;		  // 設置GC9A01自然頻率
			gc9a01_data.lightMax = 250;		  // 設置GC9A01光暈最大值
			gc9a01_data.zetaLightMax = 0.9;	  // 設置GC9A01光暈阻尼比
			gc9a01_data.omega_nLightMax = 20; // 設置GC9A01光暈自然頻率
		}
		else if (velocity_sum > 1000)
		{
			gc9a01_data.R = 73;				  // 設置GC9A01半徑
			gc9a01_data.zetaR = 0.9;		  // 設置GC9A01阻尼比
			gc9a01_data.omega_nR = 10;		  // 設置GC9A01自然頻率
			gc9a01_data.lightMax = 200;		  // 設置GC9A01光暈最大值
			gc9a01_data.zetaLightMax = 0.9;	  // 設置GC9A01光暈阻尼比
			gc9a01_data.omega_nLightMax = 10; // 設置GC9A01光暈自然頻率
		}
		else
		{
			gc9a01_data.R = 80;				 // 設置GC9A01半徑
			gc9a01_data.zetaR = 1;			 // 設置GC9A01阻尼比
			gc9a01_data.omega_nR = 5;		 // 設置GC9A01自然頻率
			gc9a01_data.lightMax = 150;		 // 設置GC9A01光暈最大值
			gc9a01_data.zetaLightMax = 1;	 // 設置GC9A01光暈阻尼比
			gc9a01_data.omega_nLightMax = 5; // 設置GC9A01光暈自然頻率
		}
		xQueueSend(gc9a01_data_quene, &gc9a01_data, 0); // 傳遞GC9A01數據
	}
}

/* 畫眼睛任務 */
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

/* 眼睛移動任務 */
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
				eyesmove.eyesMove_angle_set(data_get.eyelid_angle, data_get.x_angle, data_get.y_angle); // 設置眼睛角度
			}
		}
		is_eyesmove_update_finish = eyesmove.eyesMove_update(); // 更新眼睛
		vTaskDelay(1);
	}
}

/* UART0讀取任務 */
void taskUART0Read(void *arg)
{
	const uint16_t BUF_SIZE = 1024;

	uart_event_t event;
	uint8_t *data = (uint8_t *)malloc(BUF_SIZE);

	/* 配置UART0 */
	uart_config_t cfg = {
		.baud_rate = 115200,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_APB,
	};
	ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &cfg));
	ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, 43, 44, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
	ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart0_queue, 0));

	while (true)
	{
		/* 接收指令 */
		if (xQueueReceive(uart0_queue, &event, portMAX_DELAY))
		{
			if (event.type == UART_DATA)
			{
				uint16_t len = uart_read_bytes(UART_NUM_0, data, event.size, portMAX_DELAY);
				if (len > 0)
				{
					data[len] = '\0';
					String data_str = String((char *)data);
					data_str.trim();		// 去除空格
					data_str.toLowerCase(); // 轉換為小寫

					/* 執行指令 */
					if (data_str == "reset") // 重啓指令
					{
						ESP_LOGI("UART", "Reset command received. Restarting...");
						ESP.restart();
					}
					else if (data_str == "help") // 幫助指令
					{
						ESP_LOGI("UART", "\nAvailable commands:\nreset - Restart the device\nhelp - Show this help message");
					}
					else
					{
						ESP_LOGE("UART", "Unknown command, type \'help\' for a list of commands.");
					}
				}

				/* 清除緩存 */
				else if (event.type == UART_FIFO_OVF || event.type == UART_BUFFER_FULL)
				{
					uart_flush_input(UART_NUM_0);
					xQueueReset(uart0_queue);
					ESP_LOGW("UART", "UART0 buffer overflow");
				}
			}
		}
	}
}

void queueCreate(QueueHandle_t *quene, uint8_t queneSize, uint8_t queneType)
{
	/* 佇列建立 */
	*quene = xQueueCreate(queneSize, queneType);

	/* 佇列建立失敗 */
	if (*quene == NULL)
	{
		ESP_LOGE("quene", "quene create error");
		while (2)
		{
			vTaskDelay(1000);
		}
	}
}

void TaskDeleteSafe(TaskHandle_t *pHandle, uint32_t yieldMs)
{
	/* 檢查指標 */
	if (pHandle == NULL)
	{
		return;
	}
	TaskHandle_t h = *pHandle;
	if (h == NULL)
	{
		return;
	}

	/* 先讓出 CPU，降低和目標任務的即時競態 */
	if (yieldMs > 0)
	{
		vTaskDelay(pdMS_TO_TICKS(yieldMs));
	}

	/* 檢查是否已無效/已刪除 */
#if (INCLUDE_eTaskGetState == 1)
	eTaskState s = eTaskGetState(h);
	if (s == eDeleted || s == eInvalid)
	{
		*pHandle = NULL; // 清空以反映真實狀態
		return;
	}
#endif

	/* 自刪任務情景 */
	if (xTaskGetCurrentTaskHandle() == h)
	{
		*pHandle = NULL;   // 先清掉全域/外部可見的 handle
		vTaskDelete(NULL); // 這行之後不會再回來
		return;
	}

	/* 從其他任務刪除目標任務 */
	vTaskDelete(h);
	*pHandle = NULL; // 避免懸掛指標
}

String macToString(const uint8_t mac[6])
{
	char buf[18];
	snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return String(buf);
}
