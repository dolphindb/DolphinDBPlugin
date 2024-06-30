# DolphinDB MongoDB Plugin

MongoDB 是一个基于分布式文件存储的数据库。DolphinDB de1 mongodb 插件可以建立与 mongodb 服务器的连接，然后导入数据到 DolphinDB 的内存表中。mongodb 插件基于 mongodb-c-driver 开发。本文档仅介绍编译构建方法。通过 [文档中心 - MongoDB](https://docs.dolphindb.cn/zh/plugins/mongodb/mongodb.html) 查看接口介绍。

## 安装构建

### 自行编译

为保证插件能够成功进行编译，将 DolphinDBPlugins 仓库下载到本地（请切换到相应的分支）。

只需要安装 CMake 和对应编译器（linux 为 g++,window 为 MinGW），即可在本地编译 mongodb 插件。

#### Linux

##### 使用 CMake 构建：

安装 CMake：

```
sudo apt-get install cmake
```
构建插件内容：

```
mkdir build
cd build
cmake  ../
make
```

**注意**: 编译之前请确保 libDolphinDB.so 在 gcc 可搜索的路径中。可使用 `LD_LIBRARY_PATH` 指定其路径，或者直接将其拷贝到 build 目录下。

编译后目录下会产生文件 libPluginMongodb.so 和 PluginMongodb.txt。

#### Windows

##### 在 Windows 环境中需要使用 CMake 和 MinGW 编译

- 下载安装 [MinGW](http://www.mingw.org/)。确保将 bin 目录添加到系统环境变量 Path 中。
- 下载安装 [CMake](https://cmake.org/)。

###### 使用 CMake 构建：

在编译开始之前，要将 libDolphinDB.dll 拷贝到 build 文件夹。

构建插件内容：

```
mkdir build                                                        # 新建 build 目录
cp <ServerDir>/libDolphinDB.dll build                 # 拷贝 libDolphinDB.dll 到 build 目录下
cd build
cmake  ../ -G "MinGW Makefiles"
mingw32-make -j4
```

编译后目录下会产生文件 libPluginMongodb.dll 和 PluginMongodb.txt，还会把 <PluginDir>/mongodb/bin/windows 下的 4 个动态库拷贝到该目录下。

> 以下编译的 mongodb-c-driver、snappy、ICU 和 openssl 的依赖库文件，都可以在 <PluginDir>/mongodb/bin 目录下找到。

### 1.3 编译依赖库

#### Linux

我们在 /mongodb/bin 目录下提供预编译的依赖库 libmongoc, libbson, libicudata, libicuuc。你也可按照本节描述的步骤手动编译依赖库。

##### 安装版本 1.0.2 的 openssl

 ```
wget https://www.openssl.org/source/old/1.0.2/openssl-1.0.2i.tar.gz
tar -xzf openssl-1.0.2i.tar.gz
cd openssl-1.0.2i
./config --prefix=/usr/local/openssl1.0.2 -fPIC
make 
sudo make install
```
--prefix 是为了指定安装位置，安装 mongo-c-driver 时会使用到这个版本的 openssl 的头文件和静态库。

##### 安装 snappy

```
wget https://github.com/google/snappy/archive/1.1.7.tar.gz
tar -zxf 1.1.7.tar.gz
cd snappy-1.1.7/cmake
CXXFLAGS="-fPIC" cmake ..
make
sudo make install
```

##### 安装 ICU

```
wget https://github.com/unicode-org/icu/releases/download/release-52-2/icu4c-52_2-src.tgz
tar -xzf icu4c-52_2-src.tgz
cd icu/source
./configure
make
sudo make install
```

##### 安装 mongo-c-driver

需要设置环境变量，在命令行中设置，正是刚刚安装 openssl 的位置。
```
export OPENSSL_ROOT_DIR=/usr/local/openssl1.0.2
export OPENSSL_CRYPTO_LIBRARY=/usr/local/openssl1.0.2/lib
export OPENSSL_INCLUDE_DIR=/usr/local/openssl1.0.2/include/

wget https://github.com/mongodb/mongo-c-driver/releases/download/1.13.0/mongo-c-driver-1.13.0.tar.gz
tar -xzf mongo-c-driver-1.13.0.tar.gz
cd mongo-c-driver-1.13.0
mkdir cmake-build
cd cmake-build
cmake -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=OFF ..
```
这里我们可以在终端看到 mongodb-c-driver 需要的依赖是否安装完全。
```
make
sudo make install
```

##### 准备依赖库

将 libDolphinDB.so 拷到编译目录（DolphinDBPlugin/mongodb/build）。

```
cd DolphinDBPlugin/mongodb/bin/linux
cp <ServerDir>/libDolphinDB.so . 
cp /usr/local/lib/libmongoc-1.0.so.0 .
cp /usr/local/lib/libbson-1.0.so.0 .
cp /usr/local/lib/libicudata.so.52 .
cp /usr/local/lib/libicuuc.so.52 .
```
