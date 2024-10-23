# DolphinDB Feather Plugin

Apache Arrow Feather 文件采用列式存储格式，可用于高效存储与提取数据。DolphinDB 提供的 Feather 插件支持高效的将 Feather 文件导入和导出 DolphinDB，并且在导入导出过程中自动进行数据类型转换。本插件使用了 Arrow 的 [arrow 开源库](https://github.com/apache/arrow) 的 feather 读写接口。

本文档仅介绍编译构建方法。通过 [文档中心-feather](https://docs.dolphindb.cn/zh/plugins/feather/feather.html) 查看接口介绍；通过 [CHANGELOG.md](./CHANGELOG.md) 查看版本发布记录。

## 编译构建

### Linux 编译构建

**初始化环境配置**

(1) 编译 Feather 开发包：

```shell
git clone https://github.com/apache/arrow.git
cd arrow/cpp
mkdir build
cd build
cmake .. -DARROW_BUILD_STATIC=ON -DARROW_BUILD_SHARED=OFF -DARROW_DEPENDENCY_USE_SHARED=OFF -DARROW_WITH_ZLIB=ON -DARROW_WITH_ZSTD=ON -DARROW_WITH_LZ4=ON
make -j
```

(2) 编译完成后，拷贝以下文件到插件文件夹中的相应目录：

| **Files**                                                   | **Target Directory** |
| ------------------------------------------------------------ | -------------------- |
| arrow/cpp/src/arrow                                          | ./include            |
| arrow/cpp/build/release/libarrow.a<br/>arrow/cpp/build/jemalloc_ep-prefix/src/jemalloc_ep/lib/libjemalloc_pic.a<br/>arrow/cpp/build/zstd_ep-install/lib64/libzstd.a<br/>arrow/cpp/build/zlib_ep/src/zlib_ep-install/lib/libz.a<br/>arrow/cpp/build/lz4_ep-prefix/src/lz4_ep/lib/liblz4.a | ./lib/linux          |


**注意**
如果编译过程中出现上表 Files 列出的文件不存在，可以手动编译以下三个库。

1. 如果 libz.a 无法找到，执行以下命令：
```shell
wget http://www.zlib.net/zlib-1.2.12.tar.gz
tar -zxf zlib-1.2.12.tar.gz
cd zlib-1.2.12
CFLAGS="-fPIC" ./configure
make
```
然后在 zlib-1.2.12 目录下找到 libz.a，放到插件文件夹下的./lib/linux 目录中。

2. 如果 liblz4.a 无法找到，执行以下命令：
```shell
wget https://github.com/lz4/lz4/archive/8f61d8eb7c6979769a484cde8df61ff7c4c77765.tar.gz
tar -xzvf 8f61d8eb7c6979769a484cde8df61ff7c4c77765.tar.gz
cd lz4-8f61d8eb7c6979769a484cde8df61ff7c4c77765/
make
```
然后在 lz4-8f61d8eb7c6979769a484cde8df61ff7c4c77765/lib 目录下找到 liblz4.a，放到插件文件夹下的./lib/linux 目录中。

3. 如果 libzstd.a 无法找到，执行以下命令：
```shell
wget https://github.com/facebook/zstd/releases/download/v1.5.2/zstd-1.5.2.tar.gz
tar -zxvf zstd-1.5.2.tar.gz
cd zstd-1.5.2/
cd build/cmake/
mkdir build
cd build/
cmake ..
make -j
```
然后在 zstd-1.5.2/build/cmake/build/lib 目录下找到 libzstd.a，放到插件文件夹下的 ./lib/linux 目录中。

**编译插件**

```linux shell
cd /path/to/plugins/feather
mkdir build
cd build
cmake ..
make
```
注意：编译之前请确保libDolphinDB.so在gcc可搜索的路径中。可使用LD_LIBRARY_PATH指定其路径，或者直接将其拷贝到build目录下。

