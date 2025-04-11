#include <ESP32Servo.h>
#include <LovyanGFX.hpp>
#include "gc9a01.h"
#include "eyesMove.h"
#include "wit.h"

#define UPPER_EYELID_PIN 12	//上眼皮伺服馬達引脚
#define LOWER_EYELID_PIN 13	//下眼皮伺服馬達引脚
#define EYEBALL_PIN 14		//眼球伺服馬達引脚

#define witEyes_RX_PIN 18	//wit眼睛模組RX引脚
#define witEyes_TX_PIN 8	//wit眼睛模組TX引脚
#define witHead_RX_PIN 39	//wit頭部模組RX引脚
#define witHead_TX_PIN 38	//wit頭部模組TX引脚

#define GC9A01_BLK_PIN 5	//GC9A01背光引脚
#define GC9A01_RST_PIN 15	//GC9A01重置引脚
#define GC9A01_CS_PIN 6		//GC9A01片選引脚
#define GC9A01_DC_PIN 7		//GC9A01數據/命令引脚
#define GC9A01_SCL_PIN 17	//GC9A01時鐘引脚
#define GC9A01_SDA_PIN 16	//GC9A01數據引脚

eyesMove eyesmove(UPPER_EYELID_PIN, LOWER_EYELID_PIN, EYEBALL_PIN);

wit witEyes(SERIAL1, witEyes_RX_PIN, witEyes_TX_PIN, 115200);	//wit眼睛模組
wit witHead(SERIAL2, witHead_RX_PIN, witHead_TX_PIN, 115200);	//wit頭部模組

GC9A01 gc9a01(GC9A01_SDA_PIN, GC9A01_SCL_PIN, GC9A01_CS_PIN, GC9A01_DC_PIN, GC9A01_RST_PIN, GC9A01_BLK_PIN);	//GC9A01實例

witData witEyes_data;	//wit眼睛數據結構體
witData witHead_data;	//wit頭部數據結構體

QueueHandle_t wit_data_quene; //宣告佇列

/* 任務參照 */
TaskHandle_t taskWitEyesGetData_hamdle;	//獲取wit眼睛數據任務
TaskHandle_t taskWitHeadGetData_hamdle;	//獲取wit頭部數據任務
TaskHandle_t taskWitPProcessingData_hamdle;	//處理wit數據任務
TaskHandle_t taskGC9A01_hamdle;	//GC9A01任務

/* 獲取wit數據任務 */
void taskWitGetData(void *arg)
{
	witData wit_data;	//wit數據結構體
	wit *myWit = (wit *)arg;	//獲取wit數據

	uint8_t serialPort = myWit->wit_serial_get();	//獲取Serial端口
	String wit_name = (serialPort == SERIAL1) ? "witEyes" : "witHead";	//獲取Serial端口名稱

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
		wit_data = myWit->wit_get_data();			//獲取數據
		xQueueSend(wit_data_quene, &wit_data, 0);	//將數據放入佇列
		vTaskDelay(1);
	}
}

