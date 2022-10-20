# ZeroMQ Plugin for DolphinDB

- [ZeroMQ Plugin for DolphinDB](#zeromq-plugin-for-dolphindb)
  - [1. Install the Plugin](#1-install-the-plugin)
    - [1.1 Download Precompiled Binaries](#11-download-precompiled-binaries)
    - [1.2 Build a Plugin](#12-build-a-plugin)
  - [2. Send](#2-send)
    - [2.1 zmq::socket](#21-zmqsocket)
    - [2.2 zmq::connect](#22-zmqconnect)
    - [2.3 zmq::bind](#23-zmqbind)
    - [2.4 zmq::send](#24-zmqsend)
    - [2.5 zmq::close](#25-zmqclose)
  - [3. Subscribe](#3-subscribe)
    - [3.1 zmq::createSubJob](#31-zmqcreatesubjob)
    - [3.2 zmq::getSubJobStat](#32-zmqgetsubjobstat)
    - [3.3 zmq::cancelSubJob](#33-zmqcancelsubjob)
    - [3.4 zmq::zmqCreatepusher](#34-zmqzmqcreatepusher)
  - [4. Formatter/Parser](#4-formatterparser)
    - [4.1 createCSVFormatter](#41-createcsvformatter)
    - [4.2 createCSVParser](#42-createcsvparser)
    - [4.3 createJSONFormatter](#43-createjsonformatter)
    - [4.4 createJSONParser](#44-createjsonparser)
  - [5. Example](#5-example)

## 1. Install the Plugin

### 1.1 Download Precompiled Binaries

**Linux**

The precompiled binaries are stored in the directory DolphinDBPlugin/zmq/bin/linux64. You can execute the following command to load the plugin in DolphinDB:

```
cd DolphinDB/server //Change to the directory of DolphinDB server
./dolphindb //Start DolphinDB server
loadPlugin("<PluginDir>/zmq/bin/linux64/PluginZmq.txt") //Load the plugin
```

### 1.2 Build a Plugin

You can also manually compile a zmq plugin following the instructions:

(1) compile libzmq

Download [libzmq-4.3.4](https://github.com/zeromq/libzmq/releases/tag/v4.3.4)

```bash
cd libzmq-4.3.4
cp include/zmq.h /path/to/PluginZmq/bin/include/
mkdir build && cd build
cmake ..
make -j8
cp lib/libzmq.a /path/to/PluginZmq/bin/linux64/
```

(2) obtain the header file of cppzmq

Download [cppzmq-4.7.1](https://github.com/zeromq/cppzmq/releases/tag/v4.7.1)

```bash
cd cppzmq-4.7.1
cp zmq.hpp /path/to/PluginZmq/bin/include/
```

(3) build a plugin

```
mkdir build
cd build
cmake  ../
make
```

**Note:** Please make sure the file *libDolphinDB.so* is under the GCC search path before compilation. You can add the plugin path to the library search path `LD_LIBRARY_PATH` or copy it to the build directory.

*libPluginZmq.so* and *PluginZmq.txt* are generated under the working directory.

## 2. Send 

### 2.1 zmq::socket

**Syntax**

zmq::socket(type, formatter, [batchSize], [prefix])

**Parameters**

- type: is a STRING indicating the socket type to be created. It can be “ZMQ_PUB” and “ZMQ_PUSH”.
- formatter: is a function used to package published data in a format. Currently supported functions are `createJsonFormatter` and `createCsvFormatter`.
- batchSize: is an integer. When the content to be published is a table, it can be sent in batches, and *batchSize* indicates the number of rows sent each time.
- prefix: a STRING indicating the message prefix.

**Details**

Create a zmq socket.

Note: When using methods `connect`, `bind`, `send` and `close` for concurrent operations, different zmq sockets must be constructed for different threads.

**Example**

```
handle = streamTable(10:0, [`int], [INT])
enableTableShareAndPersistence(table=handle, tableName=`test1, asynWrite=true, compress=true, cacheSize=10000000, retentionMinutes=120)
parser = zmq::createJSONParser([INT], [`bool])
zmq::createSubJob("tcp://localhost:55633", "ZMQ_SUB", true, handle, parser, "prefix1")
```

### 2.2 zmq::connect

**Syntax**

zmq::connect(socket, addr, [prefix])

**Parameters**

- socket: a zmq socket
- addr: the address string in the form of "protocol://interface:port", indicating the remote address to be connected to. "protocol" is the underlying transport protocol to use, including tcp, ipc, inproc, and epgm. "interface:port" is the remote IP address and port number.
- prefix: a STRING indicating the message prefix.

**Details**

Use socket to establish connections to zmq. Keepalive is enabled after the tcp connection is set so that it can be automatically connected.

**Example**

```
formatter = zmq::createJSONFormatter()
socket = zmq::socket("ZMQ_PUB", formatter)
zmq::connect(socket, "tcp://localhost:55632", "prefix1")
```

### 2.3 zmq::bind

**Syntax**

zmq::bind(socket, addr, [prefix])

**Parameters**

- socket: a zmq socket
- addr: the address string in the form of "protocol://interface:port", indicating the remote address to be connected to. "protocol" is the underlying transport protocol to use, including tcp, ipc, inproc, and epgm. "interface:port" is the remote IP address and port number.
- prefix: a STRING indicating the message prefix.

**Details**

Bind a socket to a specific address to accept incoming requests.

**Example**

```
formatter = zmq::createJSONFormatter()
socket = zmq::socket("ZMQ_PUB", formatter)
zmq::bind(socket, "tcp://*:55631", "prefix1")
```

### 2.4 zmq::send

**Syntax**

zmq::send(socket, data, [prefix])

**Parameters**

- socket: is a zmq socket
- data: is a table to be sent
- prefix: a STRING indicating the message prefix.

**Details**

Send a zmq message. Return true if successful.

**Example**

```
formatter = zmq::createJSONFormatter()
socket = zmq::socket("ZMQ_PUB", formatter)
zmq::connect(socket, "tcp://localhost:55632", "prefix1")
zmq::send(socket, table(1..10 as id))
```

### 2.5 zmq::close

**Syntax**

zmq::close(socket)

**Details**

Close a zmq socket.

**Example**

```
formatter = zmq::createJSONFormatter()
socket = zmq::socket("ZMQ_PUB", formatter)
zmq::connect(socket, "tcp://localhost:55632", "prefix1")
zmq::close(socket)
```

## 3. Subscribe

### 3.1 zmq::createSubJob

**Syntax**

zmq::createSubJob(addr, type, isConnnect, handle, parser, [prefix])

**Details**

Create a zmq subscription. The subscription will automatically reconnect after network failures.

**Parameters**

- addr: the address string in the form of "protocol://interface:port", indicating the remote address to be connected to. "protocol" is the underlying transport protocol to use, including tcp, ipc, inproc, and epgm. "interface:port" is the remote IP address and port number.
- type: a STRING indicating the socket type to be created. It can be “ZMQ_SUB” and “ZMQ_PULL”.
- isConnnect: a Boolean value indicating whether to connect to addr. If false the addr is binded.
- handle: a function or a table used to handle messages sent from zmq
- parser: is a function for parsing subscribed messages. Currently supported functions are `createJsonParser` and `createCsvParser`.
- prefix: a STRING indicating the message prefix.

**Example**

```
handle = streamTable(10:0, [`int], [INT])
enableTableShareAndPersistence(table=handle, tableName=`test1, asynWrite=true, compress=true, cacheSize=10000000, retentionMinutes=120)
parser = zmq::createJSONParser([INT], [`bool])
zmq::createSubJob("tcp://localhost:55633", "ZMQ_SUB", true, handle, parser, "prefix1")
```

You can use it with a Python script:

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

**Syntax**

zmq::getSubJobStat()

**Details**

Get all zmq subscription messages. 

Return a table with the following columns:

- subscriptionId: the subscription ID.
- addr: the subscription address.
- prefix: the message prefix.
- recvPackets: the number of messages received.
- createTimestamp: the timestamp when the subscription is created.

**Example**

```
handle = streamTable(10:0, [`int], [INT])
enableTableShareAndPersistence(table=handle, tableName=`test1, asynWrite=true, compress=true, cacheSize=10000000, retentionMinutes=120)
parser = zmq::createJSONParser([INT], [`bool])
zmq::createSubJob("tcp://localhost:55633", "ZMQ_SUB", handle, parser, "prefix1")
zmq::getSubJobStat()
```

### 3.3 zmq::cancelSubJob

**Syntax**

zmq::cancelSubJob(subscription)

**Parameters**

- subscription: is the value returned by `createSubJob`, or the subscriptionId returned by `getJobStat`.

**Details**

Cancel a zmq subscription.

**Example**

```
zmq::cancelSubJob(sub1)
zmq::cancelSubJob(42070480)
```

### 3.4 zmq::zmqCreatepusher

**Syntax**

zmq::zmqCreatepusher(socket, dummyTable)

**Details**

Create a zmq pusher.

**Parameters**

- socket: is a zmq socket.
- dummyTable: schema of the input table.

**Example**

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

## 4. Formatter/Parser

### 4.1 createCSVFormatter

**Syntax**

zmq::createCSVFormatter([format], [delimiter=','], [rowDelimiter=';'])

**Parameters**

- format: is a vector of STRING type.
- delimiter: is the separator between columns, the default is ','.
- rowDelimiter: is the separator between rows, the default is ';'.

**Details**

This function creates a Formatter function in CSV format.

**Example**

```
MyFormat = take("", 5)
MyFormat[2] = "0.000"
f = createCSVFormatter(MyFormat, ',', ';')
```

### 4.2 createCSVParser

**Syntax**

zmq::createCSVParser(schema, [delimiter=','], [rowDelimiter=';'])

**Parameters**

- schema: a vector indicating the data type of each column.
- delimiter: is the separator between columns, the default is ','.
- rowDelimiter: is the separator between rows, the default is ';'.

**Details**

This function creates a Parser function in CSV format.

**Example**

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

**Syntax**

zmq::createJSONFormatter()

**Parameters**

None

**Details**

This function creates a Formatter function in JSON format

**Example**

```
def createT(n) {
    return table(take([false, true], n) as bool, take('a'..'z', n) as char, take(short(-5..5), n) as short, take(-5..5, n) as int, take(-5..5, n) as long, take(2001.01.01..2010.01.01, n) as date, take(2001.01M..2010.01M, n) as month, take(time(now()), n) as time, take(minute(now()), n) as minute, take(second(now()), n) as second, take(datetime(now()), n) as datetime, take(now(), n) as timestamp, take(nanotime(now()), n) as nanotime, take(nanotimestamp(now()), n) as nanotimestamp, take(3.1415, n) as float, take(3.1415, n) as double, take(`AAPL`IBM, n) as string, take(`AAPL`IBM, n) as symbol)
}
t = createT(100)
f = zmq::createJSONFormatter()
f(t)
```

### 4.4 createJSONParser

**Syntax**

zmq::createJSONParser(schema, colNames)

**Parameters**

- schema: is a vector indicating the data type of each column.
- colNames: is a vector indicating the name of each column.

**Details**

This function creates a Parser function in JSON format.

**Example**

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

## 5. Example

```
loadPlugin("/home/zmx/worker/DolphinDBPlugin/zmq/cmake-build-debug/PluginZmq.txt")
go
formatter = zmq::createJSONFormatter()
socket = zmq::socket("ZMQ_PUB", formatter)
zmq::bind(socket, "tcp://localhost:55632")
data = table(1..10 as id, take(now(), 10) as ts, rand(10, 10) as volume)
zmq::send(socket, data)
```

You can use it with a Python script:

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

 