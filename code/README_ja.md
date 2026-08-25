[English](README.md) | [简体中文](README_zh-cn.md) | [繁體中文](README_zh-tw.md) | 日本語

# 機械式サードアイのコード

このディレクトリには、プロジェクトのソースコードと開発用リソースが含まれています。主な構成は次のとおりです。

- `3rd_eyes`：目の制御、センサー処理、通信を担当する ESP32-S3 ファームウェア
- `3rd_eyes_web`：設定と操作に使用する Web コントロール画面

## ディレクトリ構成

- `3rd_eyes/`
  - ファームウェアのソースコード、PlatformIO 設定、パーティションテーブル、テストコード
- `3rd_eyes_web/`
  - Vue/Vite で構築された Web フロントエンド

## ビルド手順

### 3rd_eyes_web

#### ビルド

1. [Node.js](https://nodejs.org/ja/download) をインストールします。
   - インストール後、ターミナルを開いて次のコマンドで確認します。
     ```
     node -v
     npm -v
     ```
     バージョン番号が表示されれば、インストールは完了です。
2. 依存関係をインストールします。
   - ターミナルでこのディレクトリを開き、`npm install` を実行します。
3. プロジェクトをビルドします。
   - `npm run build` を実行します。ビルドが完了すると `code\3rd_eyes` フォルダー内に `data` フォルダーが生成され、Web インターフェースを開発ボードへアップロードできるようになります。

#### 開発

開発には [VS Code](https://code.visualstudio.com) と [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) の使用を推奨します。[Docker](https://www.docker.com) のインストールと設定も必要になる場合があります。

### 3rd_eyes

#### ビルド

1. [VS Code](https://code.visualstudio.com) をインストールします。
2. [PlatformIO IDE](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide) 拡張機能をインストールし、完了するまで待ちます。
3. 開発ボードをコンピューターに接続します。ウィンドウ下部の `Set upload/monitor/test port`（プラグのようなアイコン）をクリックし、接続したボードのシリアルポートを選択します。
4. 左側の `PlatformIO` アイコンをクリックし、`PROJECT TASKS` から `esp32_s3_n16r8/General/Upload` を選択してファームウェアをアップロードします。
5. `3rd_eyes_web` フォルダーで Web インターフェースをビルドした後（このディレクトリに `data` フォルダーが表示されます）、`PROJECT TASKS` から `esp32_s3_n16r8/Platform/Upload Filesystem Image` を選択してアップロードします。

#### シリアルデバッグコマンド

このプロジェクトは UART0 でシリアルデバッグコマンドを提供し、デフォルトのボーレートは `115200` です。PlatformIO のシリアルモニターまたはその他のシリアルターミナルからコマンドを送信し、最後に Enter キーを押してください。

使用可能なコマンド：

- `help`：対応しているシリアルコマンドを表示します。
- `reset`：デバイスを再起動します。
- `reservo`：保存済みのサーボ設定を消去し、サーボデバッグモードに切り替えます。
- `mode [モード番号]`：動作モードを切り替えます。
  - `0`：サーボデバッグモード。
  - `1`：ジャイロ追従モード。
  - `2`：ネットワーク制御モード。
- `log [ログレベル]`：ログレベルを設定して保存します。
  - `1`：エラー（Error）。
  - `2`：警告（Warning）。
  - `3`：情報（Info）。
  - `4`：デバッグ（Debug）。
  - `5`：詳細（Verbose）。

#### 注意事項

##### ESP32 モジュールの選択

このプロジェクトでは、16 MB QuadSPI Flash と 8 MB OctalSPI PSRAM を搭載した ESP32-S3-WROOM-1-N16R8 モジュールを使用します。

- 購入したモジュールの Flash が 16 MB 版ではない場合、`partitions\n16r8.csv` のパーティションテーブル設定を変更してください。
- 購入したモジュールに PSRAM がない場合、`platformio.ini` から次の行を削除してください。
  ```
  -D BOARD_HAS_PSRAM
  ```
