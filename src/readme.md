# 机械觉之瞳代码

## 使用方法

1. 下载并安装arduino IDE
2. 把文件夹`src`更名为`main`，打开`main.ino`
3. 安装esp32固件库
4. 安装`ESP32Servo(3.0.6)`和`LovyanGX(1.2.0)`程式库
5. 选择`ESP32S3 Dev Module (esp32)`,开发板设置如下（此为N16R8版本，其他版本的需要修改部分参数）：

    | 参数名           | 参数    |
    | --------------- | ------- |
    | PSRAM           | opi     |
    | FlashMode       | qio120  |
    | FlashSize       | 16M     |
    | LoopCore        | 1       |
    | EventsCore      | 0       |
    | USBMode         | hwcdc   |
    | CDCOnBoot       | default |
    | MSCOnBoot       | default |
    | DFUOnBoot       | default |
    | UploadMode      | default |
    | PartitionScheme | default |
    | CPUFreq         | 240     |
    | UploadSpeed     | 921600  |
    | DebugLevel      | none    |


5. 连接开发板，从`main.ino`上传代码

## 注意事项

-   我使用的是arduino 1.8.x的版本，2.x.x的版本目前没有做过测试，请谨慎选择2.x.x的版本