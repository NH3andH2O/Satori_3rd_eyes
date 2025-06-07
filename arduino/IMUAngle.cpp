#include "IMUAngle.h"

/* 四元數共軛 */
witDataQuaternion IMUAngle::quaternion_conjugate(const witDataQuaternion &q)
{
	witDataQuaternion result;
	result.wquaternion = q.wquaternion;
	result.xquaternion = -q.xquaternion;
	result.yquaternion = -q.yquaternion;
	result.zquaternion = -q.zquaternion;
	return result;
}

/* 四元數相乘 */
witDataQuaternion IMUAngle::quaternion_multiply(const witDataQuaternion &q1, const witDataQuaternion &q2)
{
	witDataQuaternion result;
	result.wquaternion = q1.wquaternion * q2.wquaternion - q1.xquaternion * q2.xquaternion - q1.yquaternion * q2.yquaternion - q1.zquaternion * q2.zquaternion;
	result.xquaternion = q1.wquaternion * q2.xquaternion + q1.xquaternion * q2.wquaternion + q1.yquaternion * q2.zquaternion - q1.zquaternion * q2.yquaternion;
	result.yquaternion = q1.wquaternion * q2.yquaternion - q1.xquaternion * q2.zquaternion + q1.yquaternion * q2.wquaternion + q1.zquaternion * q2.xquaternion;
	result.zquaternion = q1.wquaternion * q2.zquaternion + q1.xquaternion * q2.yquaternion - q1.yquaternion * q2.xquaternion + q1.zquaternion * q2.wquaternion;
	return result;
}

/* 四元數轉歐拉角 */
witDataAngle IMUAngle::quaternion_to_euler(const witDataQuaternion &q)
{
	witDataAngle result;
	result.xangle = atan2(2 * (q.wquaternion * q.xquaternion + q.yquaternion * q.zquaternion), 1 - 2 * (q.xquaternion * q.xquaternion + q.yquaternion * q.yquaternion)) * 180 / M_PI;	//x軸角度
	result.yangle = asin(2 * (q.wquaternion * q.yquaternion - q.zquaternion * q.xquaternion)) * 180 / M_PI;	//y軸角度
	result.zangle = atan2(2 * (q.wquaternion * q.zquaternion + q.xquaternion * q.yquaternion), 1 - 2 * (q.yquaternion * q.yquaternion + q.zquaternion * q.zquaternion)) * 180 / M_PI;	//z軸角度
	return result;
}
  