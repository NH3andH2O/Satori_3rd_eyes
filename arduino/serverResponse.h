#ifndef SERVER_RESPONSE_H
#define SERVER_RESPONSE_H
#define DEFAULT_SSID "3rd-Eyes" // wifi SoftAP名稱

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "verify.h"

extern Preferences prefs; // 偏好設置實例

extern QueueHandle_t wifiUpdate_data_quene; // 宣告WiFi更新佇列

void onSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void api_wifi_config(AsyncWebServerRequest *request);
void api_set_wifi_config(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
void api_softAP_config(AsyncWebServerRequest *request);
void api_set_softAP_config(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

#endif