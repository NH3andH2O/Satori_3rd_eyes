#include <ESP32Servo.h>
#include <LovyanGFX.hpp>
#include "gc9a01.h"
#include "eyesMove.h"
#include "wit.h"
#include "IMUAngle.h"

#define UPPER_EYELID_PIN 13	//上眼皮伺服馬達引脚
#define LOWER_EYELID_PIN 14	//下眼皮伺服馬達引脚
#define EYEBALL_PIN 21		//眼球伺服馬達引脚

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

typedef struct
{
	uint8_t R;
	double zetaR;
	double omega_nR;
	uint8_t lightMax;
	double zetaLightMax;
	double omega_nLightMax;
} GC9A01_data;	//GC9A01數據結構體

eyesMove eyesmove(UPPER_EYELID_PIN, LOWER_EYELID_PIN, EYEBALL_PIN);

wit witEyes(SERIAL1, witEyes_RX_PIN, witEyes_TX_PIN, 115200);	//wit眼睛模組
wit witHead(SERIAL2, witHead_RX_PIN, witHead_TX_PIN, 115200);	//wit頭部模組

GC9A01 gc9a01(GC9A01_SDA_PIN, GC9A01_SCL_PIN, GC9A01_CS_PIN, GC9A01_DC_PIN, GC9A01_RST_PIN, GC9A01_BLK_PIN);	//GC9A01實例

witData witEyes_data;	//wit眼睛數據結構體
witData witHead_data;	//wit頭部數據結構體

QueueHandle_t wit_data_quene; //宣告wit原始佇列
QueueHandle_t wit_data_relative_angle_quene; //宣告wit差值佇列
QueueHandle_t gc9a01_data_quene; //宣告GC9A01佇列

