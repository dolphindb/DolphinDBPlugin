# DolphinDB INSIGHT Plugin

为对接华泰 INSIGHT 行情服务软件，DolphinDB 开发了 INSIGHT 插件。通过该插件获取交易所的行情。

> 注意：INSIGHT 插件仅支持 Linux 系统，该版本对应的 Insight SDK 为 TCP 版本 SDK
## 1 使用 CMake 编译

**注意：** 编译之前请确保 libDolphinDB.so 在 gcc 可搜索的路径中。可使用 LD_LIBRARY_PATH 指定其路径，或者直接将其拷贝到 build 或 lib 目录下。

```bash
cd /path/to/PluginInsight
mkdir build && cd build
cmake ..
make
```

编译完成后，build 目录下将生成加载插件所需要的 libPluginInsight.so 文件。

加载插件前，需要将 [cert](include/cert) 文件夹放到 dolphindb 可执行文件同目录下，执行以下命令：
```bash
cp -r /path_to_insight/include/cert /path_to_server/
export LD_LIBRARY_PATH=/path_to_insight/lib:$LD_LIBRARY_PATH
```

## 2 用户接口

### connect

#### 语法
```
insight::connect(handles, ip, port, user, password, [workPoolThreadCount=5], [options], [ignoreApplSeq=false])
```

#### 参数

`handles`：类型为 Dictionary，Dictionary 的键为 `StockTick`, `IndexTick`, `FuturesTick`, `OrderTransaction`, `Transaction` 或 `Order`，值为共享流表 或者 一个 Dictionary。

    六种数据分别接入股票的快照、指数的快照、期货的快照、逐笔合成类型、逐笔成交类型、逐笔委托类型。

    当值为 Dictionary 时，key 为整型，代表 channel 号。value 为一个共享流表，即对应 channel 数据需要接入的数据表。

`ip`：服务器地址，类型为字符串标量。

`port`：服务器端口，类型为整型标量。

`user`：用户名，类型为字符串标量。

`password`：密码，类型为字符串标量。

`workPoolThreadCount`：可选，处理线程池的线程数，类型为整型标量，默认为 5。大小需要在 1-32767 之间

`options`：可选，是字典类型，表示扩展参数，key 为 string 类型，value 为 boolean 类型。当前支持 `ReceivedTime`, `OutputElapsed`。

    ReceivedTime 表示是否获取插件收到行情数据的时间戳，默认为 true。其指定为 dict(["ReceivedTime"], [true]) 时，插件处理输出的数据将包含行情数据的时间戳列。

    OutputElapsed 表示是否获取 Insight 插件 接收数据处理的时延，默认为 false。其指定为 dict(["OutputElapsed"], [true]) 时，插件处理输出的数据将包含行情数据的时延列。时延的定义：'insight 回调函数返回数据' 到 '准备开始 transform' 处理，或准备 append 到共享流表前’ 这段时间。该列的单位为纳秒。

`ignoreApplSeq`：可选，类型为布尔标量，默认为 false。在 `OrderTransaction` 合并类型订阅中生效。如果为 false 则当 `OrderTransaction` 数据中出现数据丢失时停止接收数据，如果为 true 则忽略数据丢失问题，继续接收数据。

#### 详情

注册消息接收接口并连接 Insight 服务器，返回 Insight tcpClient 连接的句柄。

### subscribe

#### 语法

```
insight::subscribe(tcpClient, marketDataTypes, securityIDSource, securityType)
```

#### 参数

`tcpClient`：connect 的返回值。

`marketDataTypes`：字符串向量，表示行情数据类型，支持以下值：`MD_TICK`, `MD_ORDER`, `MD_TRANSACTION` 和 `MD_ORDER_TRANSACTION`。`MD_ORDER_TRANSACTION` 为特殊的订阅类型，指的是逐笔合成类型，其他类型均与 insight 规定的 `EMarketDataType` 枚举类型含义相同。

`securityIDSource`：字符串标量，表示交易所类型，支持以下值：`XSHE`, `XSHG` `CCFX`, `CSI`。类型含义与 insight 规定的 `ESecurityIDSource` 枚举类型含义相同。

`securityType`：字符串标量，表示产品类型，支持以下值：`StockType`, `FundType`, `BondType`, `IndexType`, `FuturesType`。类型含义与 insight 规定的 `ESecurityType` 枚举类型含义相同。

#### 详情

订阅数据，并将所订阅的数据保存在由 connect 的 handles 参数指定的表中。

### close

#### 语法

`insight::close(tcpClient)`

#### 参数

`tcpClient`：connect 的返回值。

#### 详情

关闭连接。

#### 示例

```bash
insight::close(tcpClient)
```

### getSchema

#### 语法
```
insight::getSchema(type, [options])
```

#### 参数

