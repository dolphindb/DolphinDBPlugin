# DolphinDB Kafka Plugin

With this plugin, you can easily publish or subscribe to Kafka streaming services. It supports serialization and deserialization of the following data types: 

* DolphinDB scalar types
* Built-in types of Kafka Java API: String(UTF-8), Short, Integer, Long, Float, Double, Bytes, byte[] and ByteBuffer
* Vector of types above

- [DolphinDB Kafka Plugin](#dolphindb-kafka-plugin)
    - [1. Pre-compiled installation](#1-pre-compiled-installation)
    - [2. Compiled installation](#2-compiled-installation)
        - [2.1 Prerequisites](#21-prerequisites)
        - [2.2 Build with `cmake`](#22-build-with-cmake)
        - [2.3 Move the result to bin/linux64](#23-move-the-result-to-binlinux64)
    - [3. API Details](#3-api-details)
        - [3.1 Load the Kafka Plugin](#31-load-the-kafka-plugin)
        - [3.2 Producer](#32-producer)
        - [3.3 Consumer](#33-consumer)
        - [3.4 Queue](#34-queue)
        - [3.5 Event](#35-event)
        - [3.6 Global Setting](#36-global-setting)
    - [4 Example](#4-example)

## 1. Pre-compiled installation

Dowload and unzip the folder to the root directory, and execute the following script on Linux:

``` shell
export LD_LIBRARY_PATH="LD_LIBRARY_PATH:/path/to/bin/linux"
```
Start DolphinDB server on Linux. Run the following script in DolphinDB to load the plugin:

``` shell
loadPlugin("/path/to/PluginKafka.txt")
```

## 2. Compiled installation

### 2.1 Prerequisites

Install CMake. For Ubuntu users (repalce `apt` with `yum` if you use Centos):

``` shell
sudo apt install cmake
```

The project depends on 'cppkafka', which depends on 'boost' and 'librdkafka'. Download with the following script:

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

cd /path/to/the/main/project/
git submodule update --init --recursive
```

If it is too slow to download submodule, you can download it with the cppkafka.git from the hidden file .gitModules.

``` shell
git clone https://github.com/mfontanini/cppkafka.git
```
copy the libDolphinDB.so to bin/linux64 or /lib

``` shell
cp /path/to/dolphindb/server/libDolphinDB.so /path/to/kafka/bin/linux64
```

### 2.2 Build with `cmake`

Build the project:

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
# copy the libDolphinDB.so to ./build
cp /path/to/dolphindb/server/libDolphinDB.so ./build
cd build
cmake ..
make
```

### 2.3 Move the result to bin/linux64

copy the .so and .txt to bin/linux64

``` shell
cp /path/to/libPluginKafka.so /path/to/kafka/bin/linux64
cp /path/to/PluginKafka.txt /path/to/kafka/bin/linux64
```

## 3. API Details

Before loading the Kafka plugin, please download it and start Zookeeper and Kafka server. Please refer to [this link](https://kafka.apachecn.org/quickstart.html) for details.

### 3.1 Load the Kafka Plugin

Run the following script in DolphinDB to load the plugin (the directory need to be replaced with the path of luginKafka.txt):

``` shell
loadPlugin("/path/to/PluginKafka.txt")
```

### 3.2 Producer

#### 3.2.1 Initialization

##### Syntax

```shell
kafka::producer(config);
```

##### Arguments

'config' is a dictionary indicating the Kafka producer configuration, whose key is a string and value is a string or a boolean. Please refer to [CONFIGURATION](https://github.com/edenhill/librdkafka/blob/master/CONFIGURATION.md) for more about Kafka configuration.

##### Details

Create a Kafka producer with specified configurations, and return the handler.

#### 3.2.2 Produce Message

##### Syntax

```shell
kafka::produce(producer, topic, key, value, json, [partition] );
```

##### Arguments

- 'producer' is a Kafka producer handler.
- 'topic' is a string indicating the Kafka topic.
- 'key' indicates a Kafka key.
- 'value' indicates a Kafka value.
- 'json' indicates whether to transfer the data in json format or not.
- 'partition' is an optioanl parameter. It is an integer indicating the Kafka broker partition number.

##### Details

Produce key-value data in a specified partition.

Note:

Please don't send too much messages at once, otherwise an exception `Local: Queue full` might be thrown.

#### 3.2.3 Producer flushing

##### Syntax

``` shell
kafka::producerFlush(producer);
```

##### Arguments

- 'producer' is a Kafka producer handler.

##### Details

Flush all the messages of the producer.

#### 3.2.4 Get blocking time

##### Syntax

``` shell
kafka::getProducerTime(producer)
```

##### Arguments

- 'producer' is a Kafka producer handler.

#### 3.2.5 Set blocking time

##### Syntax

``` shell
kafka::setProducerTime(producer, timeout)
```

##### Arguments

- 'producer' is a Kafka producer handler.
- 'timeout' is the maximum amount of time you will wait for the response of a request.

### 3.3 Consumer

#### 3.3.1 Initialization

##### Syntax

```
kafka::consumer(config)
```

##### Arguments

- 'config' is a dictionary indicating the Kafka consumer configuration, whose key is a string and value is an anyVector. Please refer to [CONFIGURATION](https://github.com/edenhill/librdkafka/blob/master/CONFIGURATION.md) for more about Kafka configuration.

The following example is for users in SASL protocol:

```shell
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

##### Details

Create a Kafka consumer and return the handler.

#### 3.3.2 Subscribe

##### Syntax

```
kafka::subscribe(consumer, topics)
```

##### Arguments

- 'consumer' is a Kafka consumer handler.
- 'topics' is a STRING vector indicating the topics to subscribe to.

##### Details

Subscribe a Kafka topic.

#### 3.3.3 Unsubscribe

##### Syntax

```
kafka::unsubscribe(consumer)
```

##### Arguments

- 'consumer' is a Kafka consumer handler.

##### Details

Unsubscribe all topics.

#### 3.3.4 Poll a message

##### Syntax

```
kafka::consumerPoll(consumer, [timeout])
```

##### Arguments

- 'consumer' is a Kafka consumer handler.
- 'timeout' is the maximum amount of time to wait for a polling.

##### Details

Save the subscribed data to DolphinDB, and return a tuple. The first element is a string indicating the error message. The second element is a tuple including the following elements: topic, partition, key, value and the timestamp when the consumer received the data.

`kafka::consumerPoll` will block the current thread and the poll default timeout is 1000 millisecond. 

It is recommended to use function `consumerPollBatch` to submit multiple `kafka::consumerPoll` tasks.

#### 3.3.5 Poll messages

##### Syntax

``` shell
kafka::consumerPollBatch(consumer, batch_size, [time_out])
```

##### Arguments

- 'consumer' is a Kafka consumer handler.
- 'batch_size' is the number of messages you want to get.
- 'timeout' indicates the maximum amount of time to get messages.

#### 3.3.6 Poll messages by multi-thread

##### Syntax

```shell
kafka::createSubJob(consumer, table, parser, description, [timeout])
```

##### Arguments

- 'consumer' is a Kafka consumer handler.
- 'table' is a table to store the messages.
- 'parser' is a function to deal with the input data, and it returns a table. You can use mseed::parser or the user-defined function.
- 'description' is a string to describe the thread.
- 'timeout' indicates the maximum amount of time to get each message.

##### Details

The parser need a string for input and return a table, and you can use mseed::parser for example or you can define a function by yourself.

Note:

If a task you created subscribes to a partition that has been subscribed by another task, messages wil be split and distributed to all tasks that subscribe to this partition. Therefore, the subscribed messages obtained from these tasks may be incomplete.

#### 3.3.7 Get the muti-thread status

##### Syntax

```shell
kafka::getJobStat()
```

##### Arguments

- None.

#### 3.3.8 End the polling thread

##### Syntax

```shell
kafka::cancelSubJob(connection)
```

##### Arguments

- 'connection' is the result of the `kafka::createSubJob`, or the subscription id you get from function `getJobStat()`, which can be LONG, INT, and STRING data type.

#### 3.3.9 Poll messages and return a dictionary

##### Syntax

```shell
kafka::pollDict(consumer, batch_size, [timeout])
```

##### Arguments

- 'consumer' is a Kafka consumer handler.
- 'batch_size' is the number of messages you want to get.
- 'timeout' indicates the maximum amount of time to get messages.

##### Details

Save the subscribed data to DolphinDB. It returns a DolphinDB dictionary containing messages in the form of key-value pair.

#### 3.3.10 Commit

##### Syntax

```
kafka::commit(consumer)
```

##### Arguments

- 'consumer' is a Kafka consumer handler.

##### Details

Commit the offset of the last processed message to producer synchronously. If there is no lastest offset, an exception will be thrown.

#### 3.3.11 Topic Commit

##### Syntax

``` shell
kafka::commitTopic(consumer, topics, partitions, offsets)
```

##### Arguments

- 'consumer' is a Kafka consumer handler.
- 'topics' is a STRING vector indicating the topics to subscribe to.
- 'partitions' is an INT vector indicating the partition corresponding to each topic.
- 'offsets' is an INT vector indicating the offset corresponding to each topic.

#### 3.3.12 AsyncCommit

##### Syntax

```
kafka::asyncCommit(consumer)
```

##### Arguments

- 'consumer' is a Kafka consumer handler.

##### Details

Commit the offset of the last processed message to producer asynchronously to _consumer_offset, a topic that is used to store messages.

#### 3.3.13 AsyncCommitTopic

##### Syntax

```
kafka::asyncCommitTopic(consumer, topics, partitions, offsets)
```

##### Arguments

- 'consumer' is a Kafka consumer handler.
- 'topics' is a STRING vector indicating the topics to subscribe to.
- 'partitions' is an INT vector indicating the partition corresponding to each topic.
- 'offsets' is an INT vector indicating the offset corresponding to each topic.

#### 3.3.14 Get blocking time

##### Syntax

``` shell
kafka::getConsumerTime(consumer)
```

##### Arguments

- 'consumer' is a Kafka consumer handler.

#### 3.3.15 Set blocking time

##### Syntax

``` shell
kafka::setConsumerTime(consumer, timeout)
```

##### Arguments

- 'consumer' is a Kafka consumer handler.
- 'timeout' is the maximum amount of time to get messages.

#### 3.3.16 Assign topic

##### Syntax

``` shell
kafka::assign(consumer, topics, partitions, offsets)
```

##### Arguments

- 'consumer' is a Kafka consumer handler.
- 'topics' is a STRING vector indicating the topics to subscribe to.
- 'partitions' is an INT vector indicating the partition corresponding to each topic.
- 'offsets' is an INT vector indicating the offset corresponding to each topic.

##### Details

Unlike `kafka::subscribe(consumer, topics)`, this function enables you to assign specific topics, partitions and offsets to the consumer.

#### 3.3.17 Unassign topic

##### Syntax

``` shell
kafka::unassign(consumer)
```

##### Arguments

- 'consumer' is a Kafka consumer handler.

##### Detalis

Unassign all topics of the consumer.

#### 3.3.18 Get the assignment of the consumer

##### Syntax

``` shell
kafka::getAssignment(consumer)
```

##### Arguments

- 'consumer' is a Kafka consumer handler.

#### 3.3.19 Get offset

##### Syntax

``` shell
kafka::getOffset(consumer, topic, partition)
```

##### Arguments

- 'consumer' is a Kafka consumer handler.
- 'topic' is a STRING vector indicating the topics to subscribe to.
- 'partition' is an INT vector indicating the partition corresponding to each topic.

##### Details

Print the offsets of the consumer.

#### 3.3.20 Get Offset Committed

##### Syntax

``` shell
kafka::getOffsetCommitted(consumer, topics, partitions, offsets, [timeout])
```

##### Arguments

- 'consumer' is a Kafka consumer handler.
- 'topics' is a STRING vector indicating the topics to subscribe to.
- 'partitions' is an INT vector indicating the partition corresponding to each topic.
- 'offsets' is an INT vector indicating the offset corresponding to each topic.
- 'timeout' is the maximum amount of time to get messages.

##### Details

Get the offsets committed for the given topic/partition list.

#### 3.3.21 Get Offset Position

##### Syntax

``` shell
kafka::getOffsetPosition(consumer, topics, partitions)
```

##### Arguments

- 'consumer' is a Kafka consumer handler.
- 'topics' is a STRING vector indicating the topics to subscribe to.
- 'partitions' is an INT vector indicating the partition corresponding to each topic.

##### Details

Get the offset positions for the given topic/partition list.

#### 3.3.22 Store Consumed Offsets

##### Syntax

``` shell
kafka::storeConsumedOffset(consumer)
```

##### Arguments

- 'consumer' is a Kafka consumer handler.

##### Details

Store the offsets on the currently assigned topic/partitions (legacy).

Please set enable.auto.offset.store=false, enable.auto.commit=true for consumer, otherwise an error will be reported.

#### 3.3.23 Store Offsets

##### Syntax

``` shell
storeOffset(consumer, topics, partitions, offsets)
```

##### Arguments

- 'consumer' is a Kafka consumer handler.
- 'topics' is a STRING vector indicating the topics to subscribe to.
- 'partitions' is an INT vector indicating the partition corresponding to each topic.
- 'offsets' is an INT vector indicating the offset corresponding to each topic.

##### Details

Store the offsets on the given topic/partitions (legacy).

Please set enable.auto.offset.store=false, enable.auto.commit=true for consumer, otherwise an error will be reported.

#### 3.3.24 Get Member ID

##### Syntax

``` shell
kafka::getMemId(consumer)
```

##### Arguments

- 'consumer' is a Kafka consumer handler.

##### Details

Get the group member ID.

### 3.4 Queue

#### 3.4.1 Get Main Queue

##### Syntax

```shell 
kafka::getMainQueue(consumer)
```

##### Arguments

- 'consumer' is a Kafka consumer handler.

##### Details

Get the global event queue corresponding to the consumer.

#### 3.4.2 Get Consumer Queue

##### Syntax

```shell 
kafka::getConsumerQueue(consumer)
```

##### Arguments

- 'consumer' is a Kafka consumer handler.

##### Details

Get the consumer group queue.

#### 3.4.3 Get Partition Queue

##### Syntax

```shell 
kafka::getPartitionQueue(consumer, topic, partition)
```

##### Arguments

- 'consumer' is a Kafka consumer handler.
- 'topic' is a STRING vector indicating the topics to subscribe to.
- 'partition' is an INT vector indicating the partition corresponding to each topic.

##### Details

Get the queue of this partition. If the consumer is not assigned to this partition, an empty queue will be returned.

#### 3.4.4 Get queue length

##### Syntax

```shell 
kafka::queueLength(queue)
```

##### Arguments

- 'queue' is a Kafka queue handler.

##### Details

Returns the length of the queue

Note:

It is not recommended to use this function if it is deployed on the ARM architecture servers. The result may not be as expected.

#### 3.4.5 Forward to queue

##### Syntax

```shell 
kafka::forToQueue(queue, forward_queue)
```

##### Arguments

- 'queue' is a Kafka queue handler.
- 'forward_queue' is a Kafka queue handler, indicating the target that the queue forward to.

#### 3.4.6 Stop forwarding to queue

##### Syntax

```shell 
kafka::disforToQueue(queue)
```

##### Arguments

- 'queue' is a Kafka queue handler.

##### Details

Stop forwarding to another queue.

#### 3.4.7 Set queue timeout

##### Syntax

```shell 
kafka::setQueueTime(queue, timeout)
```

##### Arguments

- 'queue' is a Kafka queue handler.
- 'timeout' is the maximum amount of time for the queue.

#### 3.4.8 Get queue timeout

##### Syntax

```shell 
kafka::getQueueTime(queue)
```

##### Arguments

- 'queue' is a Kafka queue handler.

##### Details

Get the configured timeout.

#### 3.4.9 Poll message of queue

##### Syntax

```shell 
kafka::queuePoll(queue, [timeout])
```

##### Arguments

- 'queue' is a Kafka queue handler.
- 'timeout' is the maximum amount of time for the queue.

#### 3.4.10 Poll messages of queue

##### Syntax

```shell 
kafka::queuePollBatch(queue, batch_size, [timeout])
```

##### Arguments

- 'queue' is a Kafka queue handler.
- 'batch_size' is the number of messages you want to get.
- 'timeout' is the maximum amount of time for the queue.

### 3.5 Event

#### 3.5.1 Get event from a queue

##### Syntax

``` shell
kafka::queueEvent(queue)
```

##### Arguments

- 'queue' is a Kafka queue handler.

##### Details

Extract the next event in this queue.

Note:

Before deleting a consumer, please ensure that the events generated by it is still in its lifecycle (specify event=NULL to release resources), otherwise the program may collapse.

#### 3.5.2 Get event name

##### Syntax

``` shell
kafka::getEventName(event)
```

##### Arguments

- 'event' is a Kafka event handler.

##### Details

Return the name of the event.

#### 3.5.3 Get messages from the event

##### Syntax

``` shell
kafka::eventGetMessage(event)
```

##### Arguments

- 'event' is a Kafka event handler.

##### Details

Get all messages in this event (if any).

#### 3.5.4 Get the count of messages from the event

##### Syntax

``` shell
kafka::getEventMessageCount(event)
```

##### Arguments

- 'event' is a Kafka event handler.

#### 3.5.5 Get errors of the event

##### Syntax

``` shell
kafka::eventGetError(event)
```

##### Arguments

- 'event' is a Kafka event handler.

##### Details

Return error messages in this event.

#### 3.5.6 Get the partition of the event

##### Syntax

``` shell
kafka::eventGetPart(event)
```

##### Arguments

- 'event' is a Kafka event handler.

#### 3.5.7 Get partitions of the event

##### Syntax

``` shell
kafka::eventGetParts(event)
```

##### Arguments

- 'event' is a Kafka event handler.

#### 3.5.8 Determine whether it is an event or not

##### Syntax

``` shell
kafka::eventBool(event)
```

##### Arguments

- 'event' is a Kafka event handler.

### 3.6 Global Setting

#### 3.6.1 Get Buffer Size

##### Syntax

``` shell
kafka::getBufferSize()
```

#### 3.6.2 Set Buffer Size

##### Syntax

``` shell
kafka::setBufferSize(size)
```

##### Arguments

- 'size' is the capacity of the buffer size you want to set, which is no larger than the buffer size of the broker, and the default value is 900k.

#### 3.6.3 Get Message Size

##### Syntax

``` shell
kafka::getMessageSize()
```

#### 3.6.4 Set Message Size

##### Syntax

``` shell
kafka::setMessageSize(size)
```

##### Arguments

- 'size' is the capacity of the message size you want to set. The `message_size` is no larger than the `buffer_size`, and the default value is 10k.

## 4 Example

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
