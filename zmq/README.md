# DolphinDB zmq Plugin

ZeroMQ（zmq）是一个可伸缩的分布式或并发应用程序设计的高性能异步消息库。它提供了一个消息队列库，但与面向消息的中间件不同，zmq 可以在没有专门的消息代理的情况下运行；该库名字中的 Zero 意为零代理。详情可参考：[ZeroMQ](https://zeromq.org/)。

通过 DolphinDB 的 zmq 插件，用户可以创建 zmq socket，完成 zmq 消息通信的常见操作，包含通过请求应答机制的会话建立、发布、订阅以及消息的管道传输。

zmq 插件目前支持版本：[relsease200](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/zmq/README.md), [release130](https://github.com/dolphindb/DolphinDBPlugin/blob/release130/zmq/README.md), [release120](https://github.com/dolphindb/DolphinDBPlugin/blob/release120/zmq/README.md)。您当前查看的插件版本为release200，请使用 DolphinDB 2.00.X 版本 server。若使用其它版本 server，请切换至相应插件分支。

## 1. 安装构建

### 1.1 预编译安装

#### Linux

预先编译的插件文件存放在 `DolphinDBPlugin/zmq/bin/linux64` 目录下。通过 DolphinDB，执行以下命令可导入 zmq 插件：

```
cd DolphinDB/server //进入DolphinDB server目录
./dolphindb //启动 DolphinDB server
loadPlugin("<PluginDir>/zmq/bin/linux64/PluginZmq.txt") //加载插件
```

### 1.2 自行编译

#### 编译 libzmq

下载 [libzmq-4.3.4](https://github.com/zeromq/libzmq/releases/tag/v4.3.4)

```bash
cd libzmq-4.3.4
cp include/zmq.h /path/to/PluginZmq/bin/include/
mkdir build && cd build
cmake ..
make -j8
cp lib/libzmq.a /path/to/PluginZmq/bin/linux64/
```

#### 获取 cppzmq 头文件

下载 [cppzmq-4.7.1](https://github.com/zeromq/cppzmq/releases/tag/v4.7.1)

```bash
cd cppzmq-4.7.1
cp zmq.hpp /path/to/PluginZmq/bin/include/
```

#### 构建插件

构建插件内容

```
mkdir build
cd build
cmake  ../
make
```

> **注意**：编译之前请确保 libDolphinDB.so 在 gcc 可搜索的路径中。可使用 `LD_LIBRARY_PATH` 指定其路径，或者直接将其拷贝到 build 目录下。

编译后目录下会产生两个文件：libPluginZmq.so 和 PluginZmq.txt。

## 2. 发送

### 2.1 zmq::socket

**语法**

zmq::socket(type, formatter, [batchSize], [prefix])

**参数**

* type：STRING 类型，表示要创建的 socket 类型，取值为 "ZMQ_PUB" 和 "ZMQ_PUSH"。
* formatter：一个函数，用于指定发布的数据的打包格式，目前 zmq 插件自带 CSV 或 JSON 两种格式，分别由`createJSONFormatter`或`createCSVFormatter`创建。也可以自行创建自定义的格式函数，输入参数是后续 zmq::send 的参数 *data*。
* batchSize：INT 类型，表示每次发送的记录行数。当待发布内容是一个表时，可以进行分批发送。
* prefix：STRING 类型，表示发送前缀。

**详情**

创建一个 zmq socket。

> **注意**：对 connect, bind, send, close 接口进行并发操作时，需要为各线程创建不同的 zmq socket 连接句柄。

**例子**

```
formatter = zmq::createJSONFormatter()
socket = zmq::socket("ZMQ_PUB", formatter)
```

### 2.2 zmq::connect

**语法**

zmq::connect(socket, addr, [prefix])

**参数**

* socket：zmq 连接句柄。
* addr：STRING 类型，表示 socket 连接到的远端地址，格式为 "transport://address:port"。*transport* 表示要使用的底层协议，取值为 tcp, ipc, inproc, pgm 或 epgm。*address:port* 表示远端的 IP 地址和端口号。
* prefix：STRING 类型，表示发送前缀。

**详情**

进行 zmq 的 socket 连接，将 socket 连接到远端节点上，然后开始接受该端点上的传入连接。tcp 建立连接后会保活，使得网络断开又恢复后可以自动重连。

**例子**

```
formatter = zmq::createJSONFormatter()
socket = zmq::socket("ZMQ_PUB", formatter)
zmq::connect(socket, "tcp://localhost:55632", "prefix1")
```

### 2.3 zmq::bind

**语法**

zmq::bind(socket, addr, [prefix])

**参数**

* socket：zmq 连接句柄。
* addr：STRING 型，表示 socket 绑定的地址，格式为 "transport://address:port"。*transport* 表示要使用的底层协议，取值为 tcp, ipc, inproc, pgm 或 epgm。*address:port* 表示进行绑定的地址和端口号，* 表示同一个服务器的所有 IP。
* prefix：STRING 类型，表示发送前缀。
  
**详情**

绑定 socket，接收发来的链接请求。

**例子**

```
formatter = zmq::createJSONFormatter()
socket = zmq::socket("ZMQ_PUB", formatter)
zmq::bind(socket, "tcp://*:55631", "prefix1")
```

### 2.4 zmq::send

**语法**

zmq::send(socket, data, [prefix])

**参数**

* socket：zmq 连接句柄。
* data：发送的数据，类型应为 zmq::socket 创建时 formatter 的传入参数的数据类型，不然会在调用 *formatter* 时格式化失败而抛出异常。
* prefix：STRING 类型，表示消息前缀。

**详情**

发送一条 zmq 消息。如果发送成功，则返回 true。

**例子**

```
formatter = zmq::createJSONFormatter()
socket = zmq::socket("ZMQ_PUB", formatter)
zmq::connect(socket, "tcp://localhost:55632", "prefix1")
zmq::send(socket, table(1..10 as id))
```

### 2.5 zmq::close

**语法**

zmq::close(socket)

**参数**

* socket：zmq 连接句柄。
  
**详情**
关闭一个 zmq 连接句柄。

**例子**

```
formatter = zmq::createJSONFormatter()
socket = zmq::socket("ZMQ_PUB", formatter)
zmq::connect(socket, "tcp://localhost:55632", "prefix1")
zmq::close(socket)
```

## 3. 订阅

### 3.1 zmq::createSubJob

**语法**

zmq::createSubJob(addr, type, isConnnect, handle, parser, [prefix])

**参数**

* addr：STRING 类型，格式为 "transport://address:port"。*transport* 表示要使用的底层协议，取值为 tcp, ipc, inproc, pgm 或 epgm。*address:port* 表示 zmq 绑定的地址和端口。
* type：STRING 类型，表示 zmq 的 socket 类型，取值为 "ZMQ_SUB" 和 "ZMQ_PULL"。
* isConnnect：BOOL 类型，表示是否是对 *addr* 进行连接，如果为否，则对 *addr* 进行绑定。
* handle：一个函数或表，用于处理从 zmq 接收的消息。
* parser：一个函数，用于对发布的数据进行解包。目前 zmq 插件提供2种 `createJSONParser` 或 `createCSVParser` 解包方式。输出参数是一个 table，输入参数是一个 string 的 scalar。
* prefix：STRING 类型，表示消息前缀。

**详情**

创建一个 zmq 订阅，且满足网络断线重连后，订阅也自动重连。

**例子**

```
handle = streamTable(10:0, [`int], [INT])
enableTableShareAndPersistence(table=handle, tableName=`test1, asynWrite=true, compress=true, cacheSize=10000000, retentionMinutes=120)
parser = zmq::createJSONParser([INT], [`bool])
zmq::createSubJob("tcp://localhost:55633", "ZMQ_SUB", true, handle, parser, "prefix1")
```

#### 与之搭配的 python 脚本

```
import zmq
import time
import sys

context = zmq.Context()
socket = context.socket(zmq.PUB)
socket.bind("tcp://*:55633")
msg = '[{"bool":234}]'

while True:
	socket.send(msg.encode('utf-8'))
	time.sleep(2)
```

### 3.2 zmq::getSubJobStat

**语法**

zmq::getSubJobStat()

**详情**

查询所有 zmq 订阅信息。

查询所有订阅信息。返回一个表，包含如下字段：

* subscriptionId：表示订阅标识符。
* addr：zmq 订阅地址。
* prefix：zmq 订阅前缀。
* recvPackets：zmq 订阅收到的消息报文数。
* createTimestamp：表示订阅建立时间。

**例子**

```
handle = streamTable(10:0, [`int], [INT])
enableTableShareAndPersistence(table=handle, tableName=`test1, asynWrite=true, compress=true, cacheSize=10000000, retentionMinutes=120)
parser = zmq::createJSONParser([INT], [`bool])
zmq::createSubJob("tcp://localhost:55633", "ZMQ_SUB", handle, parser, "prefix1")
zmq::getSubJobStat()
```

### 3.3 zmq::cancelSubJob

**语法**

zmq::cancelSubJob(subscription)

**参数**

* subscription：是 `createSubJob` 函数返回的值或 `getJobStat` 返回的订阅标识符。

**详情**

关闭一个 zmq 订阅。

**例子**

```
zmq::cancelSubJob(sub1)
zmq::cancelSubJob(42070480)
```

### 3.4 zmq::createPusher

**语法**

zmq::createPusher(socket, dummyTable)

**参数**

* socket：zmq 连接句柄。
* dummyTable：一个表对象，用于接收注入的数据。

**详情**

创建一个 zmq 的 pusher，注入该 pusher 的数据将被推送出去。支持两种用法：

* 通过 append! 方法追加数据至 pusher。
* 流数据引擎输出表（outputTable）数据注入 pusher。

**例子**

```
share streamTable(1000:0, `time`sym`volume, [TIMESTAMP, SYMBOL, INT]) as trades
output1 = table(10000:0, `time`sym`sumVolume, [TIMESTAMP, SYMBOL, INT])

formatter = zmq::createJSONFormatter()
socket = zmq::socket("ZMQ_PUB", formatter)
zmq::connect(socket, "tcp://localhost:55632")
pusher = zmq::createPusher(socket, output1)

engine1 = createTimeSeriesEngine(name="engine1", windowSize=60000, step=60000, metrics=<[sum(volume)]>, dummyTable=trades, outputTable=pusher, timeColumn=`time, useSystemTime=false, keyColumn=`sym, garbageSize=50, useWindowStartTime=false)
subscribeTable(tableName="trades", actionName="engine1", offset=0, handler=append!{engine1}, msgAsTable=true);

insert into trades values(2018.10.08T01:01:01.785,`A,10)
insert into trades values(2018.10.08T01:01:02.125,`B,26)
insert into trades values(2018.10.08T01:01:10.263,`B,14)
insert into trades values(2018.10.08T01:01:12.457,`A,28)
insert into trades values(2018.10.08T01:02:10.789,`A,15)
insert into trades values(2018.10.08T01:02:12.005,`B,9)
insert into trades values(2018.10.08T01:02:30.021,`A,10)
insert into trades values(2018.10.08T01:04:02.236,`A,29)
insert into trades values(2018.10.08T01:04:04.412,`B,32)
insert into trades values(2018.10.08T01:04:05.152,`B,23)
```

## 4. 打/解包功能

### 4.1 createCSVFormatter

**语法**
zmq::createCSVFormatter([format], [delimiter=','], [rowDelimiter=';'])

**参数**

* format：STRING 类型的向量。
* delimiter：列之间的分隔符，默认是','。
* rowDelimiter：行之间的分隔符，默认是';'。

**详情**

创建一个 CSV 格式的 Formatter 函数。

**例子**

```
MyFormat = take("", 5)
MyFormat[2] = "0.000"
f = createCSVFormatter(MyFormat, ',', ';')
```

### 4.2 createCSVParser

**语法**
zmq::createCSVParser(schema, [delimiter=','], [rowDelimiter=';'])

**参数**

* schema：一个包含各列数据类型的向量。
* delimiter：列之间的分隔符，默认是','。
* rowDelimiter：行之间的分隔符，默认是';'。

**详情**

创建一个 CSV 格式的 Parser 函数。

**例子**

```
def createT(n) {
    return table(take([false, true], n) as bool, take('a'..'z', n) as char, take(short(-5..5), n) as short, take(-5..5, n) as int, take(-5..5, n) as long, take(2001.01.01..2010.01.01, n) as date, take(2001.01M..2010.01M, n) as month, take(time(now()), n) as time, take(minute(now()), n) as minute, take(second(now()), n) as second, take(datetime(now()), n) as datetime, take(now(), n) as timestamp, take(nanotime(now()), n) as nanotime, take(nanotimestamp(now()), n) as nanotimestamp, take(3.1415, n) as float, take(3.1415, n) as double, take(`AAPL`IBM, n) as string, take(`AAPL`IBM, n) as symbol)
}
t = createT(100)
f = zmq::createCSVFormatter([BOOL,CHAR,SHORT,INT,LONG,DATE,MONTH,TIME,MINUTE,SECOND,DATETIME,TIMESTAMP,NANOTIME,NANOTIMESTAMP,FLOAT,DOUBLE,STRING,SYMBOL])
s=f(t)
p = zmq::createCSVParser([BOOL,CHAR,SHORT,INT,LONG,DATE,MONTH,TIME,MINUTE,SECOND,DATETIME,TIMESTAMP,NANOTIME,NANOTIMESTAMP,FLOAT,DOUBLE,STRING,SYMBOL])
p(s)
```

### 4.3 createJSONFormatter

**语法**
zmq::createJSONFormatter()

**详情**

创建一个 JSON 格式的 Formatter 函数。

**例子**

```
def createT(n) {
    return table(take([false, true], n) as bool, take('a'..'z', n) as char, take(short(-5..5), n) as short, take(-5..5, n) as int, take(-5..5, n) as long, take(2001.01.01..2010.01.01, n) as date, take(2001.01M..2010.01M, n) as month, take(time(now()), n) as time, take(minute(now()), n) as minute, take(second(now()), n) as second, take(datetime(now()), n) as datetime, take(now(), n) as timestamp, take(nanotime(now()), n) as nanotime, take(nanotimestamp(now()), n) as nanotimestamp, take(3.1415, n) as float, take(3.1415, n) as double, take(`AAPL`IBM, n) as string, take(`AAPL`IBM, n) as symbol)
}
t = createT(100)
f = zmq::createJSONFormatter()
f(t)
```

### 4.4 createJSONParser

**语法**
zmq::createJSONParser(schema, colNames)

**参数**

* schema：一个向量，表示各列的数据类型。
* colNames：一个向量，表示列名。

**详情**

创建一个 JSON 格式的 Parser 函数。

**例子**

```
def createT(n) {
    return table(take([false, true], n) as bool, take('a'..'z', n) as char, take(short(-5..5), n) as short, take(-5..5, n) as int, take(-5..5, n) as long, take(2001.01.01..2010.01.01, n) as date, take(2001.01M..2010.01M, n) as month, take(time(now()), n) as time, take(minute(now()), n) as minute, take(second(now()), n) as second, take(datetime(now()), n) as datetime, take(now(), n) as timestamp, take(nanotime(now()), n) as nanotime, take(nanotimestamp(now()), n) as nanotimestamp, take(3.1415, n) as float, take(3.1415, n) as double, take(`AAPL`IBM, n) as string, take(`AAPL`IBM, n) as symbol)
}
t = createT(100)
f = zmq::createJSONFormatter()
p = createJSONParser([BOOL,CHAR,SHORT,INT,LONG,DATE,MONTH,TIME,MINUTE,SECOND,DATETIME,TIMESTAMP,NANOTIME,NANOTIMESTAMP,FLOAT,DOUBLE,STRING,SYMBOL],
`bool`char`short`int`long`date`month`time`minute`second`datetime`timestamp`nanotime`nanotimestamp`float`double`string`symbol)
s=f(t)
x=p(s)

```

## 5. 完整例子

```
loadPlugin("/home/zmx/worker/DolphinDBPlugin/zmq/cmake-build-debug/PluginZmq.txt")
go
formatter = zmq::createJSONFormatter()
socket = zmq::socket("ZMQ_PUB", formatter)
zmq::bind(socket, "tcp://localhost:55632")
data = table(1..10 as id, take(now(), 10) as ts, rand(10, 10) as volume)
zmq::send(socket, data)
```

### 5.1 与之搭配的 python 脚本

```
import zmq
from zmq.sugar import socket
import json
if __name__=='__main__':
    context = zmq.Context()
    socket = context.socket(zmq.SUB)
    
    socket.setsockopt(zmq.TCP_KEEPALIVE, 1);
    socket.setsockopt(zmq.TCP_KEEPALIVE_IDLE, 30);
    socket.setsockopt(zmq.TCP_KEEPALIVE_INTVL, 1);
    socket.setsockopt(zmq.TCP_KEEPALIVE_CNT, 5);
    
    socket.connect("tcp://192.168.0.48:55632")
    zip_filter = ""
    socket.setsockopt(zmq.SUBSCRIBE, zip_filter.encode('ascii'))

    while True:
        recvStr = socket.recv()
        print (recvStr)
```

# ReleaseNotes:

## v2.00.10

### bug修复

- 增加及调整非法参数的报错
