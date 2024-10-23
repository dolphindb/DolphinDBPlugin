# DolphinDB mseed Plugin

miniSEED 是SEED 格式的子集，一般用于地震学时间序列数据的归档和交换。DolphinDB 的 mseed 插件可以读取 miniSEED 文件的数据到 DolphinDB 的内存表中，且可以将 DolphinDB 的一段连续时间的采样值写入到 miniSEED 格式的文件中。本插件使用了 IRIS 的 [libmseed 开源库](https://github.com/iris-edu/libmseed) 的读写接口。

本文档仅介绍编译构建方法。通过 [文档中心-mseed](https://docs.dolphindb.cn/zh/plugins/mseed/mseed.html) 查看接口介绍；通过 [CHANGELOG.md](./CHANGELOG.md) 查看版本发布记录。

## 编译构建

### Linux 编译构建

**使用 cmake 构建：**

安装 cmake：

```
sudo apt-get install cmake
```

构建插件内容：

```
cd <PluginDir>/mseed
mkdir build
cd build
cmake  ../
make
```

**注意**：编译之前请确保 libDolphinDB.so 在 gcc 可搜索的路径中。可使用 `LD_LIBRARY_PATH` 指定其路径，或者直接将其拷贝到 build 目录下。

编译后目录下会产生文件 libPluginMseed.so 和 PluginMseed.txt。

### Windows 编译构建

**在 Windows 环境中需要使用 CMake 和 MinGW 编译**

- 下载安装 [MinGW](http://www.mingw-w64.org/)。确保将 bin 目录添加到系统环境变量 Path 中。
- 下载安装 [cmake](https://cmake.org/)。

**使用 cmake 构建**

在编译开始之前，要将 libDolphinDB.dll 拷贝到 build 文件夹下。

构建插件内容：

```
cd <PluginDir>\mseed
mkdir build                                             # 新建 build 目录
COPY <ServerDir>/libDolphinDB.dll build                 # 拷贝 libDolphinDB.dll 到 build 目录下
cd build
cmake  ../ -G "MinGW Makefiles"
mingw32-make -j4
```

编译后目录下会产生文件 libPluginMseed.dll 和 PluginMseed.txt。
