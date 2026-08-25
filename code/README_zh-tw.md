[English](README.md) | [简体中文](README_zh-cn.md) | 繁體中文 | [日本語](README_ja.md)

# 機械覺之瞳程式碼說明

本目錄包含此專案的程式碼及相關開發資源，主要分為以下部分：

- `3rd_eyes`：ESP32-S3 韌體程式碼，負責眼睛控制、感測器處理及通訊
- `3rd_eyes_web`：網頁控制端程式碼，用於設定及互動

## 目錄結構

- `3rd_eyes/`
  - 韌體原始碼、PlatformIO 設定、分割區表及測試程式碼
- `3rd_eyes_web/`
  - 使用 Vue/Vite 建置的網頁前端程式碼

## 建置說明

### 3rd_eyes_web

#### 建置

1. 安裝 [Node.js](https://nodejs.org/zh-tw/download)。
   - 安裝完成後，開啟終端機並輸入以下命令進行確認：
     ```
     node -v
     npm -v
     ```
     如果能看到版本號，表示安裝成功。
2. 安裝相依套件。
   - 使用終端機進入本目錄，執行 `npm install` 安裝相依套件。
3. 建置專案。
   - 執行 `npm run build` 建置專案。建置完成後，將在 `code\3rd_eyes` 資料夾下產生 `data` 資料夾，之後即可將網頁程式碼上傳至開發板。

#### 開發

建議使用 [VS Code](https://code.visualstudio.com) 及 [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) 進行開發。可能還需要安裝並設定 [Docker](https://www.docker.com)。

### 3rd_eyes

#### 建置

1. 安裝 [VS Code](https://code.visualstudio.com)。
2. 安裝 [PlatformIO IDE](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide) 擴充套件，並等待安裝完成。
3. 將開發板連接至電腦，點選視窗底部的 `Set upload/monitor/test port`（類似插頭的圖示），然後選擇剛連接的開發板序列埠。
4. 點選左側的 `PlatformIO` 圖示，然後在 `PROJECT TASKS` 中選擇 `esp32_s3_n16r8/General/Upload` 上傳程式。
5. 在 `3rd_eyes_web` 資料夾中建置網頁程式碼後（本目錄下應出現 `data` 資料夾），在 `PROJECT TASKS` 中選擇 `esp32_s3_n16r8/Platform/Upload Filesystem Image` 上傳網頁程式。

#### 序列埠除錯命令

本專案在 UART0 上提供序列埠除錯命令，預設鮑率為 `115200`。可以透過 PlatformIO 序列埠監視器或其他序列埠工具傳送命令，並在命令末尾按下 Enter。

可用命令如下：

- `help`：顯示目前支援的序列埠命令。
- `reset`：重新啟動裝置。
- `reservo`：清除已儲存的伺服馬達設定，並切換至伺服馬達除錯模式。
- `mode [模式編號]`：切換執行模式：
  - `0`：伺服馬達除錯模式。
  - `1`：陀螺儀追蹤模式。
  - `2`：網路控制模式。
- `log [記錄層級]`：設定並儲存記錄層級：
  - `1`：錯誤（Error）。
  - `2`：警告（Warning）。
  - `3`：資訊（Info）。
  - `4`：除錯（Debug）。
  - `5`：詳細（Verbose）。

#### 注意事項

##### ESP32 模組選擇

本專案使用 ESP32-S3-WROOM-1-N16R8 模組，配備 16 MB QuadSPI Flash 及 8 MB OctalSPI PSRAM。

- 如果購買的模組 Flash 不是 16 MB 版本，需要修改 `partitions\n16r8.csv` 分割區表設定。
- 如果購買的模組沒有 PSRAM，需要在 `platformio.ini` 設定檔中刪除以下敘述：
  ```
  -D BOARD_HAS_PSRAM
  ```
