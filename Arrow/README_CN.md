# Arrow 插件使用说明

[Apache Arrow](https://arrow.apache.org/) 是一种跨平台的内存数据格式，被设计为一种内存中的列式数据格式，可以高效地存储和传输大规模数据集，同时提供了高性能的数据操作功能。在引入 Arrow 插件之前，DolphinDB 与 API 进行数据交互时可以通过 PICKLE 或者 DDB 协议进行序列化。引入 Arrow 插件后，DolphinDB 数据，可以通过 ARROW 协议转换为 Arrow 格式进行传输，从而使得 DolphinDB 与 API 之间的数据传输更高效。

本文档仅介绍编译构建方法。通过 [文档中心 - Arrow](https://docs.dolphindb.cn/zh/plugins/Arrow/arrow.html) 查看使用介绍；通过 CHANGELOG.md 查看版本发布记录。

## 编译安装

如果不通过插件市场安装插件，也可以使用如下方式从源码编译 Arrow 插件。

### 在 Linux 下编译安装

#### 环境准备

- cmake
- g++ 4.8.5
- arrow 9.0.0

其中 arrow 可以使用预编译好的 libarrow.so

#### 编译整个项目

```
cd DolphinDBPlugin/Arrow
mkdir build
cd build
cp /path_to_dolphindb/libDolphinDB.so ./
cp ../lib/linux_x86_64/libarrow.so.900 ./
cmake .. -DCMAKE_C_COMPILER=/usr/bin/gcc-4.8 -DCMAKE_CXX_COMPILER=/usr/bin/g++-4.8
make -j
```

#### 安装

1. 在 DolphinDB 安装目录的 plugins 子目录下新建 arrow 文件夹。

2. 将下述文件复制至该文件夹下：

   1. libarrow.so.900

   2.  libPluginArrow.so

   3. PluginArrow.txt

3. 启动 DolphinDB server 并加载插件：


```
login("admin", "123456");
loadPlugin("arrow");
```

### 在 Windows 下编译安装

#### 环境准备

- MinGW
- cmake
- arrow 9.0.0

其中 arrow 可以使用预编译好的 libarrow.dll

#### 编译整个项目

```
# MinGW 终端
cd DolphinDBPlugin/Arrow
mkdir build
cp /path_to_dolphindb/libDolphinDB.dll ./
cp /path_to_dolphindb/libarrow.dll ./
cmake -G "MinGW Makefiles" ..
make -j
```

#### 安装

1. 在 DolphinDB 安装目录的 plugins 子目录下新建 arrow 文件夹。

2. 将下述文件复制至该文件夹下：

   1. libarrow.dll

   2. libPluginArrow.dll

3. 将 PluginArrow.txt.win 文件重命名为 PluginArrow.txt 后复制到 arrow 文件夹下。

4. 启动 DolphinDB server 并加载插件：

```
login("admin", "123456");
loadPlugin("arrow");
```