# DolphinDB zmq Plugin

ZeroMQ（zmq）是一个可伸缩的分布式或并发应用程序设计的高性能异步消息库。它提供了一个消息队列库，但与面向消息的中间件不同，zmq 可以在没有专门的消息代理的情况下运行；该库名字中的 Zero 意为零代理。详情可参考：[ZeroMQ](https://zeromq.org/)。通过 DolphinDB 的 zmq 插件，用户可以创建 zmq socket，完成 zmq 消息通信的常见操作，包含通过请求应答机制的会话建立、发布、订阅以及消息的管道传输。本文档仅介绍编译构建方法。通过 [文档中心 - zmq](https://docs.dolphindb.cn/zh/plugins/zmq/zmq.html) 查看接口介绍；通过 [CHANGELOG.md](./CHANGELOG.md) 查看版本发布记录。

## 编译构建

### 编译 libzmq

下载 [libzmq-4.3.4](https://github.com/zeromq/libzmq/releases/tag/v4.3.4)

```bash
cd libzmq-4.3.4
cp include/zmq.h /path/to/PluginZmq/bin/include/
mkdir build && cd build
cmake ..
make -j8
cp lib/libzmq.a /path/to/PluginZmq/bin/linux64/
```

### 获取 cppzmq 头文件

下载 [cppzmq-4.7.1](https://github.com/zeromq/cppzmq/releases/tag/v4.7.1)

```bash
cd cppzmq-4.7.1
cp zmq.hpp /path/to/PluginZmq/bin/include/
```

### 构建插件

构建插件内容

```
mkdir build
cd build
cmake  ../
make
```

> **注意**：编译之前请确保 libDolphinDB.so 在 gcc 可搜索的路径中。可使用 `LD_LIBRARY_PATH` 指定其路径，或者直接将其拷贝到 build 目录下。

编译后目录下会产生两个文件：libPluginZmq.so 和 PluginZmq.txt。