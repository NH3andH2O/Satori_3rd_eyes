#ifndef eyesMove_h
#define eyesMove_h
#include <ESP32Servo.h>
#include <Arduino.h>

class eyesMove
{
private:
	Servo upper_eyelid_servo;	//上眼皮伺服馬達限位：100°-180°
	Servo lower_eyelid_servo;	//下眼皮伺服馬達限位：55°-115°
	Servo eyeball_servo;		//眼球伺服馬達限位：55°-125°
public:
	/* 伺服馬達引脚 */
	uint8_t upper_eyelid_pin;
	uint8_t lower_eyelid_pin;
	uint8_t eyeball_pin;

	void eyesMove_servo(uint8_t upper_eyelid_angle, uint8_t lower_eyelid_angle, uint8_t eyeball_angle);
	void eyesMove_angle(int8_t eyelid_angle, int8_t x_angle, int8_t y_angle);

	//初始化眼睛
	void eyesMove_init();
	eyesMove(uint8_t upper_eyelid_pin, uint8_t lower_eyelid_pin, uint8_t eyeball_pin);
};
#endif