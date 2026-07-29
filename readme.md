# 机械觉之瞳
东方project古明地觉的觉之瞳，目前已经可以实现陀螺仪跟踪，网页控制

## 成品展示
https://www.bilibili.com/video/BV1CK7mzvE3U

## 作者及协议

-   代码：一水合氨基酸(NH3andH2O)，使用`Apache-2.0`协议
-   3D模型
    -   觉之瞳本体`3rd_eyes`：牧鈴華SUZUKA，已询问原作者，并获得上传到本github仓库的许可，如需要用于其他用途，请务必联系原作者（[项目地址](https://www.bilibili.com/video/BV1rN1gYJE3K)）
    -   心形配饰`heart_shaped`：一水合氨基酸，使用`CC BY 4.0`协议
-   PCD
    -   `3rd_eyes`：修改自 R饼锐度不够 ，使用`CC BY-NC-SA 3.0`协议（[项目地址](https://oshwhub.com/flanker-e/jue-zhi-tong-esp32)）
    -   `GC9A01`：一水合氨基酸，使用`CC BY-NC-SA 3.0`协议

## 素材引用
-   觉之瞳网页icon图标：幽蝶_Zomgel

## 项目架构

-   `3d_model`：存放3d模型的地方
-   `pcd`：存放pcd的地方
-   `code`：存放代码的地方
-   `tools`：工具，用于调试设备中的传感器

## 使用方法

下载本仓库，阅读各个文件夹下的readme.md\
推荐阅读顺序：`pcd`->`3d_model`->`code`

## 操控方法

-   在上传代码后，默认进入陀螺仪跟踪模式。将心形配饰绑在头上，觉之瞳固定在身体好，即可实现操控觉之瞳
-   如果想更改设定，可连接觉之瞳的wifi `3rd-Eyes` (无密码)后，进入网页`3rdeyes.local`即可更改操控模式和设定

## 未来计划

-   加入觉之瞳自主动作（眨眼、摇头等）
-   加入手柄控制

## 联系

欢迎进入觉之瞳项目交流群 “qq：727559409”