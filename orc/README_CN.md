# DolphinDB ORC Plugin

ORC 是一种自描述列式文件格式，专门为 Hadoop 生态设计，可用于高效存储与提取数据，因此适合大批量流数据读取场景。DolphinDB 提供的 ORC 插件支持将 ORC 文件导入和导出 DolphinDB，并且在导入导出过程中自动进行数据类型转换。

本文档仅介绍编译构建方法。通过 [文档中心-ORC](https://docs.dolphindb.cn/zh/plugins/orc.html) 查看接口介绍；通过 [CHANGELOG.md](./CHANGELOG.md) 查看版本发布记录。

## 编译构建

### Linux-x86-64 编译构建

**安装 cmake：**

```bash
sudo apt-get install cmake
```

**安装 cyrus-sasl：**

下载源码：https://github.com/cyrusimap/cyrus-sasl/releases/download/cyrus-sasl-2.1.27/cyrus-sasl-2.1.27.tar.gz

```bash
tar -xf cyrus-sasl-2.1.27.tar.gz
cd cyrus-sasl-2.1.27
./configure CFLAGS=-fPIC CXXFLAGS=-fPIC LDFLAGS=-static
cp ./lib/.libs/libsasl2.a path_to_orc_plugin/lib/linux
```

**安装 openssl：**

```bash
wget https://www.openssl.org/source/old/1.0.2/openssl-1.0.2u.tar.gz
tar -xzf openssl-1.0.2u.tar.gz
cd openssl-1.0.2u
./config shared --prefix=/tmp/ssl -fPIC
make
make install

cp /tmp/ssl/lib/libcrypto.a path_to_orc_plugin/lib/linux
```


**安装 ORC 开发包：**

下载源码：https://www.apache.org/dyn/closer.cgi/orc/orc-1.6.7/orc-1.6.7.tar.gz

```bash
tar -xf orc-1.6.7.tar.gz
cd orc-1.6.7
mkdir build
cd build
cmake .. -DBUILD_JAVA=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON
make package
tar -xf ORC-1.6.7-Linux.tar.gz
cp ORC-1.6.7-Linux/lib/* path_to_orc_plugin/lib/linux
```

**重新编译 libprotobuf.a：**

```bash
cd orc-1.6.7/build/protobuf_ep-prefix/src/
tar -xf v3.5.1.tar.gz
cd protobuf-3.5.1
./autogen.sh
cd cmake
mkdir build
cd build
cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON
make
cp libprotobuf.a path_to_orc_plugin/lib/linux
```

> 请注意，执行 ./autogen.sh 后出现的 ./autogen.sh: line 50: autoreconf: command not found，并不影响接下来的步骤

**重新编译 libsnappy.a：**

```bash
cd orc-1.6.7/build/snappy_ep-prefix/src/
tar -xf 1.1.7.tar.gz
cd snappy-1.17
mkdir build
cd build
cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON
make
cp libsnappy.a path_to_orc_plugin/lib/linux
```

**重新编译 libz.a：**

```bash
cd orc-1.6.7/build/zlib_ep-prefix/src
tar -xf zlib-1.2.11.tar.gz
cd zlib-1.2.11
mkdir build
cd build
cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON
make
cp libz.a path_to_orc_plugin/lib/linux
```

**设置 libDolphinDB.so 库位置：**

```bash
export LIBDOLPHINDB=path_to_libdolphindb/
```

**编译整个项目：**

```bash
mkdir build
cd build
cmake path_to_orc_plugin/
make
```

### Linux arm 编译构建（未发布）

先在arm机器上编译ssl, orc, protobuffer, snappy, zlib依赖库，并把编译出来的静态库拷贝到lib_arm目录。

**编译openssl：**
```
wget https://www.openssl.org/source/old/1.0.1/openssl-1.0.1u.tar.gz
tar -xzf openssl-1.0.1u.tar.gz
cd openssl-1.0.1u
./config shared --prefix=/tmp/ssl -fPIC
make
make install

```

**编译orc开发包：**

下载源码：https://www.apache.org/dyn/closer.cgi/orc/orc-1.6.7/orc-1.6.7.tar.gz

```bash
tar -xf orc-1.6.7.tar.gz
cd orc-1.6.7
mkdir build
cd build
CMAKE_PREFIX_PATH=/tmp/ssl cmake .. -DBUILD_JAVA=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON//指定里前面安装openssl1.0.1u的路径。
make package
tar -xf ORC-1.6.7-Linux.tar.gz
cp ORC-1.6.7-Linux/lib/* path_to_orc_plugin/lib_arm/
```

**重新编译libprotobuf.a：**

```bash
cd orc-1.6.7/build/protobuf_ep-prefix/src/
tar -xf v3.5.1.tar.gz
cd protobuf-3.5.1
./autogen.sh
cd cmake
mkdir build
cd build
cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON
make
cp libprotobuf.a path_to_orc_plugin/lib_arm/
```

**重新编译libsnappy.a：**

```bash
cd orc-1.6.7/build/snappy_ep-prefix/src/
tar -xf 1.1.7.tar.gz
cd snappy-1.1.7
mkdir build
cd build
cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON
make
cp libsnappy.a path_to_orc_plugin/lib_arm/
```

**重新编译libz.a：**

```bash
cd orc-1.6.7/build/zlib_ep-prefix/srcap
tar -xf zlib-1.2.11.tar.gz
cd zlib-1.2.11
mkdir build
cd build
cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON
make
cp libz.a path_to_orc_plugin/lib_arm/
```

使用交叉编译器在x86-64的机器上编译，将lib_arm文件夹从arm机器上复制过来。
**编译整个项目：**

```bash
mkdir build
cd build
cp ../lib_arm/* .
CC=aarch64-linux-gnu-gcc CXX=aarch64-linux-gnu-g++ cmake .. -DBUILD_ARM=1
make
```