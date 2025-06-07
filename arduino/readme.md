# 机械觉之瞳代码

## 使用方法

1. 下载并安装arduino IDE
2. 打开`arduino.ino`
3. 安装esp32固件库
4. 安装`ESP32Servo(3.0.6)`和`LovyanGX(1.2.0)`程式库
5. 选择`ESP32S3 Dev Module (esp32)`,开发板设置如下（此为N16R8版本，其他版本的需要修改部分参数）：

    | 参数名           | 参数    |
    | --------------- | ------- |
    | PSRAM           | OPI PSRAM   |
    | Flash Mode       | QIO 120MHz  |
    | Flash Size       | 16M(128Mb)     |
    | Arduino Runs Core        | Core 1       |
    | Events Run On      | Core 0       |
    | USB Mode         | Hardware CDC and JTAG   |
    | USB CDC On Boot       | Disabled |
    | USB Firmware MSC On Boot       | Disabled |
    | USB DFU On Boot       | Disabled |
    | Upload Mode      | UART0 / Hardware CDC |
    | Partition Scheme | 16M Flash(3MB APP/9MB FATFS) |
    | CPU Frequency         | 240 MHz(WiFi)    |
    | Upload Speed     | 921600  |
    | Core Debug Level      | none    |


5. 连接开发板，从`arduino.ino`上传代码

## 注意事项

-   我使用的是arduino 1.8.x的版本，2.x.x的版本目前没有做过测试，请谨慎选择2.x.x的版本
