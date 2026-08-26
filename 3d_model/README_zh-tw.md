[English](README.md) | [简体中文](README_zh-cn.md) | 繁體中文 | [日本語](README_ja.md)

# 機械覺之瞳模型

## 其他作者

- **3rd_eyes：** 牧鈴華SUZUKA（[專案頁面](https://www.bilibili.com/video/BV1rN1gYJE3K)）

## 使用方法

使用 3D 印表機列印所有 `.stl` 檔案，然後參照 `STR-rc.prt` 進行組裝。

## 注意事項

- **伺服馬達：** 使用 MG90S 180 度伺服馬達。安裝前需先將伺服馬達置中至 90 度。
  1. 使用程式讓伺服馬達移動至置中位置。
     > [!TIP]
     > 可以先燒錄主程式，再透過網頁進入覺之瞳校正頁面，將 3 個伺服馬達的角度設為 90 度，然後安裝伺服擺臂及齒輪。
  2. 按照下圖安裝伺服擺臂及齒輪：  
     <img src="https://github.com/user-attachments/assets/966a090c-c51c-4c40-b57b-3a016ed3d949" width="300" />  \
     上眼皮伺服擺臂安裝位置

     <img src="https://github.com/user-attachments/assets/13863228-87b3-4ce6-bbb7-370c8e705013" width="300" />  \
     眼球伺服擺臂安裝位置。伺服擺臂應與伺服馬達保持垂直。

     <img src="https://github.com/user-attachments/assets/527a5b8f-8182-4cc5-b2bd-1d44201bb876" width="300" />  \
     下眼皮齒輪安裝位置。請先調整齒輪軸（黃色區域）的方向，使桿件缺口朝向正外側（如藍色標記所示），再與伺服馬達上的齒輪嚙合。
- **關於 `pupil_GC9A01.prt`：** 這是 `pupil.prt` 的中央開槽版本，可在中央放入厚度為 `0.5 mm`、直徑為 `26.8 mm` 的圓形透明壓克力板。
- **關於 `heartShaped_dowm.prt`：** 底面設有開孔，可以使用束帶固定在頭飾上。
