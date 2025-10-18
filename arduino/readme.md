# 机械觉之瞳代码

## 使用方法

### 第一部分：安装IDE
1. 下载并安装[arduino IDE](https://www.arduino.cc/en/software/)(建议下载Legacy IDE (1.8.19)版本)
2. 打开`arduino.ino`
3. 安装`esp32(3.2.1)`固件库
    > [!WARNING]
    > 由于3.3版本以上的固件库不兼容LovyanGX，故暂时不要升级到3.3以上的版本
4. 安装`ESP32Servo(3.0.9)`、`LovyanGX(1.2.7)`、`ESP Async WebServer(3.8.1)`、`AsyncTCP(3.4.9)`、`ArduinoJson(7.4.2)`程式库

### 第二部分：上传硬件代码到开发板
5. 选择`ESP32S3 Dev Module (esp32)`,开发板设置如下（此为N16R8版本，其他版本的需要修改部分参数）：

    | 参数名                               | 参数                            |
    | ------------------------------------ | ------------------------------- |
    | PSRAM                                | OPI PSRAM                       |
    | Flash Mode                           | QIO 120MHz                      |
    | Flash Size                           | 16MB (128Mb)                    |
    | Arduino Runs Core                    | Core 1                          |
    | Events Run On                        | Core 0                          |
    | USB Mode                             | Hardware CDC and JTAG           |
    | USB CDC On Boot                      | Disabled                        |
    | USB Firmware MSC On Boot             | Disabled                        |
    | USB DFU On Boot                      | Disabled                        |
    | Upload Mode                          | UART0 / Hardware CDC            |
    | Partition Scheme                     | 16M Flash (3MB APP/9.9MB FATFS) |
    | CPU Frequency                        | 240 MHz (WiFi)                  |
    | Upload Speed                         | 921600                          |
    | Erase All Flash Before Sketch Upload | Disabled                        |
    | JATG Adapter                         | Disabled                        |
    | Zigbee Mode                          | Disabled                        |

6. 连接开发板，从`arduino.ino`上传代码

### 第三部分：上传网页代码到开发板
7. 下载[ESP32fs Plugin](https://github.com/lorol/arduino-esp32fs-plugin/releases/tag/2.0.7)插件，解压缩后将`esp32fs.jar`放置到`（arduino安装位置）\tools\ESP32FS\tool`文件夹下（如果没有文件夹，新创建一个）
8. 下载[mkfatfs](https://github.com/labplus-cn/mkfatfs/releases/tag/v2.0.1)，解压缩后将里面4个文件放置到`%LOCALAPPDATA%\Arduino15\packages\esp32\hardware\esp32\3.2.1\tools`文件夹下
9. 在`vue`中构建好网页代码后（你应该可以看到本文件夹下多了`data`文件夹），在`工具`->`ESP32 Sketch Data Upload`中，选择FatFS上传网页代码

## 开发
虽然arduino IDE也可以进行开发代码，但由于此IDE过于老旧，更推荐使用[VS Code](https://code.visualstudio.com) + [Arduino Maker Workshop](https://marketplace.visualstudio.com/items?itemName=TheLastOutpostWorkshop.arduino-maker-workshop)插件进行开发
