[English](README.md) | 简体中文 | [繁體中文](README_zh-tw.md) | [日本語](README_ja.md)

# 机械觉之瞳 PCD

## 其他作者

`3rd_eyes` 修改自 R饼锐度不够（[项目地址](https://oshwhub.com/flanker-e/jue-zhi-tong-esp32)）。

主要修改内容：

- 去除摄像头模组
- 添加陀螺仪模组
- 修改舵机 I/O
- 修改 GC9A01 排线顺序
- 将 5 V 转 3.3 V 电路的电感从 _4.7 uH_ 改为 _3.9 uH_

## 在线查看

https://oshwhub.com/nh3_h2o/satori_3rd_eyes

> [!WARNING]
> 在线版本由 Altium Designer 转换为嘉立创 EDA，仅供方便查看，且未经检查。如需制作电路板，请使用 Gerber 文件。

## 使用方法

将 `3rd_eyes_GERBER.zip` 和 `GC9A01_GERBER.zip` 直接发送给板厂。  
电路板制作完成后，根据 `3rd_eyes_BOM.csv` 和 `GC9A01_BOM.csv` 将零件焊接到板上。

## 注意事项

### ESP32-S3 选择

建议选择 `ESP32-S3-WROOM-1-N16R8`。其他版本的模组可能需要修改 `platformIO` 配置。

### 陀螺仪选择和使用

本项目使用“维特智能”系列陀螺仪，建议选择以下型号：

- **眼睛（PCD 上的陀螺仪）：**建议使用 6 轴陀螺仪，如 [JY61P](https://www.wit-motion.cn/proztmz/37.html)
- **头部（通过杜邦线连接的陀螺仪）：**建议使用 9 轴陀螺仪，如 [JY901S](https://www.wit-motion.cn/proztmz/22.html)

主程序会自动设置部分陀螺仪配置，但以下项目无法自动设置，需要使用[上位机软件](https://wit-motion.yuque.com/wumwnr/aqvq6y/qngktvx5grz81zkq)进行配置：

- 校正磁场（6 轴型号无需校正；上位机软件中应有校正教程）

先上传主程序以自动设置陀螺仪，再上传 `tools\witread` 工具，以便使用上位机软件进行配置。

> [!NOTE]
> 如果不需要使用陀螺仪功能，可以不焊接陀螺仪。

### 头部陀螺仪连接

将厂商附带的弯排针焊接在头部陀螺仪的 UART 一侧，请勿焊接 I2C 一侧。  
使用母对母杜邦线连接至 `H4`，建议长度为 60–80 cm。

<img src="https://github.com/user-attachments/assets/d17616e4-5e87-48f3-a36b-859f3634cc94" width="300" />

### 电源开关选择建议

建议使用适配 7 mm 键帽的开关。

### 舵机连接

使用 MG90S 180 度舵机，分别连接至 `H1`–`H3`。

> [!WARNING]
> 品牌必须为 TIANKONGRC，并请确认所配舵臂是不带加强筋的型号。

### 屏幕连接

选择 GC9A01 裸屏，并将其焊接至 `GC9A01` PCD 上的 `U1`。

> [!WARNING]
> 焊接时请确认屏幕引脚的正反方向。焊接完成后，PCD 应能自然翻转，以便使用双面胶将 PCD 背面粘贴至显示屏背面。
