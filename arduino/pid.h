#ifndef pid_h
#define pid_h
#include <Arduino.h>

class pid
{
  private:
	double kp, ki, kd;
	double integral;
	double lastError;

  public:
	pid();

	void set_pid(double kp, double ki, double kd);
	void reset();
	double compute(double target, double current, double dt);
};

#endif
