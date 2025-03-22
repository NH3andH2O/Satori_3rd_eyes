#include <ESP32Servo.h>
#include "eyesMove.h"

#define EYESLID_CONTROL_PIN A0	//眼皮控制引脚
#define X_ANGLE_CONTROL_PIN A1	//X軸角度控制引脚
#define Y_ANGLE_CONTROL_PIN A3	//Y軸角度控制引脚
#define EYES_SHAKE_PIN A2	//眼睛震動控制引脚

#define CENTRE_DEAD_ZONE 400 //中心死區半徑
#define LOWER_DEAD_ZONE 50 //下死區半徑
#define CENTER_VALUE 2048 //中心值

eyesMove eyesmove;
uint16_t eyelid_angle = 0;	//眼皮角度
uint8_t eyes_shake_state = 0;	//眼睛震動狀態

void setup()
{
	eyesmove.eyesMove_servo(180, 55, 90);
	analogSetAttenuation(ADC_11db);
}

void loop()
{
	eyelid_angle = analogRead(EYESLID_CONTROL_PIN);
	int16_t x_angle = analogRead(X_ANGLE_CONTROL_PIN);
	int16_t y_angle = analogRead(Y_ANGLE_CONTROL_PIN);

	if(x_angle > CENTER_VALUE + CENTRE_DEAD_ZONE)
	{
		x_angle = map(x_angle, CENTER_VALUE + CENTRE_DEAD_ZONE, 4095, 0, -35);
	}
	else if (x_angle < CENTER_VALUE - CENTRE_DEAD_ZONE)
	{
		x_angle = map(x_angle, LOWER_DEAD_ZONE, CENTER_VALUE - CENTRE_DEAD_ZONE, 35, 0);
	}
	else if(x_angle <= LOWER_DEAD_ZONE)
	{
		x_angle = 35;
	}
	else
	{
		x_angle = 0;
	}
	
	/*角度控制*/
	if(y_angle > CENTER_VALUE + CENTRE_DEAD_ZONE)
	{
		y_angle = map(y_angle, CENTER_VALUE + CENTRE_DEAD_ZONE, 4095, 0, 50);
	}
	else if (y_angle < CENTER_VALUE - CENTRE_DEAD_ZONE)
	{
		y_angle = map(y_angle, LOWER_DEAD_ZONE, CENTER_VALUE - CENTRE_DEAD_ZONE, -50, 0);
	}
	else if(y_angle <= LOWER_DEAD_ZONE)
	{
		y_angle = -50;
	}
	else
	{
		y_angle = 0;
	}

	if(digitalRead(EYES_SHAKE_PIN) == LOW && eyes_shake_state == 0)
	{
		eyes_shake_state = 1;
		x_angle = x_angle-2;
	}
	else if(digitalRead(EYES_SHAKE_PIN) == LOW && eyes_shake_state == 1)
	{
		eyes_shake_state = 0;
		x_angle = x_angle+2;
	}

	if(eyelid_angle<200)
	{
		eyelid_angle=200;
	}
	eyelid_angle = map(eyelid_angle, 200, 4095, 0, 96);
	eyesmove.eyesMove_angle(eyelid_angle, x_angle, y_angle);
}