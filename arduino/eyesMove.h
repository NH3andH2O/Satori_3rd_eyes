#ifndef eyesMove_h
#define eyesMove_h
#include <ESP32Servo.h>
#include <Arduino.h>
#include "pid.h"

class eyesMove
{
  private:
	/* 眼睛移動參數 */
	const uint16_t LASTCHANGEMAX = 500;

	/* 伺服馬達限位 */
	const uint8_t UPPER_EYELID_ANGLE_MAX = 175; // 上眼皮最大角度
	const uint8_t UPPER_EYELID_ANGLE_MIN = 100; // 上眼皮最小角度
	const uint8_t LOWER_EYELID_ANGLE_MAX = 110; // 下眼皮最大角度
	const uint8_t LOWER_EYELID_ANGLE_MIN = 35;	// 下眼皮最小角度
	const uint8_t EYEBALL_ANGLE_MAX = 125;		// 眼球最大角度
	const uint8_t EYEBALL_ANGLE_MIN = 55;		// 眼球最小角度

	Servo upper_eyelid_servo; // 上眼皮伺服馬達
	Servo lower_eyelid_servo; // 下眼皮伺服馬達
	Servo eyeball_servo;	  // 眼球伺服馬達

	/* pid */
	double Kp = 1.0;  // 比例增益
	double Ki = 0.1;  // 積分增益
	double Kd = 0.05; // 微分增益

	pid pid_eyelid_angle; // 眼睛張開角度pid
	pid pid_x_angle;	  // x角度pid
	pid pid_y_angle;	  // y角度pid

	/* 眼睛參數 */
	int8_t target_eyelid_angle = 0; // 目標眼睛張開角度
	int8_t target_x_angle = 0;		// 目標x角度
	int8_t target_y_angle = 0;		// 目標y角度

	double eyelid_angle = 0; // 張開角度
	double x_angle = 0;		 // x角度
	double y_angle = 0;		 // y角度

	int8_t eyelid_angle_int = 0; // 張開角度整數
	int8_t x_angle_int = 0;		 // x角度整數
	int8_t y_angle_int = 0;		 // y角度整數

	/* 時間相關 */
	u_int64_t lastUpdate = 0;		 // 上次更新時間
	u_int64_t target_lastUpdate = 0; // 目標上次更新時間
	u_int64_t lastChange = 0;		 // 上次變化時間

	/* 更新相關 */
	u_int8_t isUpdate = 1; // 更新狀態

	void eyesMove_angle(int8_t eyelid_angle, int8_t x_angle, int8_t y_angle);
	void eyesMove_servo(uint8_t upper_eyelid_angle, uint8_t lower_eyelid_angle, uint8_t eyeball_angle);

  public:
	/* 伺服馬達引脚 */
	uint8_t upper_eyelid_pin;
	uint8_t lower_eyelid_pin;
	uint8_t eyeball_pin;

	void eyesMove_angle_set(int8_t eyelid_angle, int8_t x_angle, int8_t y_angle);
	void eyesMove_angle_pid(double kp, double ki, double kd);
	uint8_t eyesMove_update();

	// 初始化眼睛
	void eyesMove_init();
	eyesMove(uint8_t upper_eyelid_pin, uint8_t lower_eyelid_pin, uint8_t eyeball_pin);
};
#endif