[English](README.md) | [简体中文](README_zh-cn.md) | [繁體中文](README_zh-tw.md) | 日本語

# 機械式サードアイ

東方 Project の古明地さとりをモチーフにした機械式サードアイです。現在、ジャイロセンサーによる追従と Web ページからの操作に対応しています。

## 完成品デモ

https://www.bilibili.com/video/BV137yHB4ERo

## 作者とライセンス

- コード：一水合氨基酸（NH3andH2O）、`Apache-2.0` ライセンス
- 3D モデル
  - サードアイ本体 `3rd_eyes`：牧鈴華SUZUKA。原作者から本 GitHub リポジトリへのアップロード許可を得ています。その他の用途で使用する場合は、必ず原作者にお問い合わせください（[プロジェクトページ](https://www.bilibili.com/video/BV1rN1gYJE3K)）。
  - ハート型アクセサリー `heart_shaped`：一水合氨基酸、`CC BY 4.0` ライセンス
- PCD
  - `3rd_eyes`：R饼锐度不够による設計を改変、`CC BY-NC-SA 3.0` ライセンス（[プロジェクトページ](https://oshwhub.com/flanker-e/jue-zhi-tong-esp32)）
  - `GC9A01`：一水合氨基酸、`CC BY-NC-SA 3.0` ライセンス

## 素材クレジット

- サードアイ Web インターフェースのアイコン：幽蝶_Zomgel

## プロジェクト構成

- `3d_model`：3D モデル
- `pcd`：PCD 設計ファイル
- `code`：ソースコード
- `tools`：デバイスのセンサーをデバッグするためのツール

## 使用方法

このリポジトリをダウンロードし、各フォルダーの README をお読みください。  
推奨順序：[pcd](pcd/README_ja.md) → [3d_model](3d_model/README_ja.md) → [code](code/README_ja.md)

## 操作方法

- コードをアップロードすると、デフォルトでジャイロ追従モードになります。ハート型アクセサリーを頭に固定し、サードアイを身体に固定すると操作できます。
- 設定を変更するには、サードアイの Wi-Fi `3rd-Eyes`（パスワードなし）に接続し、`3rdeyes.local` を開いて操作モードや設定を変更してください。

## 今後の予定

- まばたきや首振りなど、サードアイの自律動作を追加
- コントローラー操作を追加

## 連絡先

プロジェクトの QQ グループ：`727559409`
