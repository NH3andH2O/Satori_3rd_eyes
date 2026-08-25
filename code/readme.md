# 机械觉之瞳代码说明

本目录包含该项目的代码与相关开发资源，主要分为以下几个部分：

- 3rd_eyes：ESP32-S3 固件代码，负责眼睛控制、传感器处理与通信
- 3rd_eyes_web：网页控制端代码，用于配置与交互

## 目录结构

- 3rd_eyes/
  - 固件源码、PlatformIO 配置、分区表与测试代码
- 3rd_eyes_web/
  - Vue/Vite 构建的网页前端代码

## 构建说明

### 3rd_eyes_web

#### 构建
1. 安装[Node.js](https://nodejs.org/zh-cn/download)
    -   安装完成后，打开终端，输入以下命令确认：
        ```
        node -v
        npm -v
        ```
        如果能看到版本号，说明安装成功。
2. 安装依赖
   -    使用终端进入到本文件夹，执行`npm install`安装依赖
3. 构建项目
   -    使用`npm run build`构建项目。构建完成后，将会在`code\3rd_eyes`文件夹下生成`data`文件夹，即可上传网页代码到开发板。

#### 开发
建议使用 [VS Code](https://code.visualstudio.com) + [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)进行开发，你可能需要安装和配置[Docker](https://www.docker.com)。

### 3rd_eyes

#### 构建
1. 安裝[VS Code](https://code.visualstudio.com)
   - 可选：安装[简体中文](https://marketplace.visualstudio.com/items?itemName=MS-CEINTL.vscode-language-pack-zh-hans)语言包
2. 安裝[PlatformIO IDE](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide)插件，并等待安装完毕
3. 将开发板连接电脑，点击窗口最下方的`Set upload/monitor/test port`（类似于插头的图标），选择刚刚连接的开发板串口
4. 点击最左侧`PlatformIO`图标，然后在`PROJECT TASKS`中点击`esp32_s3_n16r8/General/Upload`上传程序
5. 在`3rd_eyes_web`文件夹中构建好网页代码后（你应该可以在本文件夹下看到`data`文件夹），在`PROJECT TASKS`中点击`esp32_s3_n16r8/Platform/Upload Filesystem Image`上传网页程序

#### 串口调试命令

本项目在 UART0 上提供了串口调试命令，默认波特率为 `115200`。你可以通过 PlatformIO 的串口监视器或其他串口工具发送命令，命令末尾回车即可。

可用命令如下：

- `help`：显示当前支持的串口命令。
- `reset`：重启设备。
- `reservo`：清除已保存的舵机配置，并切换到舵机调试模式。
- `mode [模式编号]`：切换运行模式，模式编号如下：
   - `0`：舵机调试模式。
   - `1`：陀螺仪跟踪模式。
   - `2`：网络控制模式。
- `log [日志等级]`：设置并保存日志等级：
   - `1`：错误（Error）。
   - `2`：警告（Warning）。
   - `3`：信息（Info）。
   - `4`：调试（Debug）。
   - `5`：详细（Verbose）。

#### 注意事项

##### 关于esp32模组选择
本项目使用ESP32-S3-WROOM-1-n16r8(16MB QuadSPI Flash、 8MB OctalSPI PSRAM)的模组
-  如果你购买的模组flash不是16MB的版本，需要修改`partitions\n16r8.csv`分区表配置
-  如果你购买的模组没有PSRAM，你需要在`platformio.ini`配置文件中删除以下语句：
   ```
   -D BOARD_HAS_PSRAM
   ```