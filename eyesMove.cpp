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
	/*角度輸入超限更正*/
	if(upper_eyelid_angle < 100)
	{
	upper_eyelid_angle = 100;
	}
	else if(upper_eyelid_angle > 180)
	{
		upper_eyelid_angle = 180;
	}
	if(lower_eyelid_angle < 55)
	{
		lower_eyelid_angle = 55;
	}
	else if(lower_eyelid_angle > 115)
	{
		lower_eyelid_angle = 115;
	}
	if(eyeball_angle < 55)
	{
		eyeball_angle = 55;
	}
	else if(eyeball_angle > 125)
	{
		eyeball_angle = 125;
	}
	upper_eyelid_servo.write(upper_eyelid_angle);
	lower_eyelid_servo.write(lower_eyelid_angle);
	eyeball_servo.write(eyeball_angle);
}

void eyesMove::eyesMove_angle(int8_t eyelid_angle, int8_t x_angle, int8_t y_angle)
{
	/*角度輸入超限更正*/
	if(eyelid_angle < 0)
	{
		eyelid_angle = 0;
	}
	else if(eyelid_angle > 96)
	{
		eyelid_angle = 96;
	}
	if(x_angle < -35)
	{
		x_angle = -35;
	}
	else if(x_angle > 35)
	{
		x_angle = 35;
	}
	if(y_angle < -50)
	{
		y_angle = -50;
	}
	else if(y_angle > 50)
	{
		y_angle = 50;
	}

	uint8_t upper_eyelid_angle;
	int8_t lower_eyelid_angle;
	int8_t eyeball_angle = map(x_angle, -35, 35, 55, 125);

	if(eyelid_angle <= 48)
	{
		y_angle = map(y_angle, 50, -50, eyelid_angle/2, -eyelid_angle/2);
	}
	else
	{
		y_angle = map(y_angle, 50, -50, (96-eyelid_angle)/2, (-96+eyelid_angle)/2);
	}
	lower_eyelid_angle = y_angle - eyelid_angle/2;
	upper_eyelid_angle = y_angle + eyelid_angle/2;

	lower_eyelid_angle = y_angle - eyelid_angle/2;
	upper_eyelid_angle = y_angle + eyelid_angle/2;

	if (lower_eyelid_angle < -48)
	{
		lower_eyelid_angle = -48;
	}
	if (upper_eyelid_angle > 48)
	{
		upper_eyelid_angle = 48;
	}

	lower_eyelid_angle =map(lower_eyelid_angle, 0, -48, 55, 115);
	upper_eyelid_angle =map(upper_eyelid_angle, 0, 48, 180, 100);

	upper_eyelid_servo.write(upper_eyelid_angle);
	lower_eyelid_servo.write(lower_eyelid_angle);
	eyeball_servo.write(eyeball_angle);

}