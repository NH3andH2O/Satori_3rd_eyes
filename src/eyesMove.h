#ifndef eyesMove_h
#define eyesMove_h
#include <ESP32Servo.h>
#include <Arduino.h>

class eyesMove
{
private:

	/* 伺服馬達限位 */
	const uint8_t UPPER_EYELID_ANGLE_MAX = 175;	//上眼皮最大角度
	const uint8_t UPPER_EYELID_ANGLE_MIN = 100;	//上眼皮最小角度
	const uint8_t LOWER_EYELID_ANGLE_MAX = 90;	//下眼皮最大角度
	const uint8_t LOWER_EYELID_ANGLE_MIN = 35;	//下眼皮最小角度
	const uint8_t EYEBALL_ANGLE_MAX = 125;		//眼球最大角度
	const uint8_t EYEBALL_ANGLE_MIN = 55;		//眼球最小角度

	Servo upper_eyelid_servo;	//上眼皮伺服馬達
	Servo lower_eyelid_servo;	//下眼皮伺服馬達
	Servo eyeball_servo;		//眼球伺服馬達
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