#ifndef eyesMove_h
#define eyesMove_h
#include <ESP32Servo.h>
#include <Arduino.h>

#define UPPER_EYELID_PIN 12	//上眼皮伺服馬達引脚
#define LOWER_EYELID_PIN 13	//下眼皮伺服馬達引脚
#define EYEBALL_PIN 14		//眼球伺服馬達引脚

class eyesMove
{
private:
	Servo upper_eyelid_servo;	//上眼皮伺服馬達限位：100°-180°
	Servo lower_eyelid_servo;	//下眼皮伺服馬達限位：55°-115°
	Servo eyeball_servo;		//眼球伺服馬達限位：55°-125°
public:
	void eyesMove_servo(uint8_t upper_eyelid_angle, uint8_t lower_eyelid_angle, uint8_t eyeball_angle);
	void eyesMove_angle(int8_t eyelid_angle, int8_t x_angle, int8_t y_angle);
	eyesMove();
};
#endif