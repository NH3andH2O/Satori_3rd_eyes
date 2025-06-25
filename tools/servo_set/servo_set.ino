#include <ESP32Servo.h>

#define UPPER_EYELID_PIN 13 // 上眼皮伺服馬達引脚
#define LOWER_EYELID_PIN 14 // 下眼皮伺服馬達引脚
#define EYEBALL_PIN 21		// 眼球伺服馬達引脚

Servo upper_eyelid_servo; // 上眼皮伺服馬達
Servo lower_eyelid_servo; // 下眼皮伺服馬達
Servo eyeball_servo;	  // 眼球伺服馬達

void setup()
{
	upper_eyelid_servo.attach(UPPER_EYELID_PIN); // 附加上眼皮伺服馬達
	lower_eyelid_servo.attach(LOWER_EYELID_PIN); // 附加下眼皮伺服馬達
	eyeball_servo.attach(EYEBALL_PIN);			 // 附加眼球伺服馬達

	// 設置伺服馬達的初始位置
	upper_eyelid_servo.write(90); // 上眼皮初始位置
	lower_eyelid_servo.write(90); // 下眼皮初始位置
	eyeball_servo.write(90);	  // 眼球初始位置
}

void loop()
{
}