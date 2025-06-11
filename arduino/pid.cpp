#include "pid.h"

pid::pid() { reset(); }

void pid::set_pid(double kp, double ki, double kd)
{
	this->kp = kp;
	this->ki = ki;
	this->kd = kd;
}

void pid::reset()
{
	this->integral = 0;
	this->lastError = 0;
}

double pid::compute(double target, double current, double dt)
{
	if (dt <= 0.0f)
		dt = 0.001f; // 防止除以0

	double error = target - current;
	this->integral += error * dt;
	double derivative = (error - this->lastError) / dt;

	double pid = this->kp * error + this->ki * this->integral + this->kd * derivative;
	double output = current + pid * dt;

	this->lastError = error;
	return output;
}
