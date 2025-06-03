# 机械觉之瞳pcd

## 其他作者

`3rd_eyes` 修改自 “R饼锐度不够”（[项目地址](https://oshwhub.com/flanker-e/jue-zhi-tong-esp32)）

&emsp;&emsp;主要修改内容：

&emsp;&emsp;&emsp;&emsp;- 去除摄像头模组

&emsp;&emsp;&emsp;&emsp;- 添加陀螺仪模组

&emsp;&emsp;&emsp;&emsp;- 舵机IO修改

&emsp;&emsp;&emsp;&emsp;- GC9A01排线顺序修改

&emsp;&emsp;&emsp;&emsp;- 5v 转 3.3v 电感修改（_4.7uH_ -> _3.9uH_）

## 在线查看
https://oshwhub.com/nh3_h2o/satori_3rd_eyes

> [!WARNING]
> 在线查看为Altium Designer转成嘉立创EDA而来，供方便查看使用，并没有经过检查，如要打板，请使用gerber文件打板！！

## 使用方法

把`3rd_eyes_GERBER.zip`和`GC9A01_GERBER.zip`直接发送给板厂

板厂制作完成后，根据`3rd_eyes_BOM.csv`和`GC9A01_BOM.csv`，将零件焊接到板子上

## 注意事项

### ESP32-S3选择

除了`ESP32-S3-WROOM-1U-NxRx`需要额外买天线系列的版本，也可以买`ESP32-S3-WROOM-1-NxRx`自带天线系列的版本，自带的天线可以塞得下觉之瞳里面

### 陀螺仪选择和使用

本项目使用“维特智能”系列的陀螺仪，以下是选择建议：

-  **眼睛（pcd上面的陀螺仪）：** 建议使用6轴陀螺仪，如[JY61P](https://www.wit-motion.cn/proztmz/37.html)
-  **头部（通过杜邦线连接的陀螺仪）：** 建议使用9轴陀螺仪，如[JY901S](https://www.wit-motion.cn/proztmz/22.html)

陀螺仪使用前，需要使用上位机做出如下设定：

-  校正磁场(6轴不需要校正，上位机应该会有校正教程)
-  将波特率从`9600`更改为`115200`
-  回传率更改为`200`
-  眼睛陀螺仪更改为垂直安装
-  只开启角度、角速度、加速度、四元数回传，其他回传关闭

（可以先更改陀螺仪设定，在焊接陀螺仪；也可以先焊接陀螺仪，在上传`witread.ino`后更改陀螺仪设定）

### 头部陀螺仪连接

头部陀螺仪使用厂商附带的弯排针焊接到UART那一侧上面，i2c那一侧不要焊接

杜邦线使用母母接头，连接到`H4`，长度建议选择60cm

### 电源开关选择建议

建议使用7mm键帽的开关

### 舵机连接

舵机选择 MG90S 180度的舵机，分别连接到`H1`~`H3`

### 屏幕连接

屏幕选择GC9A01裸屏，然后焊接到`GC9A01`pcd上的`U1`，**注意正反！**
