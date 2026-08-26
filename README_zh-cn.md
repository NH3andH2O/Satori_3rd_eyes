[English](README.md) | 简体中文 | [繁體中文](README_zh-tw.md) | [日本語](README_ja.md)

# 机械觉之瞳

东方 Project 古明地觉的机械觉之瞳，目前已支持陀螺仪跟踪和网页控制。

## 成品展示

https://www.bilibili.com/video/BV137yHB4ERo

## 作者及协议

- 代码：一水合氨基酸（NH3andH2O），使用 `Apache-2.0` 协议
- 3D 模型
  - 觉之瞳本体 `3rd_eyes`：牧鈴華SUZUKA。已询问原作者并获得上传到本 GitHub 仓库的许可；如需用于其他用途，请务必联系原作者（[项目地址](https://www.bilibili.com/video/BV1rN1gYJE3K)）
  - 心形配饰 `heart_shaped`：一水合氨基酸，使用 `CC BY 4.0` 协议
- PCD
  - `3rd_eyes`：修改自 R饼锐度不够，使用 `CC BY-NC-SA 3.0` 协议（[项目地址](https://oshwhub.com/flanker-e/jue-zhi-tong-esp32)）
  - `GC9A01`：一水合氨基酸，使用 `CC BY-NC-SA 3.0` 协议

## 素材引用

- 觉之瞳网页图标：幽蝶_Zomgel

## 项目架构

- `3d_model`：存放 3D 模型
- `pcd`：存放 PCD 文件
- `code`：存放代码
- `tools`：用于调试设备传感器的工具

## 使用方法

下载本仓库，并阅读各文件夹下的 README。  
推荐阅读顺序：[pcd](pcd/README_zh-cn.md) → [3d_model](3d_model/README_zh-cn.md) → [code](code/README_zh-cn.md)

## 操控方法

- 上传代码后，设备默认进入陀螺仪跟踪模式。将心形配饰固定在头上，并将觉之瞳固定在身体上，即可操控觉之瞳。
- 如需更改设置，请连接觉之瞳的 Wi-Fi `3rd-Eyes`（无密码），然后打开 `3rdeyes.local` 更改操控模式和设置。

## 未来计划

- 加入觉之瞳自主动作（眨眼、摇头等）
- 加入手柄控制

## 联系

欢迎加入觉之瞳项目 QQ 交流群：`727559409`
