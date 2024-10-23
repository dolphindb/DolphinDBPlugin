# DolphinDB EncoderDecoder Plugin

DolphinDB 提供 EncoderDecoder 插件用于高效解析、转换 json 以及 protobuf 数据。EncoderDecoder 插件可以基于一定规则高效地将 json 数据转换为 DolphinDB 表格数据，也可以基于 proto 格式文件将 protobuf 序列化数据解析为 DolphinDB 表格数据。

本文档仅介绍编译构建方法。通过 [文档中心-EncoderDecoder](https://docs.dolphindb.cn/zh/plugins/EncoderDecoder.html) 查看接口介绍。

## 编译构建

### Linux 编译构建

#### 安装 Protocol Buffers

下载 [Protol Buffers v3.17.1](https://github.com/protocolbuffers/protobuf/releases/download/v3.17.1/protobuf-cpp-3.17.1.tar.gz).

``` shell
tar -xzvf protobuf-cpp-3.17.1.tar.gz
cd protobuf-3.17.1
mkdir -p cmake/build/release
cd cmake/build/release
cmake ../.. -DCMAKE_BUILD_TYPE=Release -Dprotobuf_BUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON -Dprotobuf_BUILD_TESTS=OFF
make
make check

cp -r src/google/ <path_to_plugins>/EncoderDecoder/include/
cp libprotobuf.a <path_to_plugins>/EncoderDecoder/lib/
```

#### 编译插件

``` shell
cd <path_to_plugins>/EncoderDecoder
mkdir build
cd build
cmake ..
make
```

注意：编译之前请确保 libDolphinDB.so 在 gcc 可搜索的路径中。可使用 LD_LIBRARY_PATH 指定其路径，或者直接将其拷贝到 build 目录下。
