# DolphinDB Pulsar 客户端插件使用说明

[Apache Pulsar](https://pulsar.apache.org/) 是用于处理服务器间消息传递的多租户高性能解决方案。为便于在 DolphinDB 中使用 Apache Pulsar，现提供 Pulsar 客户端插件。通过 DolphinDB Pulsar 客户端插件，用户可以创建与 Pulsar Broker 的连接，创建 Producer 对象向主题发布消息，以及创建 Consumer 对象订阅并接收主题的消息。

插件目前仅支持发布和订阅 `string` 类型的消息。其中 `createSubJob` 接口在创建订阅任务时，支持传入自定义的 parser 对接收的消息进行解析。

注意：该插件仅支持 2.00.x 系列的 DolphinDB。

## 安装

导入 bin 目录下预编译好的 Pulsar 插件，通过 DolphinDB 直接加载插件即可。

```bash
loadPlugin("/path/to/PluginPulsar.txt")
```

## 接口说明

### Client

#### client

`client(serviceUrl, [clientConfig])`

##### 参数

- serviceUrl: 要使用的 Pulsar 终端（例如：pulsar://localhost:6650），类型为 string。
- clientConfig: client 配置项，类型为 dict(string, any)，支持的选项包括：
  - operationTimeoutSeconds
  - connectionTimeoutMs

##### 详情

获取连接到指定集群地址的 Pulsar client 对象。

##### 返回值

Pulsar client 对象。

### Producer

#### producer

`producer(client, topic, [producerConfig])`

##### 参数

- client: 一个 Pulsar client 对象。
- topic: topic 名称，类型为 string。
- producerConfig: producer 配置项，类型为 dict(string, any)，支持的选项包括：
  - sendTimeoutMs
  - maxPendingMessages
  - maxPendingMessagesAcrossPartitions
  - blockIfQueueFull
  - batchingEnabled
  - batchingMaxMessages
  - batchingMaxPublishDelayMs

##### 详情

根据 topic 创建一个 Pulsar producer 对象。

##### 返回值

Pulsar producer 对象。

#### send

`send(producer, message, [partitionKey])`

##### 参数

- producer: 一个 Pulsar producer 对象。
- message: 要发送的消息，类型为 string。
- partitionKey: 消息的 partition key，类型为 string。

##### 详情

将 message 发布到 Pulsar。

##### 返回值

void

### Consumer

#### consumer

`consumer(client, topic, subscriptionName, [consumerConfig])`

[//]: # "TODO: 所有 config 的类型"

##### 参数

- client: 一个 Pulsar client 对象。
- topic: topic 名称，类型为 string。
- subscriptionName: subscription 名称，类型为 string。
- consumerConfig: consumer 配置项，类型为 dict(string, any)，支持的选项包括：
  - consumerType，类型为 int
    - `0`: ConsumerExclusive
    - `1`: ConsumerShared
    - `2`: ConsumerFailover
    - `3`: ConsumerKeyShared
  - receiverQueueSize
  - maxTotalReceiverQueueSizeAcrossPartitions
  - unAckedMessagesTimeoutMs
  - subscriptionInitialPosition，类型为 int
    - `0`: InitialPositionLatest
    - `1`: InitialPositionEarliest

##### 详情

创建一个 Pulsar consumer 对象，并根据 topic 进行订阅。

##### 返回值

Pulsar consumer 对象。

#### receive

`receive(consumer, [timeoutMs=1000])`

##### 参数

- consumer: 一个 Pulsar consumer 对象。
- timeoutMs: 接收超时，单位为毫秒，类型为 int，默认值为 1000。

##### 详情

接收一条 consumer 订阅的消息。

##### 返回值

接收到的消息，类型为 string。

### Subscription Job

#### createSubJob

`createSubJob(jobName, client, topic, subscriptionName, table, parser, [consumerConfig])`

##### 参数

- client: 一个 Pulsar client 对象。
- topic: topic 名称，类型为 string。
- subscriptionName: subscription 名称，类型为 string。
- table: 存储接收数据的表。
- parser: 处理接收数据的函数。
  - 返回值类型为 table，参数类型均为 string，参数数量可以为 1-3 个，第一个为 msg 的 data，第二个为 msg 的 partitionKey，第三个为 msg 的 topic。
- consumerConfig: consumer 配置项，类型为 dict(string, any)，支持的选项见 consumer 接口。

##### 详情

创建一个订阅任务，订阅 topic 并持续获取数据，数据通过 parser 处理存入 table 中。

##### 返回值

订阅任务对象的 handle。

#### cancelSubJob

`cancelSubJob(subJob)`

##### 参数

- subJob: 订阅任务对象的 handle，或者名称。若为名称，类型为 string。

##### 详情

取消订阅任务。

##### 返回值

void

#### getJobStat

`getJobStat()`

##### 详情

获取当前所有订阅任务的信息。

##### 返回值

包含当前所有订阅任务信息的 table。

### Configuration

#### getConfig

`getConfig(handle, configName)`

##### 参数

- handle: 一个 Pulsar client，producer，consumer，或订阅任务对象。
- configName: 要获取的 configuration 的名称，类型为 string。支持的选项见 client，producer，和 consumer 接口。

##### 详情

获取一个 configuration 的值。

##### 返回值

该 configuration 的值。

## 例子

```bash
loadPlugin("/path/to/PluginPulsar.txt");

## CLIENT

# define config dictionary
clientConfig = dict(["connectionTimeoutMs"], [2000])

client = pulsar::client("pulsar://localhost:6650", clientConfig)

## PRODUCER

producer = pulsar::producer(client, "persistent://public/default/my-topic")

pulsar::send(producer, "my message")

## CONSUMER

consumer = pulsar::consumer(client, "persistent://public/default/my-topic", "consumer-1")

pulsar::receive(consumer)

## SUBSCRIPTION JOB

# define parser
def parser(x) {
     return table([x] as msg)
}
# define table
demoTable = table(`a as msg)
share streamTable(1:0, demoTable.schema().colDefs.name, demoTable.schema().colDefs.typeString) as tb

subJob = pulsar::createSubJob("job-1", client, "persistent://public/default/my-topic", "subJob-1", tb, parser)

pulsar::cancelSubJob(subJob)

pulsar::getJobStat()

## CONFIGURATION

timeout = getConfig(client, "connectionTimeoutMs")
```
