#include <ESP32Servo.h>
#include <esp_task_wdt.h>
#include "eyesMove.h"
#include "wit.h"

#define UPPER_EYELID_PIN 12	//上眼皮伺服馬達引脚
#define LOWER_EYELID_PIN 13	//下眼皮伺服馬達引脚
#define EYEBALL_PIN 14		//眼球伺服馬達引脚

eyesMove eyesmove(UPPER_EYELID_PIN, LOWER_EYELID_PIN, EYEBALL_PIN);
wit witEyes(SERIAL1, 18, 8, 115200);	//wit眼睛模組
wit witHead(SERIAL2, 39, 38, 115200);	//wit頭部模組

witData witEyes_data;	//wit眼睛數據結構體
witData witHead_data;	//wit頭部數據結構體

QueueHandle_t init_status_quene; //宣告佇列

/* 任務參照 */
TaskHandle_t taskWitEyesGetData_hamdle;	//獲取wit眼睛數據任務
TaskHandle_t taskWitHeadGetData_hamdle;	//獲取wit頭部數據任務

/* 獲取wit數據任務 */
void taskWitGetData(void *arg)
{
	witData wit_data;	//wit數據結構體
	wit *myWit = (wit *)arg;	//獲取wit數據

	uint8_t serialProt = myWit->wit_serial_get();	//獲取Serial端口
	String wit_name = (serialProt == SERIAL1) ? "witEyes" : "witHead";	//獲取Serial端口名稱

	/* 初始化wit */
	int8_t wit_status = myWit->wit_init();	//初始化wit眼睛模組
	switch (wit_status)
	{
	case 0:
		printf("%s init success\r\n", wit_name.c_str());		//打印初始化成功
		break;
	case WIT_INIT_ERROR:
		printf("%s init error\r\n", wit_name.c_str());			//打印初始化錯誤
		break;
	case SERIAL_INIT_ERROR:
		printf("%s serial init error\r\n", wit_name.c_str());	//打印Serial初始化錯誤
		break;
	default:
		printf("%s init unknown error\r\n", wit_name.c_str());	//打印未知錯誤
		break;
	}
	while (wit_status)	//失敗進入死循環
	{
		vTaskDelay(1000);
	}

	/* 清除緩存 */
	myWit->wit_flush();

	/* 獲取數據 */
	while (1)
	{
		wit_data = myWit->wit_get_data();	//獲取數據
		vTaskDelay(1);
	}
}

void setup()
{
	Serial.begin(115200);	//初始化串口
	vTaskDelay(2000);

	Serial.println("wit init...");	//打印初始化狀態
	xTaskCreate(taskWitGetData, "taskWitEyesGetData", 4096, &witEyes, 1, &taskWitEyesGetData_hamdle);	//創建獲取數據任務
	xTaskCreate(taskWitGetData, "taskWitHeadGetData", 4096, &witHead, 1, &taskWitHeadGetData_hamdle);	//創建獲取數據任務
}

void loop()
{
	UBaseType_t taskWitEyesGetData_Stack = uxTaskGetStackHighWaterMark(taskWitEyesGetData_hamdle);	//獲取任務堆棧大小
	UBaseType_t taskWitHeadGetData_Stack = uxTaskGetStackHighWaterMark(taskWitHeadGetData_hamdle);	//獲取任務堆棧大小
	static UBaseType_t taskWitEyesGetData_Stack_highest = 0;
	static UBaseType_t taskWitHeadGetData_Stack_highest = 0;
	if (taskWitEyesGetData_Stack > taskWitEyesGetData_Stack_highest)
	{
		taskWitEyesGetData_Stack_highest = taskWitEyesGetData_Stack;
	}
	if (taskWitHeadGetData_Stack > taskWitHeadGetData_Stack_highest)
	{
		taskWitHeadGetData_Stack_highest = taskWitHeadGetData_Stack;
	}
	if(millis() % 1000 == 0)	//每秒打印一次
	{
		Serial.printf("taskWitEyesGetData stack: %u (highest: %u)\r\n", taskWitEyesGetData_Stack,taskWitEyesGetData_Stack_highest);	//打印任務狀態
		Serial.printf("taskWitHeadGetData stack: %u (highest: %u)\r\n", taskWitHeadGetData_Stack,taskWitHeadGetData_Stack_highest);	//打印任務狀態
	}

}