/* 處理角度差 */
void taskWitPProcessingData(void *arg)
{
	witData wit_data;						//wit數據結構體

	witDataAngle witEyes_reference_angle;	//眼睛參考角度
	witDataAngle witHead_reference_angle;	//頭部參考角度

	witDataAngle witEyes_angle;			//眼睛角度
	witDataAngle witHead_angle;			//頭部角度
	witDataAngle wit_angle_diff;	//眼睛角度差值

	uint8_t witEyes_angle_status = 0;	//眼睛角度狀態
	uint8_t witHead_angle_status = 0;	//頭部角度狀態

	uint8_t read_count = 0;	//讀取計數

	/* 抛棄前10次數據 */
	while (read_count < 10)
	{
		xQueueReceive(wit_data_quene, &wit_data, portMAX_DELAY);	//從佇列中獲取數據
		read_count++;
	}

	while (1)
	{
		/* 數據獲取 */
		xQueueReceive(wit_data_quene, &wit_data, portMAX_DELAY);	//從佇列中獲取數據
		String wit_name = (wit_data.serialPort == SERIAL1) ? "witEyes" : "witHead";

		/* 數據處理 */
		switch(wit_data.status)	//數據狀態
		{
			case 0:	//數據正常
				/* 眼睛數據處理 */
				if(wit_data.serialPort == SERIAL1)	
				{

					/* 設置參考角度 */
					if(witEyes_angle_status == 0)
					{
						witEyes_reference_angle.xangle = wit_data.xangle;	//設置參考角度
						witEyes_reference_angle.yangle = wit_data.yangle;
						witEyes_reference_angle.zangle = wit_data.zangle;
						witEyes_angle_status = 1;							//設置參考角度狀態
					}

					/* 處理參考角度 */
					witEyes_angle.xangle = wit_data.xangle - witEyes_reference_angle.xangle;	//處理參考角度
					witEyes_angle.yangle = wit_data.yangle - witEyes_reference_angle.yangle;
					witEyes_angle.zangle = wit_data.zangle - witEyes_reference_angle.zangle;

					/* 處理角度環繞 */
					if(witEyes_angle.xangle > 180)			//X角度環繞
					{
						witEyes_angle.xangle -= 360;
					}
					else if(witEyes_angle.xangle < -180)
					{
						witEyes_angle.xangle += 360;
					}
					if(witEyes_angle.yangle > 180)			//Y角度環繞
					{
						witEyes_angle.yangle -= 360;
					}
					else if(witEyes_angle.yangle < -180)
					{
						witEyes_angle.yangle += 360;
					}
					if(witEyes_angle.zangle > 180)			//Z角度環繞
					{
						witEyes_angle.zangle -= 360;
					}
					else if(witEyes_angle.zangle < -180)
					{
						witEyes_angle.zangle += 360;
					}
				}
				/* 頭部數據處理 */
				else if(wit_data.serialPort == SERIAL2)	
				{

					/* 設置參考角度 */
					if(witHead_angle_status == 0)
					{
						witHead_reference_angle.xangle = wit_data.xangle;	//設置參考角度
						witHead_reference_angle.yangle = wit_data.yangle;
						witHead_reference_angle.zangle = wit_data.zangle;
						witHead_angle_status = 1;							//設置參考角度狀態
					}

					/* 處理參考角度 */
					witHead_angle.xangle = wit_data.xangle - witHead_reference_angle.xangle;	//處理參考角度
					witHead_angle.yangle = wit_data.yangle - witHead_reference_angle.yangle;
					witHead_angle.zangle = wit_data.zangle - witHead_reference_angle.zangle;

					/* 處理角度環繞 */
					if(witHead_angle.xangle > 180)			//X角度環繞
					{
						witHead_angle.xangle -= 360;
					}
					else if(witHead_angle.xangle < -180)
					{
						witHead_angle.xangle += 360;
					}
					if(witHead_angle.yangle > 180)			//Y角度環繞
					{
						witHead_angle.yangle -= 360;
					}
					else if(witHead_angle.yangle < -180)
					{
						witHead_angle.yangle += 360;
					}
					if(witHead_angle.zangle > 180)			//Z角度環繞
					{
						witHead_angle.zangle -= 360;
					}
					else if(witHead_angle.zangle < -180)
					{
						witHead_angle.zangle += 360;
					}
				}
				break;
			default:
				break;
		}
		
		/* 差角計算 */
		if(witEyes_angle_status == 1 && witHead_angle_status == 1)	//參考角度設置完成
		{
			wit_angle_diff.xangle = witEyes_angle.xangle - witHead_angle.xangle;	//計算差角
			wit_angle_diff.yangle = witEyes_angle.yangle - witHead_angle.yangle;
			wit_angle_diff.zangle = witEyes_angle.zangle - witHead_angle.zangle;

			/* 處理差角環繞 */
			if(wit_angle_diff.xangle > 180)			//X角度環繞
			{
				wit_angle_diff.xangle -= 360;
			}
			else if(wit_angle_diff.xangle < -180)
			{
				wit_angle_diff.xangle += 360;
			}
			if(wit_angle_diff.yangle > 180)			//Y角度環繞
			{
				wit_angle_diff.yangle -= 360;
			}
			else if(wit_angle_diff.yangle < -180)
			{
				wit_angle_diff.yangle += 360;
			}
			if(wit_angle_diff.zangle > 180)			//Z角度環繞
			{
				wit_angle_diff.zangle -= 360;
			}
			else if(wit_angle_diff.zangle < -180)
			{
				wit_angle_diff.zangle += 360;
			}
		}
	}
}

void taskGC9A01(void *arg)
{
	/* 初始化GC9A01 */
	gc9a01.GC9A01_init();	//初始化GC9A01
	gc9a01.GC9A01_setEyes_r(80);	//設置眼睛半徑
	while (1)
	{
		for(int i = 0; i <= 5; i++)
		{
			gc9a01.GC9A01_setEyes_r(80 - 7 * i);	//設置眼睛半徑
			gc9a01.GC9A01_update();
			vTaskDelay(1);
		}
		vTaskDelay(3000);
		for(int i = 5; i >= 0; i--)
		{
			gc9a01.GC9A01_setEyes_r(80 - 7 * i);	//設置眼睛半徑
			gc9a01.GC9A01_update();
			vTaskDelay(1);
		}
		gc9a01.GC9A01_update();	//更新GC9A01
		vTaskDelay(3000);
	}
}

