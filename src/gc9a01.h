#ifndef GC9A01_H
#define GC9A01_H

#include <Arduino.h>
#include <LovyanGFX.hpp>

class LGFX_GC9A01 : public lgfx::LGFX_Device
{
	private:
		lgfx::Panel_GC9A01 panel;					//面板實例
		lgfx::Panel_GC9A01::config_t panel_config;	//面板實例配置

		lgfx::Bus_SPI bus; 							//SPI總線實例
		lgfx::Bus_SPI::config_t bus_config;		    //SPI總線實例配置
		
	public:
		LGFX_GC9A01();
		void LGFX_GC9A01_config(uint8_t MOSIPin, uint8_t SCLKPin, uint8_t CSPin, uint8_t DCPin, uint8_t RSTPin, uint8_t BLPin);     //初始化
};

class GC9A01
{
	private:

		const uint8_t lastChangeMax = 20;


		LGFX_GC9A01 myLGFX;		//LGFX_GC9A01實例
		LGFX_Sprite mySprite;	//LGFX_Sprite實例

		/* 引脚變數 */
		uint8_t MOSIPin;
		uint8_t SCLKPin;
		uint8_t CSPin;
		uint8_t DCPin;
		uint8_t RSTPin;
		uint8_t BLPin;

		/* 眼睛變數 */
		double eyesR = 80;							//眼睛半徑
		uint8_t lastChange_eyesR = lastChangeMax;	//目標眼睛半徑
		double target_eyesR = 80;					//目标眼睛半徑
		double zeta_eyesR = 1;						//眼睛半徑阻尼比
		double omega_n_eyesR = 5;					//眼睛半徑自然頻率
		double deyesR = 0;							//眼睛半徑變化量
		u_int64_t lastUpdate_eyesR = 0;				//眼睛半徑更新時間

		/* 亮度變數 */
		double lightMax = 255;							//光暈最大值
		uint8_t lastChange_lightMax = lastChangeMax;	//光暈變化次數
		double target_lightMax = 255;					//目标光暈最大值
		double zeta_lightMax = 255;						//光暈最大值阻尼比
		double omega_n_lightMax = 5;					//光暈最大值自然頻率
		double dlightMax = 0;							//光暈最大值變化量
		u_int64_t lastUpdate_lightMax = 0;				//光暈最大值更新時間

	public:
		GC9A01(uint8_t MOSIPin, uint8_t SCLKPin, uint8_t CSPin, uint8_t DCPin, uint8_t RSTPin, uint8_t BLPin);
		void GC9A01_init();																//初始化
		void GC9A01_setEyes_r(uint8_t target_eyes_r, double zeta, double omega_n);		//設置眼睛半徑
		void GC9A01_setEyes_lightMax(uint8_t lightMax, double zeta, double omega_n);	//設置眼睛光暈最大值
		uint8_t GC9A01_update();														//更新
};

#endif