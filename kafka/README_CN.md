# DolphinDB Kafka 插件
- [DolphinDB Kafka 插件](#dolphindb-kafka-插件)
  - [1. 预编译安装](#1-预编译安装)
  - [2. 手动编译安装](#2-手动编译安装)
    - [2.1 前提](#21-前提)
    - [2.2 使用 cmake 构建](#22-使用-cmake-构建)
    - [2.3  复制结果](#23--复制结果)
  - [3. API 详情](#3-api-详情)
    - [3.1 加载 Kafka 插件](#31-加载-kafka-插件)
    - [3.2 生产者（Producer）](#32-生产者producer)
    - [3.3 消费者（Consumer）](#33-消费者consumer)
    - [3.4 队列](#34-队列)
    - [3.5 事件](#35-事件)
    - [3.6 全局设置](#36-全局设置)
  - [4 示例](#4-示例)

DolphinDB 提供了 Kafka 插件用于发布或订阅 Kafka 流服务。该插件支持以下数据类型的序列化和反序列化：

- DolphinDB 标量
- Kafka Java API 的内置类型：String[UTF-8], Short, Integer, Long, Float, Double, Bytes, byte[] 以及 ByteBuffer
- 以上数据类型所组成的向量

## 1. 预编译安装

将文件夹下载解压到根目录下，在 Linux 中执行以下命令：

``` shell
export LD_LIBRARY_PATH="LD_LIBRARY_PATH:/path/to/DolphinDBPlugin/kafka/bin/linux"
```

在 Linux 上启动 DolphinDB 服务，并在 DolphinDB 客户端运行以下命令加载插件：

```shell
loadPlugin("/path/to/DolphinDBPlugin/kafka/bin/linux/PluginKafka.txt")
```

## 2. 手动编译安装

### 2.1 前提

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

### 2.2 使用 cmake 构建

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

### 2.3  复制结果

将 libPluginKafka.so 及 PluginKafka.txt 文件拷贝至 bin/linux64

``` shell
cp /path/to/DolphinDBPlugin/kafka/build/libPluginKafka.so /path/to/DolphinDBPlugin/kafka/bin/linux64
cp /path/to/DolphinDBPlugin/kafka/build/PluginKafka.txt /path/to/DolphinDBPlugin/kafka/bin/linux64
```

## 3. API 详情

在加载和使用 Kafka 插件前先下载 Kafka 并启动 Zookeeper 和 Kafka 服务器，具体可参考 [Kafka 中文文档-ApacheCN](https://kafka.apachecn.org/quickstart.html)。

### 3.1 加载 Kafka 插件

在 DolphinDB 客户端运行以下命令加载插件，需要将目录替换为 luginKafka 文本文件所在的位置：

``` shell
loadPlugin("/path/to/DolphinDBPlugin/kafka/bin/linux/PluginKafka.txt")
```

### 3.2 生产者（Producer）

#### 3.2.1 初始化 <!-- omit in toc -->

**语法**

```shell
kafka::producer(config);
```

**参数**

- 'config'：字典，表示 Kafka 生产者的配置。字典的键是一个字符串，值是一个字符串或布尔值。有关 Kafka 配置的更多信息，请参阅 [Kafka 配置](https://github.com/edenhill/librdkafka/blob/master/CONFIGURATION.md)。

**详情**

根据指定配置创建一个 Kafka 生产者，并返回句柄。

#### 3.2.2 生产消息 <!-- omit in toc -->

**语法**

```
kafka::produce(producer, topic, key, value, json, [partition]);
```

**参数**

- 'producer'：Kafka 生产者的句柄
- 'topic'：Kafka 的主题
- 'key'：Kafka 生产者配置字典的键
- 'value'：Kafka 生产者配置字典的值
- 'json'：表示是否以 json 格式传递数据
- 'partition'：可选参数，整数，表示 Kafka 的 broker 分区号。

**详情**

选择是否以 json 格式在指定分区生成键值数据。

注意：

请不要一次性生成太多数据，否则可能导致 Local: Queue full 异常

#### 3.2.3 发送生产者所有缓存记录 <!-- omit in toc -->

**语法**

```
kafka::producerFlush(producer);
```

**参数**

- 'producer'：Kafka 生产者的句柄

**详情**

将生产者的所有缓存记录发送到 Kafka。

#### 3.2.4 获取请求最大等待时间，类似于 Kafka Producer 的 request.timeout.ms 参数 <!-- omit in toc -->

**语法**

```
kafka::getProducerTime(producer)
```

**参数**

- producer：Kafka 生产者的句柄

#### 3.2.5 设置请求最大等待时间 <!-- omit in toc -->

**语法**

```
kafka::setProducerTime(producer, timeout)
```

**参数**

- 'producer'：Kafka 生产者的句柄
- 'timeout'：表示请求最大等待时间

### 3.3 消费者（Consumer）

#### 3.3.1 初始化 <!-- omit in toc -->

**语法**

```
kafka::consumer(config)
```

**参数**

- 'config'：字典，表示 Kafka 消费者的配置。字典的键是一个字符串，值是一个元组。有关 Kafka 配置的更多信息，请参阅 <https://github.com/edenhill/librdkafka/blob/master/CONFIGURATION.md>。

SASL 协议认证的用户可参考以下示例：

```
consumerCfg = dict(string, any);
consumerCfg["metadata.broker.list"] = "localhost";
consumerCfg["group.id"] = "test";
consumerCfg["sasl.mechanisms"] = "PLAIN";
consumerCfg["security.protocol"] = "sasl_plaintext";
consumerCfg["sasl.username"] = "admin";
consumerCfg["sasl.password"] = "admin";
consumer = kafka::consumer(consumerCfg);
topics=["test"];
kafka::subscribe(consumer, topics);
kafka::consumerPoll(consumer);

```

**详情**

根据指定配置创建一个 Kafka 消费者，并返回句柄。

#### 3.3.2 订阅 <!-- omit in toc -->

**语法**

```
kafka::subscribe(consumer, topics)
```

**参数**

- 'consumer'：Kafka 消费者的句柄
- 'topics'：字符串向量，表示订阅的主题

**详情**

订阅一个 Kafka 主题。

#### 3.3.3 取消订阅 <!-- omit in toc -->

**语法**

```
kafka::unsubscribe(consumer)
```

**参数**

- 'consumer'：Kafka 消费者的句柄

**详情**

取消所有 Kafka 主题的订阅。

#### 3.3.4 轮询获取消息 <!-- omit in toc -->

**语法**

```
kafka::consumerPoll(consumer, [timeout])
```

**参数**

- 'consumer'：Kafka 消费者的句柄
- 'timeout'：表示请求获取消息的最大等待时间

**详情**

将订阅数据保存到 DolphinDB。返回一个元组。

第一个元素表示出错信息的字符串，若成功获取则为空。 第二个元素是一个元组，其元素包括：主题、分区、键、值和消费者收到数据的时间戳。

该函数将阻塞当前线程，轮询最大等待时间默认为 1000 毫秒。我们推荐使用函数 consumerPollBatch 提交多个轮询任务。

#### 3.3.5 多次轮询 <!-- omit in toc -->

**语法**

```
kafka::consumerPollBatch(consumer, batch_size, [time_out])
```

**参数**

- 'consumer'：Kafka 消费者的句柄
- 'batch_size'：表示想要获取的消息数量
- 'timeout'：表示请求获取消息的最大等待时间

#### 3.3.6 多线程轮询 <!-- omit in toc -->

**语法**

```
kafka::createSubJob(consumer, table, parser, description, [timeout])
```

**参数**

- 'consumer'：Kafka 消费者的句柄
- 'table'：表示存储消息的表
- 'parser'：处理输入数据的函数，返回一个表。可以使用 mseed::parser 或自定义函数
  输入参数均为string类型，数目可以为 1-3 个，第一个参数为 msg 的 value，第二个参数为 msg 的 key，第三个参数为 msg 的 topic。
- 'description'：对线程进行描述的字符串
- 'timeout'：表示请求获取消息的最大等待时间

 注意：

若创建的子任务订阅消费主题所包含的 partition 也被其它子任务订阅消费，则该 partition 内的消息会被拆分后分发至所有订阅它的子任务中，将导致这些子任务获取的订阅消息不完整。

#### 3.3.7 获取多线程状态 <!-- omit in toc -->

**语法**

```
kafka::getJobStat()
```

**参数**

- 无

#### 3.3.8 结束轮询线程 <!-- omit in toc -->

**语法**

```
kafka::cancelSubJob(connection)
```

**参数**

- 'connection'：是函数 kafka::createSubJob 的返回值，或从函数 getJobStat() 获得的订阅 Id，数据类型为 LONG，INT 或 STRING。

#### 3.3.9 轮询返回字典 <!-- omit in toc -->

**语法**

```
kafka::pollDict(consumer, batch_size, [timeout])
```

**参数**

- 'consumer'：Kafka 消费者的句柄
- 'batch_size'：表示想要获取的消息数量
- 'timeout'：表示请求获取消息的最大等待时间

**详情**

将订阅数据保存到 DolphinDB。返回一个包含消息键值对的字典。

#### 3.3.10 提交 <!-- omit in toc -->

**语法**

```
kafka::commit(consumer)
```

**参数**

- 'consumer': Kafka 消费者的句柄

**详情**

将最新处理的消息在文件中的位置（偏移量）同步提交给生产者。 如果没有最新的偏移量，则抛出异常。

#### 3.3.11 提交主题 <!-- omit in toc -->

**语法**

```
kafka::commitTopic(consumer, topics, partitions, offsets)
```

**参数**

- 'consumer'：Kafka 消费者的句柄
- 'topics'：字符串向量，表示订阅的主题
- 'partitions'：整型向量，表示每个主题的分区
- 'offsets'：整型向量，表示每个主题的偏移量

#### 3.3.12 异步提交 <!-- omit in toc -->

**语法**

```
kafka::asyncCommit(consumer)
```

**参数**

- 'consumer'：Kafka 消费者的句柄

**详情**

将最新处理的消息在文件中的位置（偏移量）异步提交到用来保存消息偏移量的 _consumer_offset 主题。

#### 3.3.13 异步提交主题 <!-- omit in toc -->

**语法**

```
kafka::asyncCommitTopic(consumer, topics, partitions, offsets)
```

**参数**

- 'consumer': Kafka 消费者的句柄
- 'topics'：字符串向量，表示订阅的主题
- 'partitions'：整型向量，表示每个主题的分区
- 'offsets'：整型向量，表示每个主题的偏移量

#### 3.3.14 获取请求最大等待时间 <!-- omit in toc -->

**语法**

```
kafka::getConsumerTime(consumer)
```

**参数**

- 'consumer'：Kafka 消费者的句柄

#### 3.3.15 设置请求最大等待时间 <!-- omit in toc -->

**语法**

```
kafka::setConsumerTime(consumer, timeout)
```

**参数**

- 'consumer'：Kafka 消费者的句柄
- 'timeout'：表示请求获取消息的最大等待时间

#### 3.3.16 手动指定主题 <!-- omit in toc -->

**语法**

```
kafka::assign(consumer, topics, partitions, offsets)
```

**参数**

- 'consumer'：Kafka 消费者的句柄
- 'topics'：字符串向量，表示订阅的主题
- 'partitions'：整型向量，表示每个主题的分区
- 'offsets'：整型向量，表示每个主题的偏移量

**详情**

与 kafka::subscribe(consumer, topics) 不同，该函数为消费者手动指定特定的主题、分区和偏移量。

#### 3.3.17 手动撤回主题 <!-- omit in toc -->

**语法**

```
kafka::unassign(consumer)
```

**参数**

- 'consumer'：Kafka 消费者的句柄

**详情**

手动撤回消费者指定的所有主题。

#### 3.3.18 获取消费者指定主题 <!-- omit in toc -->

**语法**

```
kafka::getAssignment(consumer)
```

**参数**

- 'consumer'：Kafka 消费者的句柄

#### 3.3.19 获取偏移量 <!-- omit in toc -->

**语法**

```
kafka::getOffset(consumer, topic, partition)
```

**参数**

- 'consumer'：Kafka 消费者的句柄
- 'topics'：字符串向量，表示订阅的主题
- 'partitions'：整型向量，表示每个主题的分区

**详情**

获取消费者偏移量。

#### 3.3.20 获取已提交偏移量 <!-- omit in toc -->

**语法**

```
kafka::getOffsetCommitted(consumer, topics, partitions, offsets, [timeout])
```

**参数**

- 'consumer'：Kafka 消费者的句柄
- 'topics'：字符串向量，表示订阅的主题
- 'partitions'：整型向量，表示每个主题的分区
- 'offsets'：整型向量，表示每个主题的偏移量
- 'timeout'：表示请求获取消息的最大等待时间

**详情**

获取指定主题或分区列表提交的偏移量。

#### 3.3.21 获取偏移量位置 <!-- omit in toc -->

**语法**

```
kafka::getOffsetPosition(consumer, topics, partitions)
```

**参数**

- 'consumer'：Kafka 消费者的句柄
- 'topics'：字符串向量，表示订阅的主题列表
- 'partitions'：整型向量，表示每个主题的分区列表

**详情**

获取指定主题或分区列表提交的偏移量位置。

#### 3.3.22 存储消费者偏移量 <!-- omit in toc -->

**语法**

```
kafka::storeConsumedOffset(consumer)
```

**参数**

- 'consumer'：Kafka 消费者的句柄

**详情**

存储当前消费者指定的主题或分区上的偏移量。
调用本函数时，consumer 中需要设置 “enable.auto.offset.store=false”，”enable.auto.commit=true”，否则可能出现报错。

#### 3.3.23 存储偏移量 <!-- omit in toc -->

**语法**

```
kafka::storeOffset(consumer, topics, partitions, offsets)
```

**参数**

- 'consumer'：Kafka 消费者的句柄
- 'topics'：字符串向量，表示订阅的主题
- 'partitions'：整型向量，表示每个主题的分区
- 'offsets'：整型向量，表示每个主题的偏移量

**详情**

存储当前给定的主题或分区上的偏移量。
调用本函数时，consumer 中需要设置 “enable.auto.offset.store=false”，”enable.auto.commit=true”，否则可能出现报错。

#### 3.3.24 获取消费者成员 ID <!-- omit in toc -->

**语法**

```
kafka::getMemId(consumer)
```

**参数**

- 'consumer'：Kafka 消费者的句柄

**详情**

获取消费者成员 ID。
#### 3.3.25 轮询获取二进制消息 <!-- omit in toc -->

**语法**

```
kafka::pollByteStream(consumer, [timeout])
```

**参数**

- 'consumer'：Kafka 消费者的句柄
- 'timeout'：表示请求获取消息的最大等待时间

**详情**

将订阅数据保存到 DolphinDB。返回一个STRING类型的标量。该标量为获取到的 kafka 消息中的 value，不包含 key 和 topic。


### 3.4 队列

#### 3.4.1 获取主队列 <!-- omit in toc -->

**语法**

```
kafka::getMainQueue(consumer)
```

**参数**

- 'consumer'：Kafka 消费者的句柄

**详情**

获取消费者对应的全局事件队列。

#### 3.4.2 获取消费者队列 <!-- omit in toc -->

**语法**

```
kafka::getConsumerQueue(consumer)
```

**参数**

- 'consumer'：Kafka 消费者的句柄

**详情**

获取消费者对应消费者组的消息队列。

#### 3.4.3 获取分区队列 <!-- omit in toc -->

**语法**

```
kafka::getPartitionQueue(consumer, topic, partition)
```

**参数**

- 'consumer'：Kafka 消费者的句柄
- 'topics'：字符串向量，表示订阅的主题
- 'partitions'：整型向量，表示每个主题的分区

**详情**

获取属于该分区的队列。如果消费者没有分配到这个分区，则返回一个空队列。

#### 3.4.4 获取队列长度 <!-- omit in toc -->

**语法**

```
kafka::queueLength(queue)
```

**参数**

- 'queue'：Kafka 队列的句柄

**详情**

返回队列长度

注意：

若插件部署在 ARM 架构的服务器上，调用该函数返回的结果可能不符合预期。

#### 3.4.5 转发到队列 <!-- omit in toc -->

**语法**

```
kafka::forToQueue(queue, forward_queue)
```

**参数**

- 'queue'：要转发的 Kafka 队列的句柄
- 'forward_queue'：转发到的队列的句柄

#### 3.4.6 禁止转发 <!-- omit in toc -->

**语法**

```
kafka::disforToQueue(queue)
```

**参数**

- 'queue'：Kafka 队列的句柄

**详情**

 停止转发 queue 到其它队列。

#### 3.4.7 设置队列请求最大等待时间 <!-- omit in toc -->

**语法**

```
kafka::setQueueTime(queue, timeout)
```

**参数**

- 'queue'：Kafka 队列的句柄
- 'timeout'：表示队列请求的最大等待时间

#### 3.4.8 获取队列请求最大等待时间 <!-- omit in toc -->

**语法**

```
kafka::getQueueTime(queue)
```

**参数**

- 'queue'：Kafka 队列的句柄

**详情**

获取队列请求最大等待时间

**3.4.9 轮询队列消息**<!-- omit in toc -->

**语法**

```
kafka::queuePoll(queue, [timeout])
```

**参数**

- 'queue'：Kafka 队列的句柄
- 'timeout'：表示队列请求的最大等待时间

#### 3.4.10 多次轮询队列消息 <!-- omit in toc -->

**语法**

```
kafka::queuePollBatch(queue, batch_size, [timeout])
```

**参数**

- 'queue'：Kafka 队列的句柄
- 'batch_size'：表示想要获取的消息数量
- 'timeout'：表示队列请求的最大等待时间

### 3.5 事件

#### 3.5.1 获取队列事件 <!-- omit in toc -->

**语法**

```
kafka::queueEvent(queue)
```

**参数**

- 'queue'：Kafka 队列的句柄

**详情**

提取队列中的事件。

注意：

在删除一个 consumer 前，请确保由它生成的 event 均已成功释放（通过 event=NULL 进行资源释放），否则可能出现程序卡死的情况。

#### 3.5.2 获取事件名称 <!-- omit in toc -->

**语法**

```
kafka::getEventName(event)
```

**参数**

- 'event'：Kafka 事件的句柄

**详情**

返回事件的名称

#### 3.5.3 从事件获取消息 <!-- omit in toc -->

**语法**

```
kafka::eventGetMessage(event)
```

**参数**

- 'event'：Kafka 事件的句柄

**详情**

获取该事件中的所有消息。

#### 3.5.4 获取事件消息数 <!-- omit in toc -->

**语法**

```
kafka::getEventMessageCount(event)
```

**参数**

- 'event'：Kafka 事件的句柄

#### 3.5.5 获取事件报错信息 <!-- omit in toc -->

**语法**

```
kafka::eventGetError(event)
```

**参数**

- 'event'：Kafka 事件的句柄

**详情**

返回事件中的报错信息。

#### 3.5.6 获取事件的分区 <!-- omit in toc -->

**语法**

```
kafka::eventGetPart(event)
```

**参数**

- 'event'：Kafka 事件的句柄

#### 3.5.7 获取事件的所有分区 <!-- omit in toc -->

**语法**

```
kafka::eventGetParts(event)
```

**详情**

获取指定事件的所有分区。
返回一个table，由 STRING 类型的 topic，INT 类型的partition， INT 类型的 offset 三列组成。

**参数**

- 'event'：Kafka 事件的句柄



#### 3.5.8 判断是否为事件 <!-- omit in toc -->

语法：

```
kafka::eventBool(event)
```

**参数**

- 'event'：Kafka 事件的句柄

### 3.6 全局设置

#### 3.6.1 获取缓存区容量 <!-- omit in toc -->

**语法**

```
kafka::getBufferSize()
```

#### 3.6.2 设置缓存区容量 <!-- omit in toc -->

**语法**

```
kafka::setBufferSize(size)
```

**参数**

- 'size'：表示设置的缓存区的容量。不大于 server 的缓存区大小，默认为 900k。

#### 3.6.3 获取消息容量 <!-- omit in toc -->

**语法**

```
kafka::getMessageSize()
```

#### 3.6.4 设置消息容量 <!-- omit in toc -->

**语法**

```
kafka::setMessageSize(size)
```

**参数**

- 'size'：表示设置的消息容量。不能大于插件设定的数据发送缓存区大小，默认为 10k。

## 4 示例

``` shell
#create producer
producerCfg = dict(STRING, ANY);
producerCfg["metadata.broker.list"] = "localhost";
producer = kafka::producer(producerCfg);

#create consumer
consumerCfg = dict(string, any);
consumerCfg["metadata.broker.list"] = "localhost";
consumerCfg["group.id"] = "test";
consumer = kafka::consumer(consumerCfg);

#subscribe
topics=["test"];
kafka::subscribe(consumer, topics);
kafka::consumerPoll(consumer);

#produce and consume English string
kafka::produce(producer, "test", "1", "producer1:i'm producer",false,false);
kafka::consumerPoll(consumer);
#produce and consume Chinese string
    kafka::produce(producer, "test", "2", "I am a producer",false,false);
kafka::consumerPoll(consumer);
#produce and consume integer
kafka::produce(producer, "test", "3", 10086,false,false);
kafka::consumerPoll(consumer);
#produce and consume float
kafka::produce(producer, "test", "4", 123.456,false,false);
kafka::consumerPoll(consumer);
#produce and consume integer vector
message=[1,2,3,4];
kafka::produce(producer, "test", 1, message,false,false);
kafka::consumerPoll(consumer);
#produce and consume float vector
message=[1.1,2.2,3.3,4.4];
kafka::produce(producer, "test", 1, message,false,false);
kafka::consumerPoll(consumer);
#produce and consume Chinese string vector
message=["I","I am","I am a","I am a producer","I am a producer"];
kafka::produce(producer, "test", 1, message,false,false);
kafka::consumerPoll(consumer);
#produce and consume table
tab=table(1 2 3 as a, `x`y`z as b, 10.8 7.6 3.5 as c, "I" "I am" "I am a" as d);
kafka::produce(producer, "test", 1, tab,false,false);
kafka::consumerPoll(consumer);

#produce and consume two messages
kafka::produce(producer, "test", 1, "producer1:i'm producer",false,false);
kafka::produce(producer, "test", 1, "I am a producer",false,false);
kafka::consumerPollBatch(consumer,2);

#assign specific partition and offset
topics = ["test"];
partitions = [0];
offsets = [0];
kafka::unassign(consumer);
kafka::assign(consumer,topics,partitions,offsets);

#produce and consumer messages
kafka::produce(producer, "test", "1", "producer1:i'm producer",false,0);
kafka::produce(producer, "test", "2", "I am a producer",false,0);
kafka::produce(producer, "test", "3", 10086,false,0);
kafka::produce(producer, "test", "4", 123.456,false,0);
kafka::consumerPoll(consumer);
kafka::consumerPoll(consumer);
kafka::consumerPoll(consumer);
kafka::consumerPoll(consumer);

#Get the size of specific partitions
kafka::getOffsetCommitted(consumer,topics,partitions,offsets);
kafka::getAssignment(consumer);
kafka::getOffset(consumer,"test",2);
#Get the size of the current offset
kafka::getOffsetPosition(consumer,topics,partitions);

#deal with queue
queue=kafka::getConsumerQueue(consumer);
kafka::queueLength(queue);
kafka::queuePoll(queue);

#deal with event
event=kafka::queueEvent(queue);
kafka::getEventName(event);
kafka::eventGetMessage(event);
kafka::getEventMessageCount(event);
kafka::eventGetPart(event);
kafka::eventGetError(event);
kafka::eventBool(event);

#get a dictionary
kafka::produce(producer, "test", "1", "producer1:i'm producer",false,false,0);
kafka::produce(producer, "test", "2", "I am a producer",false,false,0);
kafka::produce(producer, "test", "3", 10086,false,false,0);
kafka::produce(producer, "test", "4", 123.456,false,false,0);
kafka::pollDict(consumer,4);

#get messages in json format
tab=table(1 2 3 as a, `x`y`z as b, 10.8 7.6 3.5 as c, "I" "I am" "I am a" as d);
dict={"1":1,"2":2,"3":3};
message=[1.1,2.2,3.3,4.4];
vec=[1,message,tab,];
kafka::produce(producer, "test", "1", tab,true,false,0);
kafka::consumerPoll(consumer);
kafka::produce(producer, "test", "1", dict,true,false,0);
kafka::consumerPoll(consumer);
kafka::produce(producer, "test", "1", message,true,false,0);
kafka::consumerPoll(consumer);
kafka::produce(producer, "test", "1", vec,true,false,0);
kafka::consumerPoll(consumer);

#change the buffer_size and message_size
kafka::getBufferSize();
kafka::getMessageSize();
kafka::setBufferSize(100);
kafka::setMessageSize(20);
a=[];
for(i in 0:120){a.append!(i%10)};
kafka::produce(producer,"test","1",a,false,false,0);
kafka::consumerPoll(consumer);
kafka::produce(producer,"test","1",tab,false,false,0);
kafka::consumerPoll(consumer);

#mult-thread
#the multithreading function need a parser, you can install mseed as an example

loadPlugin("/path/to/PluginKafka.txt");
loadPlugin("/path/to/PluginMseed.txt")

consumerCfg = dict(string, any);
consumerCfg["metadata.broker.list"] = "115.239.209.234";
consumerCfg["group.id"] = "test";
consumer = kafka::consumer(consumerCfg);

topics=["test"];
kafka::subscribe(consumer, topics);
tab = table(40000000:0,`id`time`value,[SYMBOL,TIMESTAMP,INT])

conn = kafka::createSubJob(consumer,tab,mseed::parse,"test:0:get mseed data");
kafka::getJobStat();
kafka::cancelSubJob(conn);
```
# ReleaseNotes:

## 故障修复

* 修复了接口 kafka::pollByteStream 不能接收非 JSON 格式数据的问题。（**1.30.22**）
* 修复了多线程操作导致的 server 宕机问题。（**1.30.22**）

# 功能优化

* 函数 eventGetParts , getOffsetPosition , getOffsetCommitted 增加了返回值。（**1.30.22**）
