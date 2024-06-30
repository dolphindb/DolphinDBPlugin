# DolphinDB opc Plugin

OPC 是一项应用于自动化行业及其他行业的数据安全交换可互操作性标准。DolphinDB 的 OPC 插件可用于访问并采集自动化行业 OPC 服务器的数据。OPC DA 插件实现了 OPC DA 2.05A 版本规范。本文档仅介绍编译构建方法。通过 [文档中心 - OPC](https://docs.dolphindb.cn/zh/plugins/opc/opc.html) 查看接口介绍。

## 编译构建

OPC 插件依赖 [OPC 核心组件](https://opcfoundation.org/developer-tools/samples-and-tools-classic/core-components/#)。

### OPC 核心组件安装

插件依赖 OPC Core Components Redistributable 3.0.106 及以上版本。可从 [bin](./bin/win64) 目录下载 OPC Core Components Redistributable 压缩包。下载后解压，双击 msi 文件即可安装。


### 编译安装

通过 cmake 和 MinGW 编译

安装 [cmake](https://cmake.org/)。cmake 一个流行的项目构建工具，可以帮你轻松的解决第三方依赖的问题。  

安装 [MinGW](http://www.mingw.org/) 环境，带有 com 库（应该尽量选择新的版本），目前在 64 位 win10 上用 MinGW-W64-builds-4.3.3 版本编译通过。

把 MingGW 和 cmake 的 bin 目录加入 Windows 系统 Path 路径。 

```
    git clone https://github.com/dolphindb/DolphinDBPlugin.git
    cd DolphinDBPlugin
    mkdir build
    cd build
    cmake ../opc -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
    copy /YOURPATH/libDolphinDB.dll . 
    mingw32-make clean
    mingw32-make
```

**注意：** 如果需要指定特定的 MingW 路径，请在 CmakeList.txt 中修改以下语句。

```
    set(MINGW32_LOCATION C://MinGW/MinGW/)  
```
编译之后目录下会产生 libPluginOPC.dll 文件，然后按预编译安装方法导入并加载。