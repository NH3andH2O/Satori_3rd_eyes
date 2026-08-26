[English](README.md) | [简体中文](README_zh-cn.md) | 繁體中文 | [日本語](README_ja.md)

# 機械覺之瞳

東方 Project 古明地覺的機械覺之瞳，目前已支援陀螺儀追蹤及網頁控制。

## 成品展示

https://www.bilibili.com/video/BV137yHB4ERo

## 作者及授權

- 程式碼：一水合氨基酸（NH3andH2O），使用 `Apache-2.0` 授權條款
- 3D 模型
  - 覺之瞳本體 `3rd_eyes`：牧鈴華SUZUKA。已詢問原作者並取得上傳至本 GitHub 儲存庫的許可；如需用於其他用途，請務必聯絡原作者（[專案頁面](https://www.bilibili.com/video/BV1rN1gYJE3K)）
  - 心形配飾 `heart_shaped`：一水合氨基酸，使用 `CC BY 4.0` 授權條款
- PCD
  - `3rd_eyes`：修改自 R饼锐度不够，使用 `CC BY-NC-SA 3.0` 授權條款（[專案頁面](https://oshwhub.com/flanker-e/jue-zhi-tong-esp32)）
  - `GC9A01`：一水合氨基酸，使用 `CC BY-NC-SA 3.0` 授權條款

## 素材引用

- 覺之瞳網頁圖示：幽蝶_Zomgel

## 專案架構

- `3d_model`：存放 3D 模型
- `pcd`：存放 PCD 檔案
- `code`：存放程式碼
- `tools`：用於偵錯裝置感測器的工具

## 使用方法

下載本儲存庫，並閱讀各資料夾下的 README。  
建議閱讀順序：[pcd](pcd/README_zh-tw.md) → [3d_model](3d_model/README_zh-tw.md) → [code](code/README_zh-tw.md)

## 操控方法

- 上傳程式碼後，裝置預設進入陀螺儀追蹤模式。將心形配飾固定在頭上，並將覺之瞳固定在身體上，即可操控覺之瞳。
- 如需變更設定，請連線至覺之瞳的 Wi-Fi `3rd-Eyes`（無密碼），然後開啟 `3rdeyes.local` 變更操控模式及設定。

## 未來計畫

- 加入覺之瞳自主動作（眨眼、搖頭等）
- 加入控制器操作

## 聯絡方式

歡迎加入覺之瞳專案 QQ 交流群組：`727559409`
