[English](README.md) | 简体中文 | [繁體中文](README_zh-tw.md) | [日本語](README_ja.md)

# 机械觉之瞳代码说明

本目录包含该项目的代码及相关开发资源，主要分为以下部分：

- `3rd_eyes`：ESP32-S3 固件代码，负责眼睛控制、传感器处理及通信
- `3rd_eyes_web`：网页控制端代码，用于配置和交互

## 目录结构

- `3rd_eyes/`
  - 固件源代码、PlatformIO 配置、分区表及测试代码
- `3rd_eyes_web/`
  - 使用 Vue/Vite 构建的网页前端代码

## 构建说明

### 3rd_eyes_web

#### 构建

1. 安装 [Node.js](https://nodejs.org/zh-cn/download)。
   - 安装完成后，打开终端并输入以下命令进行确认：
     ```
     node -v
     npm -v
     ```
     如果能看到版本号，说明安装成功。
2. 安装依赖。
   - 使用终端进入本目录，执行 `npm install` 安装依赖。
3. 构建项目。
   - 执行 `npm run build` 构建项目。构建完成后，将在 `code\3rd_eyes` 文件夹下生成 `data` 文件夹，之后即可将网页代码上传到开发板。

#### 开发

建议使用 [VS Code](https://code.visualstudio.com) 和 [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) 进行开发。你可能还需要安装并配置 [Docker](https://www.docker.com)。

### 3rd_eyes

#### 构建

1. 安装 [VS Code](https://code.visualstudio.com)。
   - 可选：安装[简体中文](https://marketplace.visualstudio.com/items?itemName=MS-CEINTL.vscode-language-pack-zh-hans)语言包。
2. 安装 [PlatformIO IDE](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide) 扩展，并等待安装完成。
3. 将开发板连接至电脑，点击窗口底部的 `Set upload/monitor/test port`（类似插头的图标），然后选择刚连接的开发板串口。
4. 点击左侧的 `PlatformIO` 图标，然后在 `PROJECT TASKS` 中选择 `esp32_s3_n16r8/General/Upload` 上传程序。
5. 在 `3rd_eyes_web` 文件夹中构建网页代码后（此目录下应出现 `data` 文件夹），在 `PROJECT TASKS` 中选择 `esp32_s3_n16r8/Platform/Upload Filesystem Image` 上传网页程序。

#### 串口调试命令

本项目在 UART0 上提供串口调试命令，默认波特率为 `115200`。可以通过 PlatformIO 串口监视器或其他串口工具发送命令，并在命令末尾按 Enter。

可用命令如下：

- `help`：显示当前支持的串口命令。
- `reset`：重启设备。
- `reservo`：清除已保存的舵机配置，并切换至舵机调试模式。
- `mode [模式编号]`：切换运行模式：
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

##### ESP32 模组选择

本项目使用 ESP32-S3-WROOM-1-N16R8 模组，配备 16 MB QuadSPI Flash 和 8 MB OctalSPI PSRAM。

- 如果购买的模组 Flash 不是 16 MB 版本，需要修改 `partitions\n16r8.csv` 分区表配置。
- 如果购买的模组没有 PSRAM，需要在 `platformio.ini` 配置文件中删除以下语句：
  ```
  -D BOARD_HAS_PSRAM
  ```
