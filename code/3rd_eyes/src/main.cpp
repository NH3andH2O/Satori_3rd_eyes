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
/* 第三方程式庫 */
#include <Arduino.h>
#include <ESP32Servo.h>
#include <LovyanGFX.hpp>
#include <Config.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <LittleFS.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <driver/uart.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <freertos/event_groups.h>
#include <atomic>
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

#define SERVO_SET_MODE 0		// 伺服馬達調試模式
#define GYROSCOPE_TRACKS_MODE 1 // 陀螺儀跟蹤模式
#define NETWORK_CONTROL_MODE 2	// 網絡控制模式

// 位元遮罩用於保留不同模式共用的任務
static constexpr uint8_t TASK_MASK_WIT = 0x01;
static constexpr uint8_t TASK_MASK_GYROSCOPE = 0x02;
static constexpr uint8_t TASK_MASK_NETWORK = 0x04;
static constexpr uint8_t TASK_MASK_SERVO_SET = 0x08;
static constexpr uint8_t TASK_MASK_EYES_MOVE = 0x10;
// WIT 任務僅在關閉 UART 後回報停止
static constexpr EventBits_t WIT_EYES_STOPPED_BIT = BIT0;
static constexpr EventBits_t WIT_HEAD_STOPPED_BIT = BIT1;
static constexpr uint32_t WIT_STOP_TIMEOUT_MS = 1500;

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
void taskServoSet(void *arg);				 // 伺服馬達調試任務

/* 其他函數宣告 */
void queueCreate(QueueHandle_t *quene, uint8_t queneSize, uint8_t queneType); // 佇列創建
void TaskDeleteSafe(TaskHandle_t *pHandle, uint32_t yieldMs = 0);
bool TaskCreateSafe(TaskFunction_t task, const char *name, uint32_t stackSize, void *parameter, UBaseType_t priority, TaskHandle_t *handle,
					BaseType_t core);
bool GetModeTaskMask(int8_t mode, uint8_t *taskMask);
bool StopWitTasks();
void ResetModeQueues();
void StopAllModeTasks();
bool ConnectMeshWiFi(const AppConfig::WiFiConfig &wifiConfig);
bool StartSoftAP(const AppConfig::SoftAPConfig &softAPConfig);
void ScheduleWiFiReconnect(uint8_t reason);
String macToString(const uint8_t mac[6]); // MAC地址轉字符串

/* 結構體宣告 */
eyesMove eyesmove(UPPER_EYELID_PIN, LOWER_EYELID_PIN, EYEBALL_PIN);

wit witEyes(SERIAL1, witEyes_RX_PIN, witEyes_TX_PIN, 115200, 6, 1); // wit眼睛模組
wit witHead(SERIAL2, witHead_RX_PIN, witHead_TX_PIN, 115200, 9, 0); // wit頭部模組

GC9A01 gc9a01(GC9A01_SDA_PIN, GC9A01_SCL_PIN, GC9A01_CS_PIN, GC9A01_DC_PIN, GC9A01_RST_PIN, GC9A01_BLK_PIN); // GC9A01實例

AppConfig::Config config; // 配置實例

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
QueueHandle_t wit_advanced_config_update_quene; // 每個consumer必須使用獨立佇列
QueueHandle_t gyroscope_advanced_config_update_quene;
QueueHandle_t mode_data_quene;					// 宣告模式數據佇列
QueueHandle_t uart0_queue;						// UART0事件佇列
QueueHandle_t network_control_data_quene;		// 網絡數據佇列
QueueHandle_t network_control_speed_data_quene; // 網絡控制速度數據佇列
QueueHandle_t servoSet_data_quene;				// 伺服馬達調試數據佇列
EventGroupHandle_t witStopEventGroup = nullptr; // WIT 安全停止回報

/* 任務參照 */
TaskHandle_t taskWitEyesGetData_handle = nullptr;		// 獲取wit眼睛數據任務
TaskHandle_t taskWitHeadGetData_handle = nullptr;		// 獲取wit頭部數據任務
TaskHandle_t taskWitPProcessingData_handle = nullptr;	// 處理wit數據任務
TaskHandle_t taskGyroscopeTracking_handle = nullptr;	// 陀螺儀跟蹤任務
TaskHandle_t taskNetworkControl_handle = nullptr;		// 網絡控制任務
TaskHandle_t taskNetworkControlGC9A01_handle = nullptr; // 網絡控制屏幕任務
TaskHandle_t taskGC9A01_handle = nullptr;				// GC9A01任務
TaskHandle_t taskEyesMove_handle = nullptr;				// 眼睛任務
TaskHandle_t taskWebServer_handle = nullptr;			// Web服務器任務
TaskHandle_t taskNetwork_handle = nullptr;				// 網絡任務
TaskHandle_t taskModeManagement_handle = nullptr;		// 模式管理任務
TaskHandle_t taskUART0Read_handle = nullptr;			// UART0讀取任務
TaskHandle_t taskServoSet_handle = nullptr;				// 伺服馬達調試任務
/* 定時器參照 */
TimerHandle_t wifiReconnectTimer; // WiFi重連定時器

