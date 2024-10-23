# DolphinDB zip Plugin

zip 是一种标准的压缩文件的格式。DolphinDB 的 zip 插件可以对 zip 文件进行解压、压缩文件到 zip 文件中。zip 插件基于 minizip 和 zipper 开发。本文档仅介绍编译构建方法。通过 [文档中心 - Zip](https://docs.dolphindb.cn/zh/plugins/zip/zip.html) 查看接口介绍；通过 [CHANGELOG.md](./CHANGELOG.md) 查看版本发布记录。

## 编译构建

### Linux
安装 CMake

```bash
sudo apt install cmake
```

编译整个项目
```bash
mkdir build
cd build
cp /path_to_dolphindb/libDolphinDB.so ../lib
cmake ..
make -j
```

编译后将生成 libPluginZip.so 文件。


### WINDOWS
```
mkdir build                                           # 新建 build 目录
cp <ServerDir>/libDolphinDB.dll build                 # 拷贝 libDolphinDB.dll 到 build 目录下
cd build
cmake  ../ -G "MinGW Makefiles"
mingw32-make -j4
```
编译后将生成 libPluginZip.dll 文件。

#### 编译依赖库 zlib-1.2.11
下载 zlib-1.2.11 [https://github.com/madler/zlib/archive/refs/tags/v1.2.11.zip]
解压 zlib-1.2.11.zip
然后进行编译
```
mkdir build
cd build
cmake  ../ -G "MinGW Makefiles"
mingw32-make -j4
```
复制 zlib 静态库
```
copy .\libzlibstatic.a PATH_TO_PLUGIN_ZIP\lib\Win64 #PATH_TO_PLUGIN_ZIP 为 zip 插件路径
```
复制 zlib 头文件
```
copy ..\zlib.h PATH_TO_PLUGIN_ZIP\lib_src #PATH_TO_PLUGIN_ZIP 为 zip 插件路径
```
