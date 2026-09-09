#include "pid_controller.hpp"

#include <cassert>

PidController::PidController(double kp, double ki, double kd) : kp_(kp), ki_(ki), kd_(kd), integral_(0.0), previous_error_(0.0) {}

/**
 * @brief 線上更新 PID 增益（調參）。
 *
 * @param kp 比例增益
 * @param ki 積分增益
 * @param kd 微分增益
 */
void PidController::setGains(double kp, double ki, double kd)
{
	kp_ = kp;
	ki_ = ki;
	kd_ = kd;
}

/**
 * @brief 清除控制器歷史狀態，增益保持不變。
 *
 */
void PidController::reset()
{
	integral_ = 0.0;
	previous_error_ = 0.0;
}

/**
 * @brief 執行一個控制週期，回傳 PID 輸出。
 *
 * @param setpoint 目標值
 * @param measurement 量測值（目前狀態）
 * @param dt_seconds 距上次呼叫的時間間隔（秒），必須 > 0；
 *                   僅在 debug build 斷言，release build 傳 0
 *                   會導致除以零（inf/nan）。
 * @return 本週期的控制輸出（未限幅）
 */
double PidController::update(double setpoint, double measurement, double dt_seconds)
{
	assert(dt_seconds > 0.0);

	const double error = setpoint - measurement;
	integral_ += error * dt_seconds;
	const double derivative = (error - previous_error_) / dt_seconds;
	previous_error_ = error;

	return kp_ * error + ki_ * integral_ + kd_ * derivative;
}
