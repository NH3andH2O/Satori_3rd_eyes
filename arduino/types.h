#ifndef TYPES_H
#define TYPES_H
#include <Arduino.h>
#include "wit.h"

// wit處理數據結構體
typedef struct
{
	witDataAngle relative_angle;			   // 角度差
	witDataAngularSpeed relative_angularSpeed; // 角速度差
	witDataAcceleration witEyes_acceleration;  // 眼睛加速度
	witDataAcceleration witHead_acceleration;  // 頭部加速度
} witPProcessingData;

// GC9A01數據結構體
typedef struct
{
	uint8_t R;
	double zetaR;
	double omega_nR;
	uint8_t lightMax;
	double zetaLightMax;
	double omega_nLightMax;
} GC9A01_data;

// 眼睛數據結構體
typedef struct
{
	uint8_t eyelid_angle; // 眼睛張開角度
	int8_t x_angle;		  // x角度
	int8_t y_angle;		  // y角度
} eyesMove_data;

// 網絡數據結構體
typedef struct
{
	int8_t x; // x
	int8_t y; // y
} network_control_data;

#endif