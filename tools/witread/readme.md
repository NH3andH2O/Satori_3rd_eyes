# 陀螺仪原始信息读取

本代码用于获取陀螺仪的原始信息

上传代码后，即可使用上位机查看陀螺仪的数据

## #define serial2 参数信息

如果需要获得眼睛陀螺仪（pcd上面的陀螺仪）的原始信息，请删除`#define serial2`

如果需要获得头部陀螺仪（通过杜邦线连接的陀螺仪）的原始信息，请保留`#define serial2`

## #define serialPort 参数信息

波特率相关

陀螺仪预设出产时为9600，需要在上位机修改波特率为115200后，把

```Arduino
#define serialPort 9600
```

更改为

```Arduino
#define serialPort 115200
```

在重新上传代码后，才可以继续使用上位机通信