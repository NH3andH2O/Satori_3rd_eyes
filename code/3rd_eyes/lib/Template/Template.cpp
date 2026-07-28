#include "Template.h"

void sendJsonResponse(AsyncWebServerRequest *request, int16_t statusCode, bool success, int32_t code, const char *message, JsonDocument *dataDoc)
{
	/* 創建JSON響應 */
	AsyncResponseStream *response = request->beginResponseStream("application/json");
	response->addHeader("Access-Control-Allow-Origin", "*");
	response->addHeader("Cache-Control", "no-store");

	/* 構建JSON */
	JsonDocument doc;
	doc["success"] = success;
	doc["code"] = code;
	doc["message"] = message;

	if (dataDoc != nullptr)
	{
		doc["data"] = *dataDoc;
	}

	/* 發送響應 */
	serializeJson(doc, *response);
	request->send(response);
}