void setup()
{
	Serial.begin(115200);
	while (xTaskGetTickCount() < 2000)
	{
		;//等待2秒
	}

	/* 佇列建立 */
	Serial.println("quene create...");	//打印佇列建立狀態
	wit_data_quene = xQueueCreate(10, sizeof(witData));	//建立佇列，長度10，大小為witData結構體大小
	if (wit_data_quene == NULL)	//佇列建立失敗
	{
		Serial.println("wit_data quene create error");
		while (2)
		{
			vTaskDelay(1000);
		}
	}
	Serial.println("wit_data quene create success");	//打印佇列建立成功狀態

	Serial.println("wit init...");	//打印初始化狀態
	xTaskCreatePinnedToCore(taskWitGetData, "taskWitEyesGetData", 4096, &witEyes, 1, &taskWitEyesGetData_hamdle, 1);	//創建獲取數據任務
	xTaskCreatePinnedToCore(taskWitGetData, "taskWitHeadGetData", 4096, &witHead, 1, &taskWitHeadGetData_hamdle, 1);	//創建獲取數據任務
	xTaskCreatePinnedToCore(taskWitPProcessingData, "taskWitPProcessingData", 4096, NULL, 1, &taskWitPProcessingData_hamdle, 1);	//創建數據處理任務
	xTaskCreatePinnedToCore(taskGC9A01, "taskGC9A01", 4096, NULL, 1, &taskGC9A01_hamdle, 0);	//創建GC9A01任務
}

void loop()
{
	UBaseType_t taskWitEyesGetData_Stack = uxTaskGetStackHighWaterMark(taskWitEyesGetData_hamdle);	//獲取任務堆棧大小
	UBaseType_t taskWitHeadGetData_Stack = uxTaskGetStackHighWaterMark(taskWitHeadGetData_hamdle);	//獲取任務堆棧大小
	UBaseType_t taskWitPProcessingData_Stack = uxTaskGetStackHighWaterMark(taskWitPProcessingData_hamdle);	//獲取任務堆棧大小
	UBaseType_t taskGC9A01_Stack = uxTaskGetStackHighWaterMark(taskGC9A01_hamdle);	//獲取任務堆棧大小

	static UBaseType_t taskWitEyesGetData_Stack_highest = 0;
	static UBaseType_t taskWitHeadGetData_Stack_highest = 0;
	static UBaseType_t taskWitPProcessingData_Stack_highest = 0;
	static UBaseType_t taskGC9A01_Stack_highest = 0;

	if (taskWitEyesGetData_Stack > taskWitEyesGetData_Stack_highest)
	{
		taskWitEyesGetData_Stack_highest = taskWitEyesGetData_Stack;
	}
	if (taskWitHeadGetData_Stack > taskWitHeadGetData_Stack_highest)
	{
		taskWitHeadGetData_Stack_highest = taskWitHeadGetData_Stack;
	}
	if (taskWitPProcessingData_Stack > taskWitPProcessingData_Stack_highest)
	{
		taskWitPProcessingData_Stack_highest = taskWitPProcessingData_Stack;
	}
	if (taskGC9A01_Stack > taskGC9A01_Stack_highest)
	{
		taskGC9A01_Stack_highest = taskGC9A01_Stack;
	}

	if(xTaskGetTickCount() % 1000 == 0)	//每秒打印一次
	{
		Serial.printf("taskWitEyesGetData stack: %u (highest: %u)\r\n", taskWitEyesGetData_Stack,taskWitEyesGetData_Stack_highest);	//打印任務狀態
		Serial.printf("taskWitHeadGetData stack: %u (highest: %u)\r\n", taskWitHeadGetData_Stack,taskWitHeadGetData_Stack_highest);	//打印任務狀態
		Serial.printf("taskWitPProcessingData stack: %u (highest: %u)\r\n", taskWitPProcessingData_Stack,taskWitPProcessingData_Stack_highest);	//打印任務狀態
		Serial.printf("taskGC9A01 stack: %u (highest: %u)\r\n", taskGC9A01_Stack,taskGC9A01_Stack_highest);	//打印任務狀態
	}
}