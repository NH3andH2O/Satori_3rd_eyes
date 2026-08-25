[English](README.md) | [简体中文](README_zh-cn.md) | 繁體中文 | [日本語](README_ja.md)

# 機械覺之瞳 PCD

## 其他作者

`3rd_eyes` 修改自 R饼锐度不够（[專案頁面](https://oshwhub.com/flanker-e/jue-zhi-tong-esp32)）。

主要修改內容：

- 移除相機模組
- 加入陀螺儀模組
- 修改伺服馬達 I/O
- 修改 GC9A01 排線順序
- 將 5 V 轉 3.3 V 電路的電感從 _4.7 uH_ 改為 _3.9 uH_

## 線上檢視

https://oshwhub.com/nh3_h2o/satori_3rd_eyes

> [!WARNING]
> 線上版本由 Altium Designer 轉換為嘉立創 EDA，僅供方便檢視，且未經檢查。如需製作電路板，請使用 Gerber 檔案。

## 使用方法

將 `3rd_eyes_GERBER.zip` 及 `GC9A01_GERBER.zip` 直接傳送給板廠。  
電路板製作完成後，依照 `3rd_eyes_BOM.csv` 及 `GC9A01_BOM.csv` 將零件焊接至板上。

## 注意事項

### ESP32-S3 選擇

建議選擇 `ESP32-S3-WROOM-1-N16R8`。其他版本的模組可能需要修改 `platformIO` 設定。

### 陀螺儀選擇及使用

本專案使用「維特智能」系列陀螺儀，建議選擇以下型號：

- **眼睛（PCD 上的陀螺儀）：**建議使用 6 軸陀螺儀，例如 [JY61P](https://www.wit-motion.cn/proztmz/37.html)
- **頭部（透過杜邦線連接的陀螺儀）：**建議使用 9 軸陀螺儀，例如 [JY901S](https://www.wit-motion.cn/proztmz/22.html)

主程式會自動設定部分陀螺儀參數，但以下項目無法自動設定，需要使用[上位機軟體](https://wit-motion.yuque.com/wumwnr/aqvq6y/qngktvx5grz81zkq)進行設定：

- 校正磁場（6 軸型號不需校正；上位機軟體中應有校正教學）

先上傳主程式以自動設定陀螺儀，再上傳 `tools\witread` 工具，以便使用上位機軟體進行設定。

> [!NOTE]
> 如果不需要使用陀螺儀功能，可以不焊接陀螺儀。

### 頭部陀螺儀連接

將廠商附帶的彎排針焊接在頭部陀螺儀的 UART 一側，請勿焊接 I2C 一側。  
使用母對母杜邦線連接至 `H4`，建議長度為 60–80 cm。

<img src="https://github.com/user-attachments/assets/d17616e4-5e87-48f3-a36b-859f3634cc94" width="300" />

### 電源開關選擇建議

建議使用適用於 7 mm 鍵帽的開關。

### 伺服馬達連接

使用 MG90S 180 度伺服馬達，分別連接至 `H1`–`H3`。

> [!WARNING]
> 品牌必須為 TIANKONGRC，並請確認所附伺服擺臂是不含加強肋的型號。

### 螢幕連接

選擇 GC9A01 裸螢幕，並將其焊接至 `GC9A01` PCD 上的 `U1`。

> [!WARNING]
> 焊接時請確認螢幕接腳的正反方向。焊接完成後，PCD 應能自然翻轉，以便使用雙面膠將 PCD 背面黏貼至顯示器背面。
