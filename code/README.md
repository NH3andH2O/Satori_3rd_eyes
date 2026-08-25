English | [简体中文](README_zh-cn.md) | [繁體中文](README_zh-tw.md) | [日本語](README_ja.md)

# Mechanical Satori's Third Eye Code

This directory contains the project's source code and development resources. It is divided into the following components:

- `3rd_eyes`: ESP32-S3 firmware for eye control, sensor processing, and communication
- `3rd_eyes_web`: Web control interface for configuration and interaction

## Directory Structure

- `3rd_eyes/`
  - Firmware source code, PlatformIO configuration, partition table, and tests
- `3rd_eyes_web/`
  - Vue/Vite web frontend

## Build Instructions

### 3rd_eyes_web

#### Build

1. Install [Node.js](https://nodejs.org/en/download).
   - After installation, open a terminal and verify it with:
     ```
     node -v
     npm -v
     ```
     If version numbers are displayed, the installation was successful.
2. Install dependencies.
   - Open a terminal in this directory and run `npm install`.
3. Build the project.
   - Run `npm run build`. After the build completes, a `data` directory will be generated under `code\3rd_eyes`, allowing the web interface to be uploaded to the development board.

#### Development

[VS Code](https://code.visualstudio.com) with [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) is recommended for development. You may also need to install and configure [Docker](https://www.docker.com).

### 3rd_eyes

#### Build

1. Install [VS Code](https://code.visualstudio.com).
2. Install the [PlatformIO IDE](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide) extension and wait for the installation to finish.
3. Connect the development board to the computer. Click `Set upload/monitor/test port` at the bottom of the window (the plug-shaped icon), then select the serial port of the connected board.
4. Click the `PlatformIO` icon in the left sidebar, then select `esp32_s3_n16r8/General/Upload` under `PROJECT TASKS` to upload the firmware.
5. After building the web interface in `3rd_eyes_web` (a `data` directory should appear in this directory), select `esp32_s3_n16r8/Platform/Upload Filesystem Image` under `PROJECT TASKS` to upload it.

#### Serial Debugging Commands

The project provides serial debugging commands on UART0 at a default baud rate of `115200`. Send commands through the PlatformIO serial monitor or another serial terminal, followed by Enter.

Available commands:

- `help`: Display the supported serial commands.
- `reset`: Restart the device.
- `reservo`: Clear the saved servo configuration and switch to servo debugging mode.
- `mode [mode number]`: Change the operating mode:
  - `0`: Servo debugging mode.
  - `1`: Gyroscope-tracking mode.
  - `2`: Network control mode.
- `log [log level]`: Set and save the log level:
  - `1`: Error.
  - `2`: Warning.
  - `3`: Info.
  - `4`: Debug.
  - `5`: Verbose.

#### Notes

##### ESP32 Module Selection

This project uses the ESP32-S3-WROOM-1-N16R8 module with 16 MB QuadSPI Flash and 8 MB OctalSPI PSRAM.

- If your module does not have 16 MB of flash, update the partition table configuration in `partitions\n16r8.csv`.
- If your module does not have PSRAM, remove the following line from `platformio.ini`:
  ```
  -D BOARD_HAS_PSRAM
  ```