# DolphinDB Parquet Plugin

Apache Parquet 文件采用列式存储格式，可用于高效存储与提取数据。DolphinDB 提供的 Parquet 插件支持将 Parquet 文件导入和导出 DolphinDB，并进行数据类型转换。本文档仅介绍编译构建方法。通过 [文档中心 - Parquet](https://docs.dolphindb.cn/zh/plugins/parquet/parquet.html) 查看接口介绍；通过 [CHANGELOG.md](./CHANGELOG.md) 查看版本发布记录。

## 编译构建

### Linux 编译

(1) 安装 CMake：

```
sudo apt-get install cmake
```

(2) 安装 zlib：

```
sudo apt-get install zlib1g
```

(3) 编译 Parquet 开发包：

```
git clone https://github.com/apache/arrow.git
cd arrow/cpp
mkdir build
cd build
cmake .. -DARROW_PARQUET=ON -DARROW_IPC=ON -DARROW_BUILD_INTEGRATION=ON -DARROW_BUILD_STATIC=ON -DPARQUET_BUILD_SHARED=OFF -DARROW_BUILD_SHARED=OFF -DARROW_DEPENDENCY_USE_SHARED=OFF -DARROW_WITH_ZLIB=ON -DARROW_WITH_SNAPPY=ON -DARROW_WITH_ZSTD=ON -DARROW_WITH_LZ4=ON -DARROW_WITH_BZ2=ON
```

> **请注意：编译环境中的依赖库 libstdc++ 需要和 dolphindb server 下版本的一致。DolphinDB 提供的预编译版本插件支持 zlib, snappy, zstd, lz4 和 bz2 压缩格式，您可在此基础上根据需要支持的压缩类型增减编译选项。详情请参考 [Apache Arrow 相关文档](https://github.com/apache/arrow/blob/master/docs/source/developers/cpp/building.rst#optional-components)。**

（4）编译完成后，拷贝以下文件到目标目录：

| **Files **                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                | **Target Directory**         |
|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|------------------------------|
| arrow/cpp/build/src/parquet/parquet_version.h                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             | ./parquetApi/include/parquet |
| arrow/cpp/src/arrow<br>arrow/cpp/src/parquet<br>arrow/cpp/src/generated                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   | ./parquetApi/include         |
| arrow/cpp/build/release/libarrow.a<br>arrow/cpp/build/release/libparquet.a<br>arrow/cpp/build/thrift_ep-install/lib/libthrift.a<br>arrow/cpp/build/utf8proc_ep-install/lib/libutf8proc.a<br>arrow/cpp/build/jemalloc_ep-prefix/src/jemalloc_ep/lib/libjemalloc_pic.a<br>arrow/cpp/build/zstd_ep-install/lib64/libzstd.a<br>arrow/cpp/build/zlib_ep/src/zlib_ep-install/lib/libz.a<br>arrow/cpp/build/snappy_ep/src/snappy_ep-install/lib/libsnappy.a<br>arrow/cpp/build/lz4_ep-prefix/src/lz4_ep/lib/liblz4.a<br>arrow/cpp/build/bzip2_ep-install/lib/libbz2.a<br>arrow/cpp/build/boost_ep-prefix/src/boost_ep/stage/lib/libboost_regex.a | ./parquetApi/lib/linux       |

编译整个项目：

```
mkdir build
cd build
cmake ../path_to_parquet_plugin/
make
```