/* 任務參照 */
TaskHandle_t taskWitEyesGetData_hamdle;	//獲取wit眼睛數據任務
TaskHandle_t taskWitHeadGetData_hamdle;	//獲取wit頭部數據任務
TaskHandle_t taskWitPProcessingData_hamdle;	//處理wit數據任務
TaskHandle_t taskGC9A01_hamdle;	//GC9A01任務
TaskHandle_t taskEyesMove_hamdle;	//眼睛任務

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

	witDataQuaternion witEyes_reference_quaternion;		//眼睛参考四元數
	witDataQuaternion witHead_reference_quaternion;		//頭部参考四元數
	witDataQuaternion *reference_quaternion = nullptr;	//參考四元數指標


	witDataQuaternion witEyes_quaternion;			//眼睛四元數
	witDataQuaternion witHead_quaternion;			//頭部四元數
	witDataQuaternion *quaternion = nullptr;		//四元數指標

	witDataQuaternion witEyes_acceleration_quaternion;	//眼睛加速度四元數
	witDataQuaternion witHead_acceleration_quaternion;	//頭部加速度四元數
	witDataQuaternion *acceleration_quaternion = nullptr;	//加速度四元數指標

	witDataQuaternion relative_quaternion;		//眼睛四元數差值
	witDataAngle relative_angle;

	witDataAcceleration witEyes_acceleration;	//眼睛加速度
	witDataAcceleration witHead_acceleration;	//頭部加速度
	witDataAcceleration *acceleration = nullptr;	//加速度指標

	uint8_t witEyes_angle_status = 0;	//眼睛角度狀態
	uint8_t witHead_angle_status = 0;	//頭部角度狀態
	uint8_t *angle_status = nullptr;	//角度狀態指標

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

				/* 眼睛 */
				if(wit_data.serialPort == SERIAL1)	
				{
					reference_quaternion = &witEyes_reference_quaternion;		//設置參考四元數
					quaternion = &witEyes_quaternion;							//設置四元數
					angle_status = &witEyes_angle_status;						//設置角度狀態
					acceleration_quaternion = &witEyes_acceleration_quaternion;	//設置加速度四元數
					acceleration = &witEyes_acceleration;						//設置加速度
				}

				/* 頭部 */
				else if(wit_data.serialPort == SERIAL2)	
				{
					reference_quaternion = &witHead_reference_quaternion;		//設置參考四元數
					quaternion = &witHead_quaternion;							//設置四元數
					angle_status = &witHead_angle_status;						//設置角度狀態
					acceleration_quaternion = &witHead_acceleration_quaternion;	//設置加速度四元數
					acceleration = &witHead_acceleration;						//設置加速度
				}

				if(reference_quaternion && quaternion && angle_status && acceleration && acceleration_quaternion)	//防止野指標
				{
					/* 設置參考角度 */
					if(*angle_status == 0)
					{
						reference_quaternion->wquaternion = wit_data.wquaternion;	//設置參考四元數
						reference_quaternion->xquaternion = wit_data.xquaternion;
						reference_quaternion->yquaternion = wit_data.yquaternion;
						reference_quaternion->zquaternion = wit_data.zquaternion;
						*angle_status = 1;											//設置角度狀態
					}

					/* 處理參考角度 */
					quaternion->wquaternion = wit_data.wquaternion;		//設置四元數
					quaternion->xquaternion = wit_data.xquaternion;
					quaternion->yquaternion = wit_data.yquaternion;
					quaternion->zquaternion = wit_data.zquaternion;

					*quaternion = IMUAngle::quaternion_multiply(*quaternion, IMUAngle::quaternion_conjugate(*reference_quaternion));	//處理參考四元數

					/* 處理加速度 */
					acceleration_quaternion->wquaternion = 0;						//設置加速度四元數
					acceleration_quaternion->xquaternion = wit_data.xacceleration;
					acceleration_quaternion->yquaternion = wit_data.yacceleration;
					acceleration_quaternion->zquaternion = wit_data.zacceleration;

					witDataQuaternion acceleration_world_quaternion = IMUAngle::quaternion_multiply(IMUAngle::quaternion_multiply(*reference_quaternion, *acceleration_quaternion),IMUAngle::quaternion_conjugate(*reference_quaternion));	//計算加速度四元數
					acceleration->xacceleration = acceleration_world_quaternion.xquaternion;	//設置加速度
					acceleration->yacceleration = acceleration_world_quaternion.yquaternion;
					acceleration->zacceleration = acceleration_world_quaternion.zquaternion;
				}
				break;
			default:
				break;
		}
		
		/* 差角計算 */
		if(witEyes_angle_status == 1 && witHead_angle_status == 1)	//眼睛和頭部數據都獲取完成
		{
			relative_quaternion = IMUAngle::quaternion_multiply(witEyes_quaternion, IMUAngle::quaternion_conjugate(witHead_quaternion));	//計算眼睛和頭部的四元數差值
			relative_angle = IMUAngle::quaternion_to_euler(relative_quaternion);	//計算眼睛和頭部的角度差值
			xQueueSend(wit_data_relative_angle_quene, &relative_angle, 0);	//從佇列中獲取數據
		}
	}
}

/* 畫眼睛 */
void taskGC9A01(void *arg)
{
	/* 初始化GC9A01 */
	gc9a01.GC9A01_init();	//初始化GC9A01
	gc9a01.GC9A01_setEyes_r(80, 1, 10);	//設置眼睛半徑
	uint8_t is_GC9A01_update_finish = 0;	//GC9A01更新狀態
	while (1)
	{

		/* 獲取GC9A01數據 */
		GC9A01_data gc9a01_data;

		if(is_GC9A01_update_finish)	//如果更新完成
		{
			
			if(xQueueReceive(gc9a01_data_quene, &gc9a01_data, portMAX_DELAY) == pdTRUE)
			{
				gc9a01.GC9A01_setEyes_r(gc9a01_data.R, gc9a01_data.zetaR, gc9a01_data.omega_nR);								//設置眼睛半徑
				gc9a01.GC9A01_setEyes_lightMax(gc9a01_data.lightMax, gc9a01_data.zetaLightMax, gc9a01_data.omega_nLightMax);	//設置光暈最大值
			}
		}
		else						//沒有完成更新
		{
			if(xQueueReceive(gc9a01_data_quene, &gc9a01_data, 0) == pdTRUE)	
			{
				gc9a01.GC9A01_setEyes_r(gc9a01_data.R, gc9a01_data.zetaR, gc9a01_data.omega_nR);								//設置眼睛半徑
				gc9a01.GC9A01_setEyes_lightMax(gc9a01_data.lightMax, gc9a01_data.zetaLightMax, gc9a01_data.omega_nLightMax);	//設置光暈最大值
			}
		}
		is_GC9A01_update_finish = gc9a01.GC9A01_update();	//更新GC9A01
	}
}