static constexpr uint32_t WIFI_RECONNECT_DELAYS_MS[] = {5000, 15000, 30000, 60000};
static constexpr uint32_t WIFI_RECONNECT_JITTER_MS = 1000;
static std::atomic_bool wifiReconnectAllowed{false};
static std::atomic_uint32_t wifiReconnectAttempt{0};

void setup()
{
	uint8_t log_level = config.getLogConfig();
	if (log_level < ESP_LOG_ERROR || log_level > ESP_LOG_VERBOSE)
	{
		log_level = ESP_LOG_INFO;
		config.setLogConfig(log_level);
	}
	esp_log_level_set("*", static_cast<esp_log_level_t>(log_level));

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

	/* 佇列建立 */
	ESP_LOGI("quene", "quene create..."); // 打印佇列建立狀態

	queueCreate(&wit_data_quene, 10, sizeof(witData));							 // witData結構體佇列
	queueCreate(&wit_data_relative_angle_quene, 10, sizeof(witPProcessingData)); // witPProcessingData結構體佇列
	queueCreate(&eyesmove_data_quene, 10, sizeof(eyesMove_data));				 // eyesMove_data結構體佇列
	queueCreate(&gc9a01_data_quene, 10, sizeof(GC9A01_data));					 // GC9A01_data結構體佇列
	queueCreate(&wifiUpdate_data_quene, 10, sizeof(NetworkCommand));			 // WiFi更新佇列
	// 設定更新只保留最新值
	queueCreate(&wit_advanced_config_update_quene, 1, sizeof(AppConfig::AdvancedConfig));
	queueCreate(&gyroscope_advanced_config_update_quene, 1, sizeof(AppConfig::AdvancedConfig));
	queueCreate(&mode_data_quene, 1, sizeof(int8_t));							// 模式數據佇列
	queueCreate(&network_control_data_quene, 10, sizeof(network_control_data)); // 網絡控制佇列
	queueCreate(&network_control_speed_data_quene, 10, sizeof(double));			// 網絡控制速度佇列
	queueCreate(&servoSet_data_quene, 10, sizeof(servoSet_data));				// 伺服馬達數據佇列

	ESP_LOGI("quene", "quene create success"); // 打印佇列建立成功狀態

	witStopEventGroup = xEventGroupCreate();
	if (!witStopEventGroup)
	{
		ESP_LOGE("mode", "WIT stop event group create error");
		while (1)
		{
			vTaskDelay(1000);
		}
	}

	/* 定時器建立 */
	wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(15000), pdFALSE, (void *)0, TimerReconnectWiFi); // WiFi重連定時器

	/* 使能伺服電機 */
	eyesmove.eyesMove_init();
	gc9a01.GC9A01_init();
	gc9a01.GC9A01_setEyes_r(80, 1, 10);
}

void loop()
{
	static bool tasks_created = false;
	if (!tasks_created)
	{
		tasks_created = true;
		xTaskCreatePinnedToCore(taskUART0Read, "taskUART0Read", 4096, NULL, 3, &taskUART0Read_handle, 0);				 // 創建UART0讀取任務
		xTaskCreatePinnedToCore(taskNetwork, "taskNetwork", 8192, NULL, 1, &taskNetwork_handle, 0);						 // 創建網絡任務
		xTaskCreatePinnedToCore(taskGC9A01, "taskGC9A01", 8192, NULL, 1, &taskGC9A01_handle, 1);						 // 創建GC9A01任務
		xTaskCreatePinnedToCore(taskModeManagement, "taskModeManagement", 4096, NULL, 2, &taskModeManagement_handle, 1); // 創建模式管理任務
	}
	vTaskDelay(1000);
}

