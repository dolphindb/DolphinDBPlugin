# mqtt 插件使用说明

mqtt 插件提供了一系列接口，支持与 mqtt 服务器建立连接并进行交互。支持发布消息、订阅消息并以 CSV 或 JSON 使格式解析消息。

本文档仅介绍编译构建方法。通过[文档中心 - mqtt](https://docs.dolphindb.cn/zh/plugins/mqtt/mqtt.html)查看使用介绍；通过 CHANGELOG.md 查看版本发布记录。

## 编译安装

### 在 Linux 下编译安装

* 以下步骤在64位 Linux GCC version 5.4.0 下编译测试通过。
* 在编译前需要先安装 [git](https://git-scm.com/) 和 [CMake](https://cmake.org/)。

Ubuntu 用户只需要在命令行输入以下命令即可：

```bash
$ sudo apt-get install git cmake
```

* 在 mqtt 目录下创建 build 目录，进入后运行 `cmake ..` 和 `make`，即可编译生成 'libPluginMQTTClient.so'。

```
mkdir build
cd build
cmake ..
make
```

### 在 Windows 下编译安装

因为需要通过 CMake 和 MinGW 编译，所以需要先安装 [CMake](https://cmake.org/)和 [MinGW](http://www.mingw.org/) 环境，目前在64位win10上用 MinGW-W64-builds-4.3.3 版本编译通过。把 MinGW 和 CMake 的 bin 目录加入 Windows 系统 Path 路径。 

```
    git clone https://github.com/dolphindb/DolphinDBPlugin.git
    cd DolphinDBPlugin/mqtt
    mkdir build
    cd build
    cmake ..
    copy /YOURPATH/libDolphinDB.dll . 
    make
```

**注意：** 如果需要指定特定的 MinGW 路径，请在 CmakeList.txt 中修改以下语句。

```
    set(MINGW32_LOCATION C://MinGW/MinGW/)  
```
编译之后目录下会产生 libPluginMQTTClient.dll 文件，然后按预编译安装方法导入并加载。