/* 眼睛移動 */
void taskEyesMove(void *arg)
{
	witDataAngle witEyes_data;	//眼睛數據結構體
	int8_t eyes_x = 0;
	int8_t eyes_y = 0;

	/* 初始化眼睛 */
	eyesmove.eyesMove_init();	//初始化眼睛
	while (1)
	{
		if(xQueueReceive(wit_data_relative_angle_quene, &witEyes_data, portMAX_DELAY) == pdTRUE)	//從佇列中獲取數據
		{

			/* x角度範圍 */
			if(witEyes_data.zangle < -15 && witEyes_data.zangle > 15)
			{
				eyes_x = 0;
			}
			else if(witEyes_data.zangle > 15 && witEyes_data.zangle < 60)
			{
				eyes_x = map(witEyes_data.zangle, 15, 60, -0, -35);
			}
			else if(witEyes_data.zangle < -15 && witEyes_data.zangle > -60)
			{
				eyes_x = map(witEyes_data.zangle, -15, -60, 0, 35);
			}
			else if(witEyes_data.zangle > 60)
			{
				eyes_x = -35;
			}
			else if(witEyes_data.zangle < -60)
			{
				eyes_x = 35;
			}


			/* y角度範圍 */
			double *y_angle = (double *)(abs(witEyes_data.yangle) > abs(witEyes_data.xangle) ? &witEyes_data.yangle : &witEyes_data.xangle);	//獲取y角度
			if(*y_angle < -10 && *y_angle > 10)
			{
				eyes_y = 0;
			}
			else if(*y_angle > 10 && *y_angle < 50)
			{
				eyes_y = map(*y_angle, 10, 50, 0, 50);
			}
			else if(*y_angle < -10 && *y_angle > -50)
			{
				eyes_y = map(*y_angle, -10, -50, 0, -50);
			}
			else if(*y_angle > 50)
			{
				eyes_y = 50;
			}
			else if(*y_angle < -50)
			{
				eyes_y = -50;
			}



			eyesmove.eyesMove_angle(60, eyes_x, eyes_y);	//設置眼睛角度
		}
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
	wit_data_quene = xQueueCreate(10, sizeof(witData));				//建立佇列，長度10，大小為witData結構體大小
	if (wit_data_quene == NULL)			//佇列建立失敗
	{
		Serial.println("wit_data quene create error");
		while (2)
		{
			vTaskDelay(1000);
		}
	}
	wit_data_relative_angle_quene = xQueueCreate(10, sizeof(witDataAngle));	//建立佇列，長度10，大小為witDataAngle結構體大小
	if (wit_data_relative_angle_quene == NULL)	//佇列建立失敗
	{
		Serial.println("wit_data_diff quene create error");
		while (2)
		{
			vTaskDelay(1000);
		}
	}
	gc9a01_data_quene = xQueueCreate(10, sizeof(GC9A01_data));	//建立佇列，長度10，大小為GC9A01_data結構體大小
	if (gc9a01_data_quene == NULL)	//佇列建立失敗
	{
		Serial.println("gc9a01_data quene create error");
		while (2)
		{
			vTaskDelay(1000);
		}
	}
	Serial.println("quene create success");	//打印佇列建立成功狀態

	Serial.println("wit init...");	//打印初始化狀態
	xTaskCreatePinnedToCore(taskWitGetData, "taskWitEyesGetData", 4096, &witEyes, 1, &taskWitEyesGetData_hamdle, 1);	//創建獲取數據任務
	xTaskCreatePinnedToCore(taskWitGetData, "taskWitHeadGetData", 4096, &witHead, 1, &taskWitHeadGetData_hamdle, 1);	//創建獲取數據任務
	xTaskCreatePinnedToCore(taskWitPProcessingData, "taskWitPProcessingData", 4096, NULL, 1, &taskWitPProcessingData_hamdle, 1);	//創建數據處理任務
	xTaskCreatePinnedToCore(taskGC9A01, "taskGC9A01", 8192, NULL, 1, &taskGC9A01_hamdle, 0);		//創建GC9A01任務
	xTaskCreatePinnedToCore(taskEyesMove, "taskEyesMove", 4096, NULL, 1, &taskEyesMove_hamdle, 0);	//創建眼睛移動任務
}

void loop()
{
	vTaskDelay(1000);
}