/** 事件相關函數 **/
void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info)
{
	switch (event)
	{
		case ARDUINO_EVENT_WIFI_STA_CONNECTED:
			ESP_LOGI("wifi", "STA connected: SSID='%.*s', BSSID=%s, channel=%u, auth=%d", info.wifi_sta_connected.ssid_len,
					 reinterpret_cast<const char *>(info.wifi_sta_connected.ssid), macToString(info.wifi_sta_connected.bssid).c_str(),
					 info.wifi_sta_connected.channel, info.wifi_sta_connected.authmode);
			break;
		case ARDUINO_EVENT_WIFI_STA_GOT_IP:
			xTimerStop(wifiReconnectTimer, 0);
			wifiReconnectAttempt.store(0);
			ESP_LOGI("wifi", "Connected to '%s': BSSID=%s, channel=%d, RSSI=%d dBm, IP=%s", WiFi.SSID().c_str(), WiFi.BSSIDstr().c_str(),
					 WiFi.channel(), WiFi.RSSI(), WiFi.localIP().toString().c_str());
			break;
		case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
			ESP_LOGW("wifi", "STA disconnected: BSSID=%s, reason=%u (%s)", macToString(info.wifi_sta_disconnected.bssid).c_str(),
					 info.wifi_sta_disconnected.reason,
					 WiFi.STA.disconnectReasonName(static_cast<wifi_err_reason_t>(info.wifi_sta_disconnected.reason)));
			ScheduleWiFiReconnect(info.wifi_sta_disconnected.reason);
			break;
		case ARDUINO_EVENT_WIFI_STA_STOP:
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
	if (wifiReconnectAllowed.load() && WiFi.status() != WL_CONNECTED)
	{
		// Wi-Fi 操作集中於 network task，避免跨 task 修改 driver 狀態。
		const NetworkCommand command = NetworkCommand::StaReconnect;
		if (xQueueSend(wifiUpdate_data_quene, &command, 0) != pdTRUE)
		{
			ESP_LOGE("wifi", "Failed to queue WiFi reconnect command");
		}
	}
}

void ScheduleWiFiReconnect(uint8_t reason)
{
	if (!wifiReconnectAllowed.load())
	{
		return;
	}

	const uint32_t attempt = wifiReconnectAttempt.fetch_add(1);
	const size_t delayCount = sizeof(WIFI_RECONNECT_DELAYS_MS) / sizeof(WIFI_RECONNECT_DELAYS_MS[0]);
	const size_t delayIndex = attempt < delayCount ? attempt : delayCount - 1;
	const uint32_t delayMs = WIFI_RECONNECT_DELAYS_MS[delayIndex] + esp_random() % (WIFI_RECONNECT_JITTER_MS + 1);

	ESP_LOGW("wifi", "Reconnect attempt %lu scheduled in %lu ms after reason %u", static_cast<unsigned long>(attempt + 1),
			 static_cast<unsigned long>(delayMs), reason);
	if (xTimerChangePeriod(wifiReconnectTimer, pdMS_TO_TICKS(delayMs), 0) != pdPASS)
	{
		ESP_LOGE("wifi", "Failed to schedule WiFi reconnect timer");
	}
}

bool ConnectMeshWiFi(const AppConfig::WiFiConfig &wifiConfig)
{
	if (!wifiConfig.is_enabled || wifiConfig.ssid.isEmpty())
	{
		return false;
	}

	WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
	WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
	const char *password = wifiConfig.password.isEmpty() ? nullptr : wifiConfig.password.c_str();

	if (WiFi.begin(wifiConfig.ssid.c_str(), password, 0, nullptr, false) == WL_CONNECT_FAILED)
	{
		ESP_LOGE("wifi", "Failed to configure STA credentials");
		return false;
	}

	wifi_config_t stationConfig{};
	esp_err_t error = esp_wifi_get_config(WIFI_IF_STA, &stationConfig);
	if (error != ESP_OK)
	{
		ESP_LOGE("wifi", "Failed to read STA configuration: %s", esp_err_to_name(error));
		return false;
	}

	// Mesh 選擇必須保留同一 SSID 下切換 BSSID 與頻道的自由。
	stationConfig.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
	stationConfig.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
	stationConfig.sta.bssid_set = false;
	stationConfig.sta.channel = 0;
	stationConfig.sta.failure_retry_cnt = 1;

#if defined(CONFIG_ESP_WIFI_11KV_SUPPORT) && CONFIG_ESP_WIFI_11KV_SUPPORT
	stationConfig.sta.rm_enabled = 1;
	stationConfig.sta.btm_enabled = 1;
	static bool capabilityLogged = false;
	if (!capabilityLogged)
	{
		ESP_LOGI("wifi", "802.11k/v roaming assistance enabled");
		capabilityLogged = true;
	}
#else
	static bool capabilityLogged = false;
	if (!capabilityLogged)
	{
		ESP_LOGI("wifi", "802.11k/v unavailable; using multi-BSSID scan fallback");
		capabilityLogged = true;
	}
#endif

	error = esp_wifi_set_config(WIFI_IF_STA, &stationConfig);
	if (error != ESP_OK)
	{
		ESP_LOGE("wifi", "Failed to apply STA configuration: %s", esp_err_to_name(error));
		return false;
	}

	wifiReconnectAllowed.store(true);
	error = esp_wifi_connect();
	if (error != ESP_OK)
	{
		ESP_LOGE("wifi", "Failed to start STA connection: %s", esp_err_to_name(error));
		ScheduleWiFiReconnect(WIFI_REASON_UNSPECIFIED);
		return false;
	}

	ESP_LOGI("wifi", "Connecting to '%s' with multi-BSSID selection", wifiConfig.ssid.c_str());
	return true;
}

bool StartSoftAP(const AppConfig::SoftAPConfig &softAPConfig)
{
	const bool started = softAPConfig.password.isEmpty() ? WiFi.softAP(softAPConfig.ssid.c_str())
														 : WiFi.softAP(softAPConfig.ssid.c_str(), softAPConfig.password.c_str());
	if (!started)
	{
		ESP_LOGE("wifi", "Failed to start SoftAP");
	}
	return started;
}

/** 任務相關函數 **/
/* 網絡相關任務 */
void taskNetwork(void *pvParameters)
{
	NetworkCommand networkCommand = NetworkCommand::StaConfigUpdate;

	/* WiFi初始化 */
	WiFi.onEvent(WiFiEvent); // 註冊事件回呼
	WiFi.setAutoReconnect(false);
	WiFi.setSleep(WIFI_PS_MIN_MODEM);
	// SoftAP 與 STA 共用頻道，STA 重連期間不得重啟 SoftAP。
	WiFi.mode(WIFI_AP_STA);
	if (!StartSoftAP(config.getSoftAPConfig()))
	{
		vTaskDelete(NULL);
		return;
	}

	if (WiFi.getSleep() == WIFI_PS_MIN_MODEM)
	{
		ESP_LOGI("wifi", "WiFi power save mode: MIN_MODEM");
	}
	else
	{
		ESP_LOGW("wifi", "Failed to enable WiFi power save");
	}

	/* 伺服器任務創建 */
	xTaskCreatePinnedToCore(taskWebServer, "taskWebServer", 4096, NULL, 1, &taskWebServer_handle, 0); // 創建網絡任務

	while (1)
	{

		if (networkCommand == NetworkCommand::StaConfigUpdate)
		{
			AppConfig::WiFiConfig wificonfig = config.getWiFiConfig(); // 獲取WiFi配置

			wifiReconnectAllowed.store(false);
			wifiReconnectAttempt.store(0);
			xTimerStop(wifiReconnectTimer, 0);
			if (xTaskGetTickCount() > 5000) // 判斷初始化還是更新
			{
				ESP_LOGI("wifi", "Updating WiFi connection...");
				vTaskDelay(500);
			}

			WiFi.disconnect(false, true, 1000);
			if (wificonfig.is_enabled && !wificonfig.ssid.isEmpty())
			{
				ConnectMeshWiFi(wificonfig);
			}
			else
			{
				ESP_LOGI("wifi", "STA connection disabled; SoftAP remains active");
			}
		}
		else if (networkCommand == NetworkCommand::SoftAPConfigUpdate)
		{
			AppConfig::SoftAPConfig softapconfig = config.getSoftAPConfig(); // 獲取SoftAP配置

			ESP_LOGI("wifi", "Updating SoftAP configuration...");
			vTaskDelay(500);
			WiFi.softAPdisconnect();
			StartSoftAP(softapconfig);
		}
		else if (networkCommand == NetworkCommand::StaReconnect)
		{
			const AppConfig::WiFiConfig wificonfig = config.getWiFiConfig();
			if (wifiReconnectAllowed.load() && WiFi.status() != WL_CONNECTED && wificonfig.is_enabled && !wificonfig.ssid.isEmpty())
			{
				ESP_LOGI("wifi", "Attempting reconnect...");
				ConnectMeshWiFi(wificonfig);
			}
		}
		xQueueReceive(wifiUpdate_data_quene, &networkCommand, portMAX_DELAY); // 等待WiFi更新信號
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

	File webVersionFile = LittleFS.open("/www/version.txt", FILE_READ);
	if (!webVersionFile)
	{
		ESP_LOGE("System", "Web version file is missing or could not be opened");
	}
	else
	{
		String webVersion = webVersionFile.readString();
		webVersionFile.close();
		webVersion.trim();

		if (webVersion.length() == 0)
		{
			ESP_LOGW("System", "Web version file is empty");
		}
		else
		{
			ESP_LOGI("System", "Web Version: %s", webVersion.c_str());
			if (webVersion != VERSION)
			{
				ESP_LOGW("System", "Version mismatch: Firmware=%s, Web=%s", VERSION, webVersion.c_str());
			}
		}
	}

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
	server.on("/api/version", HTTP_GET, api_version);
	server.on("/api/wifi_config", HTTP_GET, api_wifi_config);
	server.on("/api/set_wifi_config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, api_set_wifi_config);
	server.on("/api/softap_config", HTTP_GET, api_softAP_config);
	server.on("/api/set_softap_config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, api_set_softAP_config);
	server.on("/api/mode_config", HTTP_GET, api_mode_config);
	server.on("/api/set_mode_config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, api_set_mode_config);
	server.on("/api/advanced_config", HTTP_GET, api_advanced_config);
	server.on("/api/set_advanced_config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, api_set_advanced_config);
	server.on("/api/servo_config", HTTP_GET, api_servo_config);
	server.on("/api/set_servo_config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, api_set_servo_config);

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
	int8_t mode = config.getModeConfig().mode; // 獲取當前模式
	uint8_t activeTaskMask = 0;				   // 僅記錄完整啟動成功的任務
	ESP_LOGI("mode", "Current mode: %d", mode);
	while (1)
	{
		uint8_t targetTaskMask = 0;
		if (!GetModeTaskMask(mode, &targetTaskMask))
		{
			ESP_LOGE("mode", "Invalid mode: %d", mode);
		}
		else if (targetTaskMask != activeTaskMask)
		{
			// 只切換差異任務，避免重建共用任務
			uint8_t stopMask = activeTaskMask & ~targetTaskMask;
			uint8_t startMask = targetTaskMask & ~activeTaskMask;
			ESP_LOGI("mode", "Transition mode %d: active=0x%02X target=0x%02X stop=0x%02X start=0x%02X", mode, activeTaskMask, targetTaskMask,
					 stopMask, startMask);

			// 先停止舊模式的資料生產者
			if (stopMask & TASK_MASK_GYROSCOPE)
			{
				TaskDeleteSafe(&taskGyroscopeTracking_handle);
			}
			if (stopMask & TASK_MASK_NETWORK)
			{
				TaskDeleteSafe(&taskNetworkControl_handle);
				TaskDeleteSafe(&taskNetworkControlGC9A01_handle);
			}
			if (stopMask & TASK_MASK_SERVO_SET)
			{
				TaskDeleteSafe(&taskServoSet_handle);
			}
			if (stopMask & TASK_MASK_WIT)
			{
				StopWitTasks();
				TaskDeleteSafe(&taskWitPProcessingData_handle);
			}
			if (stopMask & TASK_MASK_EYES_MOVE)
			{
				TaskDeleteSafe(&taskEyesMove_handle);
			}

			ResetModeQueues(); // producer 停止後才清除殘留資料

			// consumer 先建立，WIT reader 最後啟動
			bool createSuccess = true;
			if (startMask & TASK_MASK_EYES_MOVE)
			{
				createSuccess = TaskCreateSafe(taskEyesMove, "taskEyesMove", 4096, NULL, 1, &taskEyesMove_handle, 1);
			}
			if (createSuccess && (startMask & TASK_MASK_WIT))
			{
				createSuccess = TaskCreateSafe(taskWitPProcessingData, "taskWitPProcessingData", 4096, NULL, 1, &taskWitPProcessingData_handle, 1);
			}
			if (createSuccess && (startMask & TASK_MASK_GYROSCOPE))
			{
				createSuccess = TaskCreateSafe(taskGyroscopeTracking, "taskGyroscopeTracking", 4096, NULL, 1, &taskGyroscopeTracking_handle, 1);
			}
			if (createSuccess && (startMask & TASK_MASK_NETWORK))
			{
				createSuccess =
					TaskCreateSafe(taskNetworkControlGC9A01, "taskNetworkControlGC9A01", 4096, NULL, 1, &taskNetworkControlGC9A01_handle, 1);
				if (createSuccess)
				{
					createSuccess = TaskCreateSafe(taskNetworkControl, "taskNetworkControl", 4096, NULL, 1, &taskNetworkControl_handle, 1);
				}
			}
			if (createSuccess && (startMask & TASK_MASK_SERVO_SET))
			{
				createSuccess = TaskCreateSafe(taskServoSet, "taskServoSet", 4096, NULL, 1, &taskServoSet_handle, 1);
			}
			if (createSuccess && (startMask & TASK_MASK_WIT))
			{
				// 清除上一次切換留下的停止狀態
				witEyes.wit_clear_stop();
				witHead.wit_clear_stop();
				xEventGroupClearBits(witStopEventGroup, WIT_EYES_STOPPED_BIT | WIT_HEAD_STOPPED_BIT);
				createSuccess = TaskCreateSafe(taskWitGetData, "taskWitEyesGetData", 4096, &witEyes, 1, &taskWitEyesGetData_handle, 1);
				if (createSuccess)
				{
					createSuccess = TaskCreateSafe(taskWitGetData, "taskWitHeadGetData", 4096, &witHead, 1, &taskWitHeadGetData_handle, 1);
				}
			}

			if (createSuccess)
			{
				activeTaskMask = targetTaskMask;
				ESP_LOGI("mode", "Mode %d active; task mask=0x%02X", mode, activeTaskMask);
			}
			else
			{
				ESP_LOGE("mode", "Mode %d activation failed; rolling back", mode);
				// 部分建立成功時回到無模式任務狀態
				StopAllModeTasks();
				activeTaskMask = 0;
			}
		}

		xQueueReceive(mode_data_quene, &mode, portMAX_DELAY);
		ESP_LOGI("mode", "Current mode change: %d", mode);
	}
}

/* 獲取wit數據任務 */
void taskWitGetData(void *arg)
{
	witData wit_data;		 // wit數據結構體
	wit *myWit = (wit *)arg; // 獲取wit數據

	uint8_t serialPort = myWit->wit_serial_get(); // 獲取Serial端口

	/* 初始化wit */
	int8_t wit_status = WIT_INIT_ERROR;
	// 初始化失敗時保留重試能力並持續接受停止要求
	while (!myWit->wit_stop_requested())
	{
		wit_status = myWit->wit_init();
		if (wit_status == 0 || wit_status == WIT_INIT_CANCELLED)
		{
			break;
		}
		ESP_LOGW("WIT", "Serial%d initialization failed; retrying", serialPort);
		for (uint8_t i = 0; i < 50 && !myWit->wit_stop_requested(); i++)
		{
			vTaskDelay(pdMS_TO_TICKS(20));
		}
	}

	if (!myWit->wit_stop_requested() && wit_status == 0)
	{
		myWit->wit_flush();
		while (!myWit->wit_stop_requested())
		{
			wit_data = myWit->wit_get_data();
			if (wit_data.status != WIT_CANCELLED)
			{
				xQueueSend(wit_data_quene, &wit_data, 0);
			}
			vTaskDelay(1);
		}
	}

	myWit->wit_end(); // 回報停止前先釋放 UART
	EventBits_t stoppedBit = serialPort == SERIAL1 ? WIT_EYES_STOPPED_BIT : WIT_HEAD_STOPPED_BIT;
	if (witStopEventGroup)
	{
		xEventGroupSetBits(witStopEventGroup, stoppedBit);
		vTaskSuspend(NULL);
	}
	vTaskDelete(NULL);
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

	// 任務啟動後以佇列更新本地快照，避免高頻讀取NVS
	AppConfig::AdvancedConfig advanced_config = config.getAdvancedConfig();
	uint16_t reset_reference_timer = advanced_config.correction_timer; // 重置参考角度時間器
	uint64_t reset_reference_time = 0;								   // 重置参考角度時間

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

		if (xQueueReceive(wit_advanced_config_update_quene, &advanced_config, 0) == pdTRUE)
		{
			reset_reference_timer = advanced_config.correction_timer;
			// 新週期不可沿用舊設定已累積的靜止時間
			reset_reference_time = xTaskGetTickCount();
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
	// 任務啟動後以佇列更新本地快照，避免高頻讀取NVS
	AppConfig::AdvancedConfig advanced_config = config.getAdvancedConfig();

	double eyes_x = 0;			   // x角度
	double eyes_y = 0;			   // y角度
	double calculate_angularSpeed; // 角速度絕對值

	uint64_t last_move_time[2] = {0, 0}; // 上次移動時間
	while (1)
	{
		if (xQueueReceive(wit_data_relative_angle_quene, &data_get, portMAX_DELAY) == pdTRUE) // 從佇列中獲取數據
		{
			xQueueReceive(gyroscope_advanced_config_update_quene, &advanced_config, 0);

			/* x角度範圍 */
			eyes_x = map(constrain(data_get.relative_angle.zangle, -60, 60), 60, -60, -55, 55); // z角度映射

			/* y角度範圍 */
			eyes_y = map(constrain(data_get.relative_angle.yangle, -30, 30), 30, -30, 35, -35); // y角度映射

			/* 傳遞眼睛角度 */
			angle_data_send.x_angle = round((int8_t)eyes_x); // 設置x角度
			angle_data_send.y_angle = round((int8_t)eyes_y); // 設置y角度
			angle_data_send.eyelid_angle = advanced_config.gyroscope_eyelid_angle;
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
	double eyelid_angle = 40;			  // 眼睛張開角度
	uint8_t eyelid_angle_int = 40;		  // 眼睛張開角度整數
	uint8_t send_eyelid_angle = 40;		  // 眼睛張開目標角度
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
			y = map(data_get.y, -100, 100, -40, 40);

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

/* 伺服馬達調試任務 */
void taskServoSet(void *arg)
{
	servoSet_data servoSet;	 // 伺服馬達調試類
	GC9A01_data gc9a01_data; // GC9A01數據結構體

	gc9a01_data.R = 80;								// 設置GC9A01半徑
	gc9a01_data.zetaR = 1;							// 設置GC9A01阻尼比
	gc9a01_data.omega_nR = 5;						// 設置GC9A01自然頻率
	gc9a01_data.lightMax = 150;						// 設置GC9A01光暈最大值
	gc9a01_data.zetaLightMax = 1;					// 設置GC9A01光暈阻尼比
	gc9a01_data.omega_nLightMax = 5;				// 設置GC9A01光暈自然頻率
	xQueueSend(gc9a01_data_quene, &gc9a01_data, 0); // 傳遞GC9A01數據
	while (1)
	{
		if (xQueueReceive(servoSet_data_quene, &servoSet, portMAX_DELAY) == pdTRUE) // 從佇列中獲取數據
		{
			eyesmove.eyesMove_servo_debug(servoSet.upper_eyelid_angle, servoSet.lower_eyelid_angle,
										  servoSet.eyeball_angle); // 更新伺服馬達調試類
		}
	}
}

/* 畫眼睛任務 */
void taskGC9A01(void *arg)
{
	ESP_LOGV("GC9A01", "GC9A01 task started");

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
	ESP_LOGV("EyesMove", "EyesMove task started");

	eyesMove_data data_get;				   // 眼睛數據結構體
	uint8_t is_eyesmove_update_finish = 0; // 眼睛更新狀態

	/* 初始化眼睛 */
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
				uint16_t read_size = event.size < BUF_SIZE ? event.size : BUF_SIZE - 1;
				uint16_t len = uart_read_bytes(UART_NUM_0, data, read_size, portMAX_DELAY);
				if (len > 0)
				{
					data[len] = '\0';
					String data_str = String((char *)data);
					data_str.trim();		// 去除空格
					data_str.toLowerCase(); // 轉換為小寫

					/** 執行指令 **/
					/* 重啓指令 */
					if (data_str == "reset")
					{
						ESP_LOGI("UART", "Reset command received. Restarting...");
						ESP.restart();
					}
					/* 幫助指令 */
					else if (data_str == "help")
					{
						ESP_LOGI("UART", "\nAvailable commands:\nreset - Restart the device\nreservo - Reset servo configuration\nhelp - Show this "
										 "help message\nmode [number] - Show or change "
										 "the operating mode\nlog [number] - Show or change the log level (1-5)");
					}
					/* 重置伺服馬達設定指令 */
					else if (data_str == "reservo")
					{
						config.clearServoConfig();

						int8_t mode = SERVO_SET_MODE;
						AppConfig::ModeConfig mode_config = config.getModeConfig();
						mode_config.mode = static_cast<uint8_t>(mode);
						config.setModeConfig(mode_config);

						ESP_LOGI("UART", "Servo configuration cleared. Switched to servo setup mode.");
						xQueueOverwrite(mode_data_quene, &mode); // 快速切換時只保留最新模式
					}
					/* 模式指令 */
					else if (data_str == "mode" || data_str.startsWith("mode "))
					{
						String mode_value = data_str.substring(4);
						mode_value.trim();

						if (mode_value.length() == 0) // 顯示當前模式
						{
							AppConfig::ModeConfig mode_config = config.getModeConfig();
							ESP_LOGI("UART", "Current mode: %d", static_cast<int>(mode_config.mode));
						}
						else // 嘗試更改模式
						{
							bool is_number = true;
							for (size_t i = 0; i < mode_value.length() && is_number; i++)
							{
								is_number = isDigit(mode_value.charAt(i));
							}

							if (!is_number) // 檢測是否為數字
							{
								ESP_LOGE("UART", "Invalid mode command. Usage: mode <number>");
							}
							else
							{
								long mode_value_number = mode_value.toInt();
								if (mode_value_number < 0 || mode_value_number > 2) // 檢測模式值是否有效
								{
									ESP_LOGE("UART", "Invalid mode value: %ld. Valid values are %d or %d", mode_value_number, GYROSCOPE_TRACKS_MODE,
											 NETWORK_CONTROL_MODE);
								}
								else
								{
									int8_t mode = static_cast<int8_t>(mode_value_number);
									AppConfig::ModeConfig mode_config = config.getModeConfig();
									mode_config.mode = static_cast<uint8_t>(mode);
									config.setModeConfig(mode_config);
									xQueueOverwrite(mode_data_quene, &mode); // 快速切換時只保留最新模式
								}
							}
						}
					}
					else if (data_str == "log" || data_str.startsWith("log "))
					{
						String log_value = data_str.substring(3);
						log_value.trim();

						if (log_value.length() == 0)
						{
							char response[32];
							int response_length = snprintf(response, sizeof(response), "Current log level: %u\r\n",
														   static_cast<unsigned int>(esp_log_get_default_level()));
							uart_write_bytes(UART_NUM_0, response, response_length);
						}
						else
						{
							bool is_number = true;
							for (size_t i = 0; i < log_value.length() && is_number; i++)
							{
								is_number = isDigit(log_value.charAt(i));
							}

							if (!is_number)
							{
								ESP_LOGE("UART", "Invalid log command. Usage: log <1-5>");
							}
							else
							{
								long log_level = log_value.toInt();
								if (log_level < ESP_LOG_ERROR || log_level > ESP_LOG_VERBOSE)
								{
									ESP_LOGE("UART", "Invalid log level: %ld. Valid values are 1-5", log_level);
								}
								else
								{
									config.setLogConfig(static_cast<uint8_t>(log_level));
									esp_log_level_set("*", static_cast<esp_log_level_t>(log_level));

									char response[32];
									int response_length = snprintf(response, sizeof(response), "Log level changed to: %ld\r\n", log_level);
									uart_write_bytes(UART_NUM_0, response, response_length);
								}
							}
						}
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

bool TaskCreateSafe(TaskFunction_t task, const char *name, uint32_t stackSize, void *parameter, UBaseType_t priority, TaskHandle_t *handle,
					BaseType_t core)
{
	if (!task || !name || !handle)
	{
		return false;
	}
	if (*handle)
	{
		// 禁止覆寫仍可管理的任務 handle
		ESP_LOGE("task", "Task already exists: %s", name);
		return false;
	}

	BaseType_t result = xTaskCreatePinnedToCore(task, name, stackSize, parameter, priority, handle, core);
	if (result != pdPASS)
	{
		*handle = nullptr;
		ESP_LOGE("task", "Failed to create %s; free heap: %u", name, ESP.getFreeHeap());
		return false;
	}
	return true;
}

bool GetModeTaskMask(int8_t mode, uint8_t *taskMask)
{
	if (!taskMask)
	{
		return false;
	}

	switch (mode)
	{
		case SERVO_SET_MODE:
			*taskMask = TASK_MASK_SERVO_SET;
			return true;
		case GYROSCOPE_TRACKS_MODE:
			*taskMask = TASK_MASK_WIT | TASK_MASK_GYROSCOPE | TASK_MASK_EYES_MOVE;
			return true;
		case NETWORK_CONTROL_MODE:
			*taskMask = TASK_MASK_NETWORK | TASK_MASK_EYES_MOVE;
			return true;
		default:
			*taskMask = 0;
			return false;
	}
}

bool StopWitTasks()
{
	// 只等待目前實際存在的 WIT reader
	EventBits_t expectedBits = 0;
	if (taskWitEyesGetData_handle)
	{
		expectedBits |= WIT_EYES_STOPPED_BIT;
	}
	if (taskWitHeadGetData_handle)
	{
		expectedBits |= WIT_HEAD_STOPPED_BIT;
	}
	if (!expectedBits)
	{
		return true;
	}

	xEventGroupClearBits(witStopEventGroup, expectedBits);
	if (expectedBits & WIT_EYES_STOPPED_BIT)
	{
		witEyes.wit_request_stop();
	}
	if (expectedBits & WIT_HEAD_STOPPED_BIT)
	{
		witHead.wit_request_stop();
	}

	// acknowledgement 代表 UART 已由 reader 自行關閉
	EventBits_t stoppedBits = xEventGroupWaitBits(witStopEventGroup, expectedBits, pdTRUE, pdTRUE, pdMS_TO_TICKS(WIT_STOP_TIMEOUT_MS));
	bool stoppedCleanly = (stoppedBits & expectedBits) == expectedBits;
	if (!stoppedCleanly)
	{
		ESP_LOGE("mode", "WIT stop timeout; expected=0x%02X received=0x%02X", expectedBits, stoppedBits);
	}

	TaskDeleteSafe(&taskWitEyesGetData_handle);
	TaskDeleteSafe(&taskWitHeadGetData_handle);
	if (!stoppedCleanly)
	{
		// 超時後只能在任務已刪除的前提下補做清理
		witEyes.wit_end();
		witHead.wit_end();
	}
	return stoppedCleanly;
}

void ResetModeQueues()
{
	// 呼叫前必須先停止所有舊模式 producer
	xQueueReset(wit_data_quene);
	xQueueReset(wit_data_relative_angle_quene);
	xQueueReset(wit_advanced_config_update_quene);
	xQueueReset(gyroscope_advanced_config_update_quene);
	xQueueReset(network_control_data_quene);
	xQueueReset(network_control_speed_data_quene);
	xQueueReset(servoSet_data_quene);
	xQueueReset(eyesmove_data_quene);
	xQueueReset(gc9a01_data_quene);
}

void StopAllModeTasks()
{
	// 任務建立失敗時回滾所有模式資源
	TaskDeleteSafe(&taskGyroscopeTracking_handle);
	TaskDeleteSafe(&taskNetworkControl_handle);
	TaskDeleteSafe(&taskNetworkControlGC9A01_handle);
	TaskDeleteSafe(&taskServoSet_handle);
	StopWitTasks();
	TaskDeleteSafe(&taskWitPProcessingData_handle);
	TaskDeleteSafe(&taskEyesMove_handle);
	ResetModeQueues();
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
