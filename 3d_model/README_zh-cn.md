[English](README.md) | 简体中文 | [繁體中文](README_zh-tw.md) | [日本語](README_ja.md)

# 机械觉之瞳模型

## 其他作者

- **3rd_eyes：** 牧鈴華SUZUKA（[项目地址](https://www.bilibili.com/video/BV1rN1gYJE3K)）

## 如何使用

使用 3D 打印机打印所有 `.stl` 文件，然后参照 `STR-rc.prt` 进行安装。

## 注意事项

- **舵机：**使用 MG90S 180 度舵机。安装前需要先将舵机居中至 90 度。
  1. 使用程序让舵机处于居中位置。
     > [!TIP]
     > 可以先烧录主程序，再通过网页进入觉之瞳校正页面，将 3 个舵机的角度设置为 90 度，然后安装舵臂和齿轮。
  2. 按照下图安装舵臂和齿轮：  
     <img src="https://github.com/user-attachments/assets/966a090c-c51c-4c40-b57b-3a016ed3d949" width="300" />  \
     上眼皮舵臂安装位置

     <img src="https://github.com/user-attachments/assets/13863228-87b3-4ce6-bbb7-370c8e705013" width="300" />  \
     眼球舵臂安装位置。舵臂应与舵机保持垂直。

     <img src="https://github.com/user-attachments/assets/527a5b8f-8182-4cc5-b2bd-1d44201bb876" width="300" />  \
     下眼皮齿轮安装位置。请先调整齿轮轴（黄色区域）的方向，使杆件缺口朝向正外侧（如蓝色标记所示），再与舵机上的齿轮啮合。
- **关于 `pupil_GC9A01.prt`：**这是 `pupil.prt` 的中间开槽版本，可在中间放入厚度为 `0.5 mm`、直径为 `26.8 mm` 的圆形透明亚克力板（可在淘宝定制）。
- **关于 `heartShaped_dowm.prt`：**底面设有开孔，可以使用扎带固定在头饰上。
