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
		LGFX_GC9A01 myLGFX;	//LGFX_GC9A01實例

		/* 引脚變數 */
		uint8_t MOSIPin;
		uint8_t SCLKPin;
		uint8_t CSPin;
		uint8_t DCPin;
		uint8_t RSTPin;
		uint8_t BLPin;

	public:
		GC9A01(uint8_t MOSIPin, uint8_t SCLKPin, uint8_t CSPin, uint8_t DCPin, uint8_t RSTPin, uint8_t BLPin);
		void GC9A01_init();	//初始化
		void fillScreen();	//清屏
};

#endif