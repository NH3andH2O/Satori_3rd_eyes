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

	pid_eyelid_angle.set_pid(Kp, Ki, Kd); // 設置眼睛張開角度pid
	pid_x_angle.set_pid(Kp, Ki, Kd);	  // 設置x角度pid
	pid_y_angle.set_pid(Kp, Ki, Kd);	  // 設置y角度pid

	eyesMove_angle(0, 0, 0); // 設置眼睛角度
}

void eyesMove::eyesMove_servo(uint8_t upper_eyelid_angle, uint8_t lower_eyelid_angle, uint8_t eyeball_angle)
{

	/* 角度輸入超限更正 */
	upper_eyelid_angle = constrain(upper_eyelid_angle, this->UPPER_EYELID_ANGLE_MIN, this->UPPER_EYELID_ANGLE_MAX); // 上眼皮角度
	lower_eyelid_angle = constrain(lower_eyelid_angle, this->LOWER_EYELID_ANGLE_MIN, this->LOWER_EYELID_ANGLE_MAX); // 下眼皮角度
	eyeball_angle = constrain(eyeball_angle, this->EYEBALL_ANGLE_MIN, this->EYEBALL_ANGLE_MAX);						// 眼球角度

	/* 眼皮輸出 */
	this->upper_eyelid_servo.write(upper_eyelid_angle);
	this->lower_eyelid_servo.write(lower_eyelid_angle);
	this->eyeball_servo.write(eyeball_angle);
}

void eyesMove::eyesMove_angle(int8_t eyelid_angle, int8_t x_angle, int8_t y_angle)
{
	/* 眼睛張開輸入超限更正 */
	eyelid_angle = constrain(eyelid_angle, 0, 80); // 眼睛張開角度

	/* y輸入超限更正 */
	if (eyelid_angle <= 40 && abs(y_angle) > eyelid_angle)
	{
		y_angle = (y_angle > 0) ? eyelid_angle : -eyelid_angle;
	}
	else if (eyelid_angle > 40 && abs(y_angle) > abs(eyelid_angle - 80))
	{
		y_angle = (y_angle > 0) ? abs(eyelid_angle - 80) : -abs(eyelid_angle - 80);
	}

	/* x輸入超限更正 */
	x_angle = constrain(x_angle, -55, 55); // x角度

	/* 眼皮輸出角度确定 */
	uint8_t upper_eyelid_angle = this->UPPER_EYELID_ANGLE_MAX - eyelid_angle -
								 map(y_angle, 0, 80, 0, this->UPPER_EYELID_ANGLE_MAX - this->UPPER_EYELID_ANGLE_MIN); // 上眼皮角度
	uint8_t lower_eyelid_angle = this->LOWER_EYELID_ANGLE_MIN + eyelid_angle -
								 map(y_angle, 0, 80, 0, this->LOWER_EYELID_ANGLE_MAX - this->LOWER_EYELID_ANGLE_MIN); // 下眼皮角度
	uint8_t eyeball_angle = (this->EYEBALL_ANGLE_MAX + this->EYEBALL_ANGLE_MIN) / 2 + x_angle;						  // 眼球角度

	/* 眼皮輸出 */
	this->eyesMove_servo(upper_eyelid_angle, lower_eyelid_angle, eyeball_angle); // 設置眼皮角度
}

void eyesMove::eyesMove_angle_set(int8_t eyelid_angle, int8_t x_angle, int8_t y_angle)
{
	if ((this->target_eyelid_angle != eyelid_angle || this->target_x_angle != x_angle || this->target_y_angle != y_angle) &&
		!isUpdate) // 如果目標角度不等於當前角度
	{
		this->lastUpdate = xTaskGetTickCount(); // 設置更新時間
		this->lastChange = xTaskGetTickCount(); // 設置上次更新時間
		this->isUpdate = 1;						// 設置更新狀態為1
	}
	this->target_eyelid_angle = eyelid_angle; // 設置眼睛張開角度
	this->target_x_angle = x_angle;			  // 設置x角度
	this->target_y_angle = y_angle;			  // 設置y角度
}

void eyesMove::eyesMove_angle_pid(double kp, double ki, double kd)
{
	/* 更新pid */
	this->Kp = kp; // 設置比例增益
	this->Ki = ki; // 設置積分增益
	this->Kd = kd; // 設置微分增益

	/* 重置pid */
	this->pid_eyelid_angle.reset(); // 重置眼睛張開角度pid
	this->pid_x_angle.reset();		// 重置x角度pid
	this->pid_y_angle.reset();		// 重置y角度pid

	/* 更新時間 */
	this->lastUpdate = xTaskGetTickCount();

	/* 設置pid */
	this->pid_eyelid_angle.set_pid(this->Kp, this->Ki, this->Kd); // 設置眼睛張開角度pid
	this->pid_x_angle.set_pid(this->Kp, this->Ki, this->Kd);	  // 設置x角度pid
	this->pid_y_angle.set_pid(this->Kp, this->Ki, this->Kd);	  // 設置y角度pid
}

uint8_t eyesMove::eyesMove_update()
{
	if (isUpdate)
	{
		/* 獲取時間差 */
		u_int64_t now = xTaskGetTickCount();		   // 當前時間
		double dt = (now - this->lastUpdate) / 1000.0; // 時間差
		if (dt >= 0.02)
		{
			this->lastUpdate = now; // 更新時間

			/* 时间过久重置pid */
			if (dt > 0.4)
			{
				this->pid_eyelid_angle.reset();
				this->pid_x_angle.reset();
				this->pid_y_angle.reset();
			}

			/* 更新角度 */
			this->eyelid_angle = this->pid_eyelid_angle.compute(this->target_eyelid_angle, this->eyelid_angle, dt); // 計算眼睛張開角度pid
			this->x_angle = this->pid_x_angle.compute(this->target_x_angle, this->x_angle, dt);						// 計算x角度pid
			this->y_angle = this->pid_y_angle.compute(this->target_y_angle, this->y_angle, dt);						// 計算y角度pid

			/* 檢查角度墻 */
			this->eyelid_angle_int = round(constrain(this->eyelid_angle, (double)0, (double)80)); // 眼睛張開角度整數化
			this->x_angle_int = round(constrain(this->x_angle, (double)-55, (double)55));		  // x角度整數化
			this->y_angle_int = round(constrain(this->y_angle, (double)-80, (double)80));		  // y角度整數化

			/* 設置眼睛角度 */
			this->eyesMove_angle(this->eyelid_angle_int, this->x_angle_int, this->y_angle_int); // 設置眼睛角度
		}

		/* 檢查更新狀態 */
		if (this->target_eyelid_angle == round(this->eyelid_angle) && this->target_x_angle == round(this->x_angle) &&
			this->target_y_angle == round(this->y_angle)) // 如果眼睛張開角度和x角度和y角度都達到目標值
		{
			;
		}
		else
		{
			this->lastChange = now; // 更新時間
		}
		if ((now - this->lastChange) > this->LASTCHANGEMAX) // 如果時間差大於LASTCHANGEMAX
		{
			this->isUpdate = 0; // 設置更新狀態為0
			return 1;			// 完成更新
		}
		else
		{
			return 0; // 未完成更新
		}
	}
	else
	{
		return 1; // 完成更新
	}
}