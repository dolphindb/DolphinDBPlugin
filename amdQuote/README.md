# DolphinDB amdQuote Plugin(Linux)

您正在查看的教程位于 release200 分支，需要使用 2.00.X 版本 server。

DolphinDB amdQuote 插件目前仅支持 Linux 系统。本文仅介绍 Linux 系统上如何安装及使用该插件。

## 1. 安装构建

### 1.1. 预编译安装

预先编译的插件文件存放在[bin/linux](https://github.com/dolphindb/DolphinDBPlugin/tree/release200/amdQuote/bin/linux) 目录。将该目录下对应 Amd 版本文件夹内的所有文件（包括动态库文件）下载至 DolphinDB server 所在机器的如下目录：/DolphinDB/server/plugins/amdQuote。

启动 DolphinDB，加载插件：
``` bash
cd DolphinDB/server                                   //进入 DolphinDB server 目录
./dolphindb                                           //启动 DolphinDB server
loadPlugin("/your_plugin_path/PluginAmdQuote.txt")    //加载插件
```

### 1.2. 自行编译构建

编译 3.9.8，4.0.1，4.2.0 版本的插件
``` bash
mkdir build
cd build
cp <path_to_dolphindb_server>/libDolphinDB.so ./
cmake .. -DAMDAPIDIR=<amd_ami_dir>
make -j
make install
```

编译支持 amd 3.9.6 版本的插件，需要在 cmake 时指定 AMD_VERSION 为 3.9.6
``` bash
mkdir build
cd build
cp <path_to_dolphindb_server>/libDolphinDB.so ./
cmake .. -DAMDAPIDIR=<amd_ami_dir> -DAMD_VERSION=3.9.6
make -j
make install
```

编译后在文件 libPluginAmdQuote.so, PluginAmdQuote.txt 和所依赖的一系列动态库会被放在 bin/linux 目录下。

## 2. 接口说明

### 2.1. **amdQuote::connect(username, password, ips, ports, options)**

**参数**

`username` 为字符串标量，AMD 行情服务器的用户名。

`password` 为字符串标量，AMD 行情服务器的密码。

`ips` 为字符串向量, AMD 行情服务器 IP 列表，需要和端口列表数量相同。

`ports` 为整型向量，AMD 行情服务器端口列表，需要和 IP 列表数量相同。

`options` 可选参数。是字典类型，表示扩展参数。当前支持 `ReceivedTime`, `DailyIndex`, `StartTime`, `OutputElapsed`。
其中：

* ReceivedTime 表示是否获取插件收到行情数据的时间戳。其指定为 dict(["ReceivedTime"], [true]) 时，getSchema 获取的表结构中将包含插件收到行情数据的时间戳列。
* DailyIndex 表示是否添加每天按 channel_no 递增的数据列，仅对 order 和 execution 类型的行情数据有效，其他类型该列均为空值。其指定为 dict(["DailyIndex"], [true]) 时，getSchema 获取的表结构中将包含插件收到行情数据的按 channel_no 递增的列。如果订阅的时间超过了 StartTime（默认为 `8:40:00.0000`），则置为空值。具体 StartTime 的含义见 StartTime 条目。
* StartTime 表示每天计算的 dailyIndex 起始的时间, 默认为 `8:40:00.0000` 。其指定为 dict(["StartTime"], [true]) 时，如果执行 amdQuote::subscribe 的服务器时间小于 StartTime，则当日的 dailyIndex 都为空值，进入第二天则从头计数，不置为空值。
* OutputElapsed 表示是否获取 amdQuote 插件 接收数据处理的时延。其指定为 dict(["OutputElapsed"], [true]) 时，getSchema 获取的表结构中将包含插件收到行情数据的时延列

  时延的定义：'amd 回调函数返回数据' 到 '准备开始 transform' 处理，或准备 append 到共享流表前’ 这段时间。

如果需要启用合并类型 'bondExecution'，'orderExecution'，'fundOrderExecution'，则 ReceivedTime 和 DailyIndex 必须指定为 true。

**函数详情**

创建一个和 AMD 行情服务器之间的连接，返回一个句柄。

注意：如果当前已有一个 amdQuote 连接并且尚未调用 amdQuote::close 进行关闭，如果再次以完全相同的参数进行调用，则不会重新进行连接，而会直接返回已有连接的 handle。如果参数指定不同，则会报错，无法新建。此时如果需要获取已有的 AMD 行情连接句柄需要调用 amdQuote::getHandle() 函数。

### 2.2. **amdQuote::subscribe(handle, type, outputTable, marketType, codeList, transform)**

**参数**

`handle` connect 接口返回的句柄。

`type` 字符串标量，表示行情的类型，可取以下值：'snapshot'（股票快照）, 'execution'（股票逐笔成交）, 'order'（股票逐笔委托）, 'index'（指数）, 'orderQueue'（委托队列）, 'fundSnapshot'（基金快照）, 'fundExecution'（基金逐笔成交），'fundOrder'（基金逐笔委托），'bondSnapshot'（债券快照），'bondOrder'（债券逐笔委托），'bondExecution'（债券逐笔成交），'orderExecution'（股票逐笔委托、逐笔成交合并），'fundOrderExecution'（基金逐笔委托、逐笔成交合并），'bondOrderExecution'（债券逐笔委托、逐笔成交合并），'option' (期权)，'IOPV' （ ETF 基金份额参考净值）。

注意，最后三种 type 获取的行情数据中。第一列 `SecurityID 会在末尾加上 '.SZ' 或者 '.SH'

`outputTable` 如果 type 类型**不是**合并类型 'bondExecution'，'orderExecution'，'fundOrderExecution'，则表示一个 共享流表 或者 IPC 表 对象， 需要在订阅前创建。该表的 schema 需要和获取的行情数据结构一致。可以通过插件提供的 getSchema 函数来获取行情数据的 schema。

如果 type 类型是合并类型 'bondExecution'，'orderExecution'，'fundOrderExecution'。则该参数需要传入一个字典。字典的 key 为整型标量，指代特定的 channel，需要大于 0。字典的 value 为 共享流表 或 IPC 表。流表的 schema 需要和获取的行情数据结构一致。

`marketType` 整型标量。表示市场类型。需要和 AMD 中定义的市场类型一致。**amdQuote 插件不支持订阅全部市场，因此必须填写具体的市场代码如 101。**
注意，如果 type 类型是合并类型 'bondExecution'，'orderExecution'，'fundOrderExecution'，则一次只能订阅一种市场，重新订阅后将不会收到之前订阅的市场的数据。

`codeList` 字符串向量，可选。表示股票列表。不传该参数表示订阅所有股票。

`transform`: 一元函数（其参数是一个表）。插入到 DolphinDB 表前对表进行转换，例如替换列。请注意，传入的一元函数中不能存在对 DFS 表的操作，例如：读取或写入 DFS 表，获取 DFS 表的 schema 等。

**函数详情**

订阅指定市场、行情数据类型和股票列表的行情数据到 目标表。

注意，如果未取消订阅就重新订阅某种类型的行情，该种类前一次订阅的内容会被取消，即只有最后一次订阅生效。

### 2.3. **amdQuote::unsubscribe(handle, type, marketType, codeList)**

**参数**

`handle` connect 接口返回的句柄。

`dataType` 字符串标量，表示行情的类型，可取以下值：'snapshot', 'execution', 'order', 'index', 'orderQueue', 'fundSnapshot', 'fundExecution', 'fundOrder', 'bondSnapshot', 'bondOrder', 'bondExecution'，'orderExecution'，'fundOrderExecution'，'bondOrderExecution'，'option'，'IOPV' 和 'all'。其中，'all' 表示取消所有订阅。

`marketType` 整型标量，表示市场类型，需要和 AMD 中定义的市场类型一致。

`codeList` 字符串向量，表示股票列表。

**函数详情**

取消对行情数据的订阅。
* 如果 *dataType* 指定为 'all'，表示取消所有订阅，此时无需指定 *marketType* 和 *codeList*。
* 如果 *dataType* 指定非 'all' 的值：
  * 只指定 *marketType*，表示取消 *marketType* 下的所有订阅。
  * 同时指定 *marketType* 和 *codeList*，表示只取消对 *codeList* 的订阅。

### 2.4. **amdQuote::close(handle)**

**参数**

`handle` connect 接口返回的句柄。

**函数详情**

关闭当前连接。通过 connect 创建连接时，AMD API 会创建线程在内的一些资源。当用户确定不使用行情数据之后需要手动调用 close 接口释放资源。

注意 AMD 提供的 SDK 在连接时会创建一些全局变量，这些部分在 dolphindb 插件内无法析构，因此用户需要尽量避免频繁连接，以免内存占用过高导致 dolphindb 不可用。

### 2.5. **amdQuote::getSchema(type)**

**参数**

`type` 字符串标量，表示行情的类型，可取以下值：'snapshot', 'execution', 'order', 'index', 'orderQueue', 'fundSnapshot', 'fundExecution', 'fundOrder', 'bondSnapshot', 'bondOrder'，'option'，'IOPV' 和 'bondExecution'，'orderExecution'，'fundOrderExecution'，'bondOrderExecution'。

**函数详情**

该函数应该在 connect 函数之后调用。获取行情数据的表结构。返回一个表，包含三列：name，type 和 typeInt，分别表示该行情表中 字段的名字，字段类型的名称和类型的枚举值。通过该表来创建具有相同结构的共享流表。

如果在 amdQuote::connect 中指定了 options，在返回的 schema 中会增加对应的列。如果指定了`ReceivedTime`为 true，则会增加一个 NANOTIMESTAMP 类型列 `receivedTime`；如果指定了`DailyIndex`为 true，则会增加一个 LONG 类型列 `dailyIndex`；如果指定了`OutputElapsed`为 true，则会增加一个 LONG 类型列 `perPenetrationTime`

### 2.6. **amdQuote::getStatus(handle)**

**参数**

`handle` connect 接口返回的句柄。

**函数详情**

返回一个表格，包含各种已订阅数据的状态信息，不包含未订阅过的数据类型

| 列名                    | 含义                       | 类型          |
| ----------------------- | -------------------------- | ------------- |
| **topicName**           | 订阅的名称                 | STRING        |
| **startTime**           | 订阅开始的时间             | NANOTIMESTAMP |
| **endTime**             | 订阅结束的时间             | NANOTIMESTAMP |
| **firstMsgTime**        | 第一条消息收到的时间       | NANOTIMESTAMP |
| **lastMsgTime**         | 最后一条消息收到的时间     | NANOTIMESTAMP |
| **processedMsgCount**   | 已经处理的消息数           | LONG          |
| **lastErrMsg**          | 最后一条错误信息           | STRING        |
| **failedMsgCount**      | 处理失败的消息数           | LONG          |
| **lastFailedTimestamp** | 最后一条错误消息发生的时间 | NANOTIMESTAMP |

### 2.7. **amdQuote::getHandle()**

**参数**

无

**函数详情**

获取当前已有的 AMD 连接句柄。如果尚未连接则抛出异常。

### 2.8. **amdQuote::getCodeList()**

**参数**

无

**函数详情**

获取当前连接下的代码表结构。

Amd sdk 版本为 3.9.6 的插件不支持该函数

### 2.9. **amdQuote::getETFCodeList()**

**参数**

无

**函数详情**

获取当前连接下的 ETF 代码表结构。

Amd sdk 版本为 3.9.6 的插件不支持该函数

### 2.10. **amdQuote::setLogError(flag)**

**参数**

`flag` 布尔类型，表示是否在 log 中输出所有 error。

**函数详情**

开启或关闭在 log 中输出所有 error 的功能。

如果 flag 设置为 false，则同一 error 只会在第一次出现时输出，以后将不会输出。如果 flag 设置为 true，则每次遇到 error 时，就会在 log 中进行输出 (由于 amd 的重连机制，开启可能导致 log 中出现大量 error 信息)

## 3. 使用示例

1.使用 loadPlugin 加载插件
```dolphindb
loadPlugin("Your_plugin_path/PluginAmdQuote.txt")
```
2.连接 AMD 服务器
```dolphindb
handle = amdQuote::connect(`admin, `123456, [`119.29.65.231], [8031], dict(["ReceivedTime"], [true]))
```
3.获取对应的表结构
```dolphindb
snapshotSchema = getSchema(`snapshot);

executionSchema = getSchema(`execution);

orderSchema = getSchema(`order);
```

4.创建流表
```dolphindb
snapshotTable = streamTable(10000:0, snapshotSchema[`name], snapshotSchema[`type]);

executionTable = streamTable(10000:0, executionSchema[`name], executionSchema[`type]);

orderTable = streamTable(10000:0, orderSchema[`name], orderSchema[`type]);
```
5.共享并持久化流表
```dolphindb
enableTableShareAndPersistence(table=snapshotTable, tableName=`snapshot1, cacheSize=10000)

enableTableShareAndPersistence(table=executionTable, tableName=`execution1, cacheSize=10000)

enableTableShareAndPersistence(table=orderTable, tableName=`order1, cacheSize=10000)
```
6.订阅深圳市场全部股票代码

因为 AMD API 文档中深圳市场的枚举值为 102，所以 `subscribe` 的 *marketType* 参数指定为 102.

对应的订阅快照，逐笔成交和逐笔委托的示例为：
```dolphindb
amdQuote::subscribe(handle, `snapshot, snapshot1, 102)

amdQuote::subscribe(handle, `execution, execution1, 102)

amdQuote::subscribe(handle, `order, order1, 102)
```
7.取消深圳市场的快照订阅
```dolphindb
amdQuote::unsubscribe(handle， `snapshot, 102)
```
取消逐笔成交，逐笔委托，取消全部数据的订阅分别是：
```dolphindb
amdQuote::unsubscribe(handle, `execution, 102)

amdQuote::unsubscribe(handle, `order, 102)

amdQuote::unsubscribe(handle, `all, 102)
```
8.使用完成后，手动调用接口释放资源

```dolphindb
amdQuote::close(handle)
```

# Release Notes

## 2.00.11

### 新功能

- 持接收 ETF 期权与 IOPV 数据（支持华锐 SDK 3.9.8 及以后版本）
- 新增 getHandle 接口，获取已连接的句柄。
- 接口 getStatus 新增返回内容 processedMsgCount，显示已经处理的消息数。
- 新增支持华锐 SDK 4.3.0 版本。

### 故障修复

- 规避了低内存情况下，连接华锐行情源可能导致 crash 的问题。
- 修复了接入异步持久化流表会导致 crash 的问题。

## 2.00.10

### 新增功能

- 新增支持接收委托表（order）和成交表（trade ）按交易所原始频道代码（ChannelNo）多线程异步写入 server 中的目标表。同一个原始频道代码的委托表和成交表将写入同一张表，且保证其写入顺序。
- 接口 amdQuote::connect 新增支持参数 outputElapsed，用于统计插件内部的时延。
- 新增支持订阅输出至非流数据表。

### 故障修复

- 修复在订阅一个错误市场后，无法使用 amdQuote::unsubscribe 取消该订阅的问题。
- 修复了在长时间订阅后，行情数据偶现无法正确写入流表的问题。
