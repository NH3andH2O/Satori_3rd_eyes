#include "gc9a01.h"

LGFX_GC9A01::LGFX_GC9A01()
{
	;
}

void LGFX_GC9A01::LGFX_GC9A01_config(uint8_t MOSIPin, uint8_t SCLKPin, uint8_t CSPin, uint8_t DCPin, uint8_t RSTPin, uint8_t BLPin)
{
	/* 設定SPI */
	bus_config.spi_host = SPI2_HOST;	//設置SPI總線
	bus_config.spi_mode = 0;			//設置SPI模式
	bus_config.spi_3wire = false;		//設置SPI三線模式
	bus_config.use_lock = true;			//設置SPI鎖定
	bus_config.freq_read = 16000000;	//設置SPI讀取頻率
	bus_config.freq_write = 40000000;	//設置SPI寫入頻率
	bus_config.pin_dc = DCPin;			//設置DC引脚
	bus_config.pin_sclk = SCLKPin;		//設置SCLK引脚
	bus_config.pin_mosi = MOSIPin;		//設置MOSI引脚
	bus_config.pin_miso = -1;			//設置MISO引脚
	bus_config.dma_channel = 1;			//設置DMA通道
	bus.config(bus_config);	//設置SPI總線配置

	panel_config.pin_cs = CSPin;		//設置CS引脚
	panel_config.pin_rst = RSTPin;		//設置RST引脚
	panel_config.pin_busy = -1;			//設置BL引脚
	panel_config.panel_width = 240;		//設置面板寬度
	panel_config.panel_height = 240;	//設置面板高度
	panel_config.memory_width = 240;	//設置內存寬度
	panel_config.memory_height = 240;	//設置內存高度
	panel_config.offset_x = 0;			//設置X偏移量
	panel_config.offset_y = 0;			//設置Y偏移量
	panel_config.offset_rotation = 0;	//設置旋轉偏移量
	panel_config.dummy_read_pixel = 8;	//設置讀取像素的空閒時間
	panel_config.dummy_read_bits = 1;	//設置讀取位的空閒時間
	panel_config.readable = false;		//設置可讀性
	panel_config.invert = true;			//設置反轉
	panel_config.rgb_order = false;		//設置RGB順序
	panel_config.dlen_16bit = false;	//設置16位數據長度
	panel_config.bus_shared = true;		//設置總線共享
	panel.config(panel_config);			//設置面板配置
	panel.setBus(&bus);					//設置面板寬度
	setPanel(&panel);					//設置面板

}


GC9A01::GC9A01(uint8_t MOSIPin, uint8_t SCLKPin, uint8_t CSPin, uint8_t DCPin, uint8_t RSTPin, uint8_t BLPin)
{
	/* 設定引脚 */
	this->MOSIPin = MOSIPin;	//MOSI引脚
	this->SCLKPin = SCLKPin;	//SCLK引脚
	this->CSPin = CSPin;		//CS引脚
	this->DCPin = DCPin;		//DC引脚
	this->RSTPin = RSTPin;		//RST引脚
	this->BLPin = BLPin;		//BL引脚
}

void GC9A01::GC9A01_init()
{
	myLGFX.LGFX_GC9A01_config(this->MOSIPin, this->SCLKPin, this->CSPin, this->DCPin, this->RSTPin, this->BLPin);	
	myLGFX.init();	//初始化
}

void GC9A01::fillScreen()
{
	myLGFX.fillScreen(myLGFX.color565(0, 0, 0));	//清屏
}