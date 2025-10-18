# 机械觉之瞳网页代码

## 构建
1. 安装[Node.js](https://nodejs.org/zh-cn/download)
    -   安装完成后，打开终端，输入以下命令确认：
        ```
        node -v
        npm -v
        ```
        如果能看到版本号，说明安装成功。
2. 安装依赖
   -    使用终端进入到本文件夹，执行`npm install`安装依赖
3. 构建项目
   -    使用`npm run build`构建项目。构建完成后，将会在`arduino`文件夹下生成`data`文件夹，即可上传网页代码到开发板。

## 开发
建议使用 [VS Code](https://code.visualstudio.com) + [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)进行开发，你可能需要安装和配置[Docker](https://www.docker.com)。