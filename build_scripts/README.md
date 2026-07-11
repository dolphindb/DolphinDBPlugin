# 构建说明

## 编译参数

- 通用编译参数通过标准的 cmake 命令传递，详见 util.sh
- 对于各个库提供的选项，尽量通过修改源码中 CMakeLists.txt
- 官方提供 cmake target 的库，使用 CMAKE_PREFIX_PATH 指定依赖路径
- OpenSSL 使用 OPENSSL_ROOT_DIR

## 依赖库列表

### 已支持默认参数 cmake 的库

| name | build root | dependency |
| ---- | ---------- | ---------- |
| abseil |
| curl | | OpenSSL |
| librdkafka | | OpenSSL <br> zlib/zstd/lz4/snappy <br> sasl2/krb5 |
| libzmq |
| lz4 | build/cmake |
| protobuf | | abseil <br> zlib |
| pulsar | | OpenSSL/curl <br> zlib/zstd/lz4/snappy <br> protobuf <br> boost |
| snappy |
| zlib |
| zstd | build/cmake |
| thrift | | OpenSSL <br> boost |
