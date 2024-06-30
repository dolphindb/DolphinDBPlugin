# DolphinDB TCPSocket 插件使用说明

通过 DolphinDB 的 TCPSocket 插件，用户可以创建 TCP 连接并连接到指定的 IP 和端口，然后进行数据的接收和发送。本文档仅介绍编译构建方法。通过 [文档中心 - TCPSocket](https://docs.dolphindb.cn/zh/plugins/tcpsocket.html) 查看接口介绍。

## 编译构建

### Linux

```
mkdir build
cd build
cmake  ../
make
```

> **注意**：编译之前请确保 libDolphinDB.so 在 gcc 可搜索的路径中。可使用 `LD_LIBRARY_PATH` 指定其路径，或者直接将其拷贝到 build 目录下。

编译后目录下会产生两个文件：libPluginTCPSocket.so 和 PluginTCPSocket.txt。
