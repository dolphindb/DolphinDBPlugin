# DolphinDB Kafka 插件

Kafka 是一种高吞吐量的分布式消息队列，DolphinDB 提供了 kafka 插件用于发布或订阅 Kafka 流服务。kafka 插件支持以 Json 或 DolphinDB序列化格式发送、接收数据流，也可以通过自定义回调函数的方式，订阅数据流写入 DolphinDB 中。插件基于开源库 [librdkafka](https://github.com/confluentinc/librdkafka), [cppkafka](https://github.com/mfontanini/cppkafka) 进行开发。

本文档仅介绍编译构建方法。通过 [文档中心-kafka](https://docs.dolphindb.cn/zh/plugins/kafka/kafka.html) 查看接口介绍；通过 [CHANGELOG.md](./CHANGELOG.md) 查看版本发布记录。

## 编译构建

### Linux 编译构建

**前提**

安装 CMake。Ubuntu 用户可执行以下命令（Centos 用户将 apt 改为 yum 即可）：

``` shell
sudo apt install cmake
```

该项目依赖于 “cppkafka”, “boost” 以及 “librdkafka”。执行以下命令进行下载：

``` shell
# The ubuntu which is a low version such as 14.04 will not
# find rdkafka, and you need to compile the librdkafka manully.
# The address is https://github.com/edenhill/librdkafka

# For ubuntu install
sudo apt install librdkafka-dev
sudo apt install libboost-dev
sudo apt install libssl-dev

# For Centos install
sudo yum install librdkafka-devel
sudo yum install boost-devel
sudo yum install openssl-devel

cd /path/to/DolphinDBPlugin
git submodule update --init --recursive
```

若 submodule 下载太慢，可以从隐藏文件。 gitmodules 中获取 cppkafka git 链接进行下载。

```shell
git clone https://github.com/mfontanini/cppkafka.git
```

将 libDolphinDB.so 文件拷贝至 bin/linux64 或 /lib 目录：

```shell
cp /path/to/dolphindb/server/libDolphinDB.so /path/to/DolphinDBPlugin/kafka/bin/linux64
```

**使用 cmake 构建**

构建项目：

```shell
cd /path/to/DolphinDBPlugin/kafka
cd cppkafka
mkdir build
cd build
cmake ..
make
sudo make install
cd ../..
mkdir build
cd build
cmake ..
make
```
