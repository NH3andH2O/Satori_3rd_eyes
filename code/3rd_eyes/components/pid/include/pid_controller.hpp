#pragma once

class PidController
{
  public:
	PidController(double kp, double ki, double kd);

	void setGains(double kp, double ki, double kd);
	void reset();
	[[nodiscard]] double update(double setpoint, double measurement, double dt_seconds);

  private:
	double kp_;
	double ki_;
	double kd_;
	double integral_;
	double previous_error_;
};