`dataType`：为字符串标量，指需要获取 schema 的类型 `OrderTransaction`, `StockTick`, `IndexTick`, `FuturesTick`, `Transaction`, `Order` 或 `OrderTransaction`。

`options`：可选，是字典类型，表示扩展参数，key 为 string 类型，value 为 boolean 类型。当前支持 `ReceivedTime`, `OutputElapsed`。

    ReceivedTime 表示是否获取插件收到行情数据的时间戳，默认为 true。其指定为 dict(["ReceivedTime"], [true]) 时，getSchema 获取的表结构中将包含插件收到行情数据的时间戳列

    OutputElapsed 表示是否获取 Insight 插件 接收数据处理的时延，默认为 false。其指定为 dict(["OutputElapsed"], [true]) 时，getSchema 获取的表结构中将包含插件收到行情数据的时延列。时延的定义：'insight 回调函数返回数据' 到 '准备开始 transform' 处理，或准备 append 到共享流表前’ 这段时间。该列的单位为纳秒。


#### 详情

获取对应表结构。返回一个表，包含 name 和 type 两列。

### getHandle()

#### 语法
```
handle = insight::getHandle()
```

#### 参数

无

#### 详情

返回已有的 insight 连接句柄，如果插件没有被连接过，会抛出异常。

### getStatus(tcpClient)

#### 语法
```
insight::getStatus(tcpClient)
```

#### 参数

`tcpClient`：insight 连接句柄，即 connect 函数的返回值。

#### 详情

返回一个表格，包含各种已订阅数据的状态信息，包含数据类型  `OrderTransaction`, `StockTick`, `IndexTick`, `FuturesTick`, `Transaction` 和 `Order`

| 列名                    | 含义                       | 类型          |
| ----------------------- | -------------------------- | ------------- |
| **topicType**           | 订阅的名称                 | STRING        |
| **channelNo**           | OrderTransaction 分 channel 订阅时的 channel号 | INT        |
| **startTime**           | 订阅开始的时间             | NANOTIMESTAMP |
| **endTime**             | 订阅结束的时间             | NANOTIMESTAMP |
| **firstMsgTime**        | 第一条消息收到的时间       | NANOTIMESTAMP |
| **lastMsgTime**         | 最后一条消息收到的时间     | NANOTIMESTAMP |
| **processedMsgCount**   | 已经处理的消息数           | LONG          |
| **lastErrMsg**          | 最后一条错误信息           | STRING        |
| **failedMsgCount**      | 处理失败的消息数           | LONG          |
| **lastFailedTimestamp** | 最后一条错误消息发生的时间 | NANOTIMESTAMP |
| **subscribeInfo**       | 该订阅涉及的市场和投资品类型  | STRING        |

### unsubscribe(tcpClient)

#### 语法
```
insight::unsubscribe(tcpClient)
```

#### 参数

`tcpClient`：insight 连接句柄，即 connect 函数的返回值。

#### 详情

取消当前所有订阅。

### 示例

1. 加载插件

```
pluginFile = “path_to_insight/PluginInsight.txt”;
loadPlugin(pluginFile);
```


2. 创建用于保存订阅数据的表
```
stockTickSchema = insight::getSchema(`StockTick);
share streamTable(10000:0, stockTickSchema[`name], stockTickSchema[`type]) as stockTickTable;
```


3. 连接服务器
```
ip = "168.61.69.192";
port = 10317;
user = "mdc-flow-client-25-36";
password = "mdc-vss-shlv1";

handles = dict([`StockTick], [stockTickTable]);
tcpClient = insight::connect(handles, ip, port, user, password);
```


4. 订阅
```
insight::subscribe(tcpClient, `MD_TICK`MD_ORDER`MD_TRANSACTION, `XSHG, `StockType);
```

5. 取消订阅
```
insight::unsubscribe(tcpClient);
```

6. 关闭连接
```
insight::close(tcpClient);
```

# Release Note

## 1.30.23

### 新增功能

- 新增 `insight::getHandle` 接口，用于获取已有连接句柄。
- `insight::connect` 接口新增参数 *options*，表示扩展参数。
- `insight::connect` 接口新增参数 *ignoreApplSeq*，用于决定当 `OrderTransaction` 数据中出现数据丢失时是否停止接收数据。
- 新增对 OrderTransaction 合并数据类型和基金、债券 投资品类型的支持。
- 新增时延统计功能。（`insight::connect`  *options*）
- 支持同时接收 order 和 trade 数据按 ChannelNo 多线程异步写入 DolphinDB 目标表。

### 功能优化

- 优化了数据解析过程，降低了时延。

## 1.30.22

### 故障修复

- 修复了在断网时取消订阅失败的问题。
- 修复了在执行 insight::close 后，再次执行 insight::getStatus 时 server 宕机的问题。
- 修复了当首次连接时输入错误密码，后续连接一直报错的问题。
