#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

void sendJsonResponse(AsyncWebServerRequest *request, int16_t statusCode, bool success, int32_t code, const char *message = "",
					  JsonDocument *dataDoc = nullptr);