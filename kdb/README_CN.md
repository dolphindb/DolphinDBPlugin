# DolphinDB kdb+ Plugin

Kdb+ 是可以用于存储、分析、处理和检索大型数据集的列式关系型时间序列数据库。为便于从 Kdb+ 迁移数据，DolphinDB 提供了 kdb+ 插件，支持导入 kdb+ 数据库存储在磁盘上的数据。通过 loadTable 和 loadFile 接口导入 DolphinDB 内存表。

本文档仅介绍编译构建和测试方法。通过 [文档中心-kdb](https://docs.dolphindb.cn/zh/plugins/kdb/kdb.html) 查看接口介绍。

## 编译构建

### Linux 编译构建

**编译 zlib**

```
wget http://www.zlib.net/zlib-1.2.11.tar.gz
tar -zxf zlib-1.2.11.tar.gz
cd zlib-1.2.11
CFLAGS="-fPIC" ./configure  --prefix=/tmp/zlib
make
make install
```
拷贝 /tmp/zlib/lib/libz.a 到 <path_to_kdb>/lib/linux

**编译 snappy**

下载 [snappy-release](https://github.com/google/snappy/tree/1.1.10)
```
mkdir build
cd build
cmake .. -DCMAKE_CXX_FLAGS="-fPIC"
make -j10
```
拷贝 ./build/libsnappy.a 到插件文件夹 <path_to_kdb>/lib/linux

**编译 lz4**
```
wget https://github.com/lz4/lz4/releases/download/v1.9.4/lz4-1.9.4.tar.gz
tar -xzvf lz4-1.9.4.tar.gz
cd lz4-1.9.4
make
```
拷贝 ./lib/liblz4.a  到插件文件夹 <path_to_kdb>/lib/linux

**编译 kdb 插件**
```
mkdir build
cd build
cmake ..
make
```

## 测试方法

为方便开发者进行自测，kdb 插件自2.00.11版本起提供自行测试的数据、脚本。以下将介绍测试文件和测试方法。

### 测试文件的组成

测试必需的文件在本插件文件夹下 test 文件夹中。以下为详细说明：

```
kdb
├── lib
├── src
├── test                  // 插件目录下 test 文件夹，包含所有测试需要的文件
│   ├── data              // 测试时需要用到的一些数据文件，包括 kdb 持久化文件和一些验证文件
│   │   └── ...
│   ├── setup             // 配置参数存放文件夹
│   │   └── settings.txt  // 具体存放配置参数的 txt 文件，在运行测试命令前需要修改
│   └── test_kdb.txt      // dolphindb kdb 插件测试脚本文件
├── CMakeLists.txt
└── ...
```

### 测试方法

1. 修改 test/setup/settings.txt 文件下的参数。以下是 kdb 测试脚本运行的一些必要配置：

    DATA_DIR： kdb 测试数据，在 /test/data 目录下提供，需修改测试时所放目录前缀。

    HOST：启动的 kdb+ 数据库的 IP 地址。

    PORT：启动的 kdb+ 数据库设置的监听端口。

    usernamePassword：连接启动的 kdb+ 数据库的用户名和密码。

    pluginTxtPath：编译后的插件 txt 文件路径，需在同目录下有对应的插件动态库文件。

2. 启动一个可以连接的 kdb+ 数据库，启动方法参考 connecting-to-a-kdb-process。

3. 运行 dolphindb。

4. 运行以下 dolphindb 脚本：

    ```
    login(`admin,`123456);
    test("<plugin_src_path>/test/test_kdb.txt");
    ```

    测试结果将会显示在屏幕上。其他 test 函数的使用方法与配置可以参考 DolphinDB 的 test 函数。