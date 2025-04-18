#ifndef IMU_ANGLE_H
#define IMU_ANGLE_H

#include <Arduino.h>
#include "wit.h"

class IMUAngle 
{
	public:
		// 四元數操作
		static witDataQuaternion quaternion_conjugate(const witDataQuaternion &q);
		static witDataQuaternion quaternion_multiply(const witDataQuaternion &q1, const witDataQuaternion &q2);
		static witDataAngle quaternion_to_euler(const witDataQuaternion &q);	//四元數轉歐拉角
};

#endif
