#include "pid_controller.hpp"

#include <cassert>

PidController::PidController(double kp, double ki, double kd) : kp_(kp), ki_(ki), kd_(kd), integral_(0.0), previous_error_(0.0) {}

void PidController::setGains(double kp, double ki, double kd)
{
	kp_ = kp;
	ki_ = ki;
	kd_ = kd;
}

void PidController::reset()
{
	integral_ = 0.0;
	previous_error_ = 0.0;
}

double PidController::update(double setpoint, double measurement, double dt_seconds)
{
	assert(dt_seconds > 0.0);

	const double error = setpoint - measurement;
	integral_ += error * dt_seconds;
	const double derivative = (error - previous_error_) / dt_seconds;
	previous_error_ = error;

	return kp_ * error + ki_ * integral_ + kd_ * derivative;
}
