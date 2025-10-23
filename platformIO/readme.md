# 机械觉之瞳代码

## 使用方法
1. 安裝[VS Code](https://code.visualstudio.com)
   - 可选：安装[简体中文](https://marketplace.visualstudio.com/items?itemName=MS-CEINTL.vscode-language-pack-zh-hans)语言包
2. 安裝[PlatformIO IDE](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide)插件，并等待安装完毕
3. 将开发板连接电脑，点击窗口最下方的`Set upload/monitor/test port`（类似于插头的图标），选择刚刚连接的开发板串口
4. 点击最左侧`PlatformIO`图标，然后在`PROJECT TASKS`中点击`esp32_s3_n16r8/General/Upload`上传程序
5. 在`vue`文件夹中构建好网页代码后（你应该可以在本文件夹下看到`data`文件夹），在`PROJECT TASKS`中点击`esp32_s3_n16r8/Platform/Upload Filesystem Image`上传网页程序