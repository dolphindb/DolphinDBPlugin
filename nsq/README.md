# DolphinDB NSQ Plugin

为对接恒生 NSQ 极速行情服务软件，DolphinDB 开发了 NSQ 插件。通过该插件能够获取上海和深圳市场的行情。主要获得以下三种行情：

1. 主推-现货深度行情主推回调（OnRtnSecuDepthMarketData->snapshot）
2. 主推-现货逐笔成交行情主推回调（OnRtnSecuTransactionTradeData->trade）
3. 主推-现货逐笔委托行情主推回调（OnRtnSecuTransactionEntrustData->orders）

恒生公司发布了 NSQ 极速行情服务软件的 SDK，名称为 HSNsqApi。其对应 linux 下的 libHSNsqApi.so 或 windows 下的 HSNsqApi.dll。编译时需要将对应动态库拷贝至插件项目的 lib/[linux.x/win][32/64]（如 lib/linux.x64) 文件夹。在运行时需要保证对应链接库能被找到。

请注意，DolphinDB 仅提供对接 HSNsqApi 的 NSQ 插件。数据源和接入服务可咨询数据服务商或证券公司。

本文包含以下内容：
- [DolphinDB NSQ Plugin](#dolphindb-nsq-plugin)
	- [支持 NSQ 的 server 版本](#支持-nsq-的-server-版本)
	- [构建](#构建)
		- [Linux](#linux)
		- [windows](#windows)
	- [插件加载](#插件加载)
	- [API](#api)
		- [nsq::connect(configFilePath, options)](#nsqconnectconfigfilepath-options)
		- [nsq::getSchema(dataType)](#nsqgetschemadatatype)
		- [nsq::subscribe(type, location, streamTable)](#nsqsubscribetype-location-streamtable)
		- [nsq::unsubscribe(type, location)](#nsqunsubscribetype-location)
		- [nsq::close()](#nsqclose)
		- [nsq::getSubscriptionStatus()](#nsqgetsubscriptionstatus)
	- [示例](#示例)
	- [报错信息](#报错信息)

## 支持 NSQ 的 server 版本

目前，仅 DolphinDB_Linux64_V2.00.6, DolphinDB_Linux64_V1.30.18, DolphinDB_Win64_V1.30.18_JIT, DolphinDB_Win64_V2.00.6_JIT 及以上版 server 支持 NSQ 插件。

其中，若使用 Linux 系统，在使用 NSQ 插件前，需指定环境变量：

```
export LD_LIBRARY_PATH= /your_plugin_path:$LD_LIBRARY_PATH 
```

## 构建

### Linux

使用 cmake 编译构建

首先，在 nsq 插件文件夹下创建一个 build 文件夹，作为编译工作区。因为编译插件时需要链接 libDolphinDB.so （libDolphinDB.so 是运行 dolphindb 所依赖的库，非插件特有）。编译开始之前，需要将 dolphindb server 同级或上级目录下的 libDolphinDB.so  拷贝至 build 文件夹。

注意：在 `nsq/lib` 下，有针对不同操作系统的 NSQ SDK 库，分别存放于不同的文件夹，需要根据当前构建系统的类型拷贝其中一个动态库至 lib 目录（默认是 linux64）。在插件运行的时候也需要加载此 so 库，因此还需要拷贝到能识别该 so 文件的路径下 (例如：dolphindb server 下与 libDolphinDB.so 同级的目录)。

```
mkdir build
cd build
cmake ..
make
```

### windows

windows 系统下，需要将 nsq/lib/win32 或者 nsq/lib/win64 中的 HSNsqApi.dll 拷贝至 nsq/lib/ 目录。

**编译准备**

在 Windows 环境中需要使用 cmake 和 MinGW 进行编译，通过以下链接下载：

- 下载安装 [MinGW](http://www.mingw-w64.org/)。确保将 bin 目录添加到系统环境变量 Path 中。
- 下载安装 [cmake](https://cmake.org/)。

**使用 cmake 构建**

首先，在 nsq 插件文件夹下创建一个 build 文件夹，作为编译工作区。因为编译插件时需要链接 libDolphinDB.so （libDolphinDB.so 是运行 dolphindb 所依赖的库，非插件特有）。编译开始之前，需要将 dolphindb server 同级或上级目录下的 libDolphinDB.dll 拷贝至 build 文件夹。

构建插件内容：

```
cd <PluginDir>\nsq
mkdir build                                             # 新建 build 目录
COPY <ServerDir>/libDolphinDB.dll build                 # 拷贝 libDolphinDB.dll 到 build 目录下
cd build
cmake  ../ -G "MinGW Makefiles"
mingw32-make -j
```

## 插件加载

配置文件为 PluginNsq.txt，位于和 libPluginNsq.so 和 libPluginNsq.dll 同级的 build 目录。

编译生成 libPluginNsq.so 之后，通过以下脚本加载插件：

```
loadPlugin("/path/to/PluginNsq.txt");
```

## API

### nsq::connect(configFilePath, options)

**参数**

`configFilePath` 一个字符串，表示 `sdk_config.ini` 的绝对路径；若拷贝 `sdk_config.ini` 至 dolphindb server，则可以是相对于 dolphindb server 的一个相对路径。

`options` 可选参数。是字典类型，表示扩展参数。当前键只支持 receivedTime，表示是否显示接收时间，对应值为布尔值。详见后文示例。

**函数详情**

该函数将根据 NSQ 配置文件 `sdk_config.ini` 的配置，和行情服务器进行连接。连接成功后在日志文件 dolphindb.log 中会打印 “OnFrontConnected”。

请注意，再次执行 connect 进行重新连接前，需要先执行 nsq::close() 断开连接，否则会抛出异常。

### nsq::getSchema(dataType)

**参数**

`dataType` 一个字符串，表示所要获取的表结构的类型，包含 snapshot, trade, orders。

**函数详情**

该函数需要在 connect 函数之后调用。后续根据 getSchema 返回的表结构创建流表。

### nsq::subscribe(type, location, streamTable)

**参数**

`type` 一个字符串，表示行情的类型，包含以下值：
* "snapshot"：表示回调函数 OnRtnSecuDepthMarketData（主推 - 现货深度行情）获取的行情数据。
* "trade"：表示回调函数 OnRtnSecuTransactionTradeData（主推 - 现货逐笔成交行情主）获取的行情数据。
* "orders"：表示回调函数 OnRtnSecuTransactionEntrustData（主推 - 现货逐笔委托行情）获取的行情数据。

`location`: 一个字符串，表示上海证券交易所或深圳证券交易所。上海证券交易所用 `sh` 表示，深圳证券交易所用 `sz` 表示。

`streamTable`: 表示一个共享流表的表对象。订阅前需要创建一个流表，且该流表的 schema 需要和获取的行情数据结构一致。请注意，建议设置为一个持久化后的流表对象（参见 [enableTableShareAndPersistence](https://www.dolphindb.cn/cn/help/FunctionsandCommands/CommandsReferences/e/enableTableShareAndPersistence.html) 或 [enableTablePersistence](https://www.dolphindb.cn/cn/help/FunctionsandCommands/CommandsReferences/e/enableTablePersistence.html)）。否则，可能会发生 OOM。

**函数详情**

表示对上海证券交易所或深圳证券交易所发布的某种行情数据进行订阅，并将结果保存到由参数 `streamTable` 指定的流表中。

订阅成功后，在日志（dolphindb.log) 中会有打印如下信息(若出现 successfully，表示订阅成功)：

```
OnRspSecuTransactionSubscribe: nRequestID[0], ErrorID[0], ErrorMsg[subscribe all transaction trans type[1] of exchange_id [1] successfully]---------------------
```

请注意，若需要将已经订阅的同一个(`type`, `location`) 的行情数据输出到另一个 `streamTable`，需要通过 `unscribeTable` 命令取消订阅，否则会抛出异常。

streamTable（流表）是一种特殊的内存表，用于存储及发布流数据。更多流表的使用方法可参考文档：https://www.dolphindb.cn/cn/help/130/DatabaseandDistributedComputing/Streaming.html

### nsq::unsubscribe(type, location)

**参数**

unsubscribe 命令的两个参数 `type` 和 `location` 的说明同 subscribe 的一致。

**函数详情**

表示取消对上海证券交易所或深圳证券交易所发布的某种行情数据的订阅，例如：unsubscribe(\`snapshot, \`sz) 表示取消对深圳证券交易所的 snapshot 行情数据的订阅。

取消订阅成功后，在日志（dolphindb.log) 中会有打印如下信息(若出现 successfully，表示取消订阅成功)：

```
OnRspSecuTransactionCancel: nRequestID[0], ErrorID[0], ErrorMsg[unsubscribe all transaction trans type [2] of exchange_id [2] successfully]---------------------
```

### nsq::close()

**参数**

无

**函数详情**

表示断开当前连接。如果修改了配置文件，则需要执行 close 后，再执行 connect，从而建立新的连接。

### nsq::getSubscriptionStatus()

**参数**

无

**函数详情**

`getSubscriptionStatus` 是一个运维命令，用于获取当前连接状态，以及每个订阅的状态。

该函数会返回一个表，通过 select 语句来查看获取的状态，用法如下：

```
status = nsq::getSubscriptionStatus();
select * from status;
```

例如当前状态可能如下:

```
topicType     isConnected isSubscribed processedMsgCount lastErrMsg failedMsgCount lastFailedTimesconnecttamp
-------------- ----------- ------------ ----------------- ---------- -------------- -------------------
(snapshot, sh) true        true         0                            0
(snapshot, sz) true        true         0                            0
(trade, sh)    true        true         0                            0
(trade, sz)    true        true         0                            0
(orders, sh)    true        true         0                            0
(orders, sz)    true        true         0                            0
```

## 示例

在插件根目录中的 `nsq_script.txt` 文件展示了运用 nsq 插件的一个完整的实例。下面展示了部分脚本：

```
// 登录
login("admin", "123456")
// 加载插件
loadPlugin("Your_plugin_path/PluginNsq.txt");
// 连接行情服务器，第二个参数为可选
nsq::connect(your_config_path，dict(["ReceivedTime"], [true]);
// 获取行情数据的表结构
snapshotSchema = nsq::getSchema(`snapshot);
tradeSchema = nsq::getSchema(`trade);
// 根据表结构创建流表
streamTable(1000:0, snapshotSchema[`name], snapshotSchema[`type]) as t1;
streamTable(1000:0, tradeSchema[`name], tradeSchema[`type]) as t2;

go
// 流表持久化
enableTableShareAndPersistence(table=t1, tableName=`snapshot_sh, cacheSize=100000)
enableTableShareAndPersistence(table=t2, tableName=`trade_sh, cacheSize=100000)

// 订阅上海证券交易所的深度行情
nsq::subscribe(`snapshot, `sh, snapshot_sh);
// 取消订阅
nsq::unsubscribe(`snapshot, `sh)
// 订阅上海证券交易所的逐笔成交行情
nsq::subscribe(`trade`, `sh`, trade_sh);
// 用这个表对象进行操作
select * from snapshot_sh limit 100;
// 取消订阅
nsq::unsubscribe(`trade`, `sh`)

// 获取每个订阅的状态
status = nsq::getSubscriptionStatus();
select * from status;

// 关闭连接
nsq::close();
```

注意：schema 中的字段类型和顺序需要和 sdk 文档中的字段类型和顺序严格一致。

## 报错信息

插件正常运行的信息会打印在日志文件中（dolphindb.log），若运行中出现错误，则会抛出异常。具体异常信息及解决办法如下：

1. 重复连接异常。若当前已连接，则需要先通过 close 关闭连接，再 connect 重连。

  You are already connected. To reconnect, please execute close() and try again.

2. API 初始化错误，需要确认 connect 传入的配置文件路径和配置信息是否正确。

  Initialization failed. Please check the config file path and the configuration.

3. API 连接服务器失败，需要确认 connect 传入的配置文件路径和配置信息是否正确。

  Failed to connect to server. Please check the config file path and the configuration.

4. 登录错误，用户名，密码错误。

  login failed: iRet [iRet], error: [errorMsg]

5. API 未初始化错误，需要检查是否 connect() 成功。

  API is not initialized. Please check whether the connection is set up via connect().

6. subscribe 的 `streamTable` 参数错误，需要是一个 shared streamTable（共享流表）。

  The third parameter "streamTable" must be a shared stream table.

7. subscribe 的 `location` 参数错误，需要是 `sh` 或 `sz`。

  The second parameter "location" must be `sh` or `sz`.

8. subscribe 的 `type` 参数错误，应该是 `snapshot` or `trade` or `orders`。

  The first parameter "type" must be  `snapshot`, `trade` or `orders`.

9. subscribe `streamTable` 参数的 schema 错误，schema 需和 SDK 一致。

  Subscription failed. Please check if the schema of “streamTable” is correct.

10. 重复订阅错误，想要更换同一类订阅 (如 `snapshot`, `sh` 两个字段唯一标识一类订阅) 订阅的流表，需要先执行 unsubscribe，然后再更新订阅。

  Subscription already exists. To update subscription, call unsubscribe() and try again.

11. unsubscribe 时 API 未初始化错误。

   API is not initialized. Please check whether the connection is set up via connect().

12. close() 错误，在未初始化（未调用 connect）的 API 上进行了 close。

   Failed to close(). There is no connection to close.