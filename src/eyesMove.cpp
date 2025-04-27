#include "eyesMove.h"

eyesMove::eyesMove(uint8_t upper_eyelid_pin, uint8_t lower_eyelid_pin, uint8_t eyeball_pin)
{
	this->upper_eyelid_pin = upper_eyelid_pin;
	this->lower_eyelid_pin = lower_eyelid_pin;
	this->eyeball_pin = eyeball_pin;
}

void eyesMove::eyesMove_init()
{
	upper_eyelid_servo.attach(upper_eyelid_pin);
	lower_eyelid_servo.attach(lower_eyelid_pin);
	eyeball_servo.attach(eyeball_pin);

	upper_eyelid_servo.write(110);
	lower_eyelid_servo.write(95);
	eyeball_servo.write(90);
}

void eyesMove::eyesMove_servo(uint8_t upper_eyelid_angle, uint8_t lower_eyelid_angle, uint8_t eyeball_angle)
{

	/* 角度輸入超限更正 */
	if(upper_eyelid_angle < UPPER_EYELID_ANGLE_MIN)
	{
	upper_eyelid_angle = UPPER_EYELID_ANGLE_MIN;
	}
	else if(upper_eyelid_angle > UPPER_EYELID_ANGLE_MAX)
	{
		upper_eyelid_angle = UPPER_EYELID_ANGLE_MAX;
	}
	if(lower_eyelid_angle < LOWER_EYELID_ANGLE_MIN)
	{
		lower_eyelid_angle = LOWER_EYELID_ANGLE_MIN;
	}
	else if(lower_eyelid_angle > LOWER_EYELID_ANGLE_MAX)
	{
		lower_eyelid_angle = LOWER_EYELID_ANGLE_MAX;
	}
	if(eyeball_angle < EYEBALL_ANGLE_MIN)
	{
		eyeball_angle = EYEBALL_ANGLE_MIN;
	}
	else if(eyeball_angle > EYEBALL_ANGLE_MAX)
	{
		eyeball_angle = EYEBALL_ANGLE_MAX;
	}

	/* 眼皮輸出 */
	upper_eyelid_servo.write(upper_eyelid_angle);
	lower_eyelid_servo.write(lower_eyelid_angle);
	eyeball_servo.write(eyeball_angle);
}

void eyesMove::eyesMove_angle(int8_t eyelid_angle, int8_t x_angle, int8_t y_angle)
{
	/* 眼睛張開輸入超限更正 */
	if(eyelid_angle < 0)
	{
		eyelid_angle = 0;
	}
	else if(eyelid_angle > 80)
	{
		eyelid_angle = 80;
	}
	
	/* y輸入超限更正 */
	if(eyelid_angle <= 80 && abs(y_angle) > eyelid_angle)
	{
		y_angle = (y_angle > 0) ? eyelid_angle : -eyelid_angle;
	}
	else if(eyelid_angle > 80 && abs(y_angle) > abs(eyelid_angle - 80))
	{
		y_angle = (y_angle > 0) ? abs(eyelid_angle - 80) : -abs(eyelid_angle - 80);
	}

	/* x輸入超限更正 */
	if(x_angle < -55)
	{
		x_angle = -55;
	}
	else if(x_angle > 55)
	{
		x_angle = 55;
	}

	/* 眼皮輸出角度确定 */
	uint8_t upper_eyelid_angle = UPPER_EYELID_ANGLE_MAX - eyelid_angle - map(y_angle, 0, 80, 0, UPPER_EYELID_ANGLE_MAX - UPPER_EYELID_ANGLE_MIN);	//上眼皮角度
	uint8_t lower_eyelid_angle = LOWER_EYELID_ANGLE_MIN + eyelid_angle - map(y_angle, 0, 80, 0, LOWER_EYELID_ANGLE_MAX - LOWER_EYELID_ANGLE_MIN);	//下眼皮角度
	uint8_t eyeball_angle = (EYEBALL_ANGLE_MAX + EYEBALL_ANGLE_MIN) / 2 + x_angle;											//眼球角度

	/* 眼皮輸出 */
	eyesMove_servo(upper_eyelid_angle, lower_eyelid_angle, eyeball_angle);	//設置眼皮角度

}