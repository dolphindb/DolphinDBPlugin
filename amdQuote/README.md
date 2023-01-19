# DolphinDB amdQuote Plugin(Linux)

您正在查看的教程位于 release200 分支，需要使用 2.00.X 版本 server。

DolphinDB amdQuote 插件目前仅支持 Linux 系统。本文仅介绍 Linux 系统上如何安装及使用该插件。

## 1. 安装构建

### 1.1. 预编译安装

预先编译的插件文件存放在[bin/linux64](https://github.com/dolphindb/DolphinDBPlugin/tree/release130/amdQuote/bin/linux64) 目录。将该目录下的所有文件（包括动态库文件）下载至 DolphinDB server 所在机器的如下目录：/DolphinDB/server/plugins/amdQuote。

Linux 终端执行以下命令，指定插件运行时需要的动态库路径
```
export LD_LIBRARY_PATH=/your_plugin_path:$LD_LIBRARY_PATH //指定动态库位置 
```
启动 DolphinDB，加载插件：
```
cd DolphinDB/server //进入 DolphinDB serve r目录
./dolphindb //启动 DolphinDB server
 loadPlugin("/your_plugin_path/PluginAmdQuote.txt") //加载插件
```

### 1.2. 使用 CMake 编译构建

安装 CMake

```bash
sudo apt install cmake
```

编译整个项目
```bash
mkdir build
cd build
cp /path_to_dolphindb/libDolphinDB.so ../lib
cmake .. -DAMDAPIDIR=<amd_ami_dir>
make -j
```

libPluginAmdQuote.so 文件会在编译后生成。编译后 build 目录下会产生文件 libPluginAmdQuote.so 和 PluginAmdQuote.txt。

参考 [预编译安装](#11-预编译安装) 指定动态库的环境变量，并加载插件。

## 接口说明

**amdQuote::connect(username, password, ips, ports, options)**

**参数**

`username` 为字符串标量，AMD 行情服务器的用户名。

`password` 为字符串标量，AMD 行情服务器的密码。

`ips` 为字符串向量, AMD 行情服务器 IP 列表，需要和端口列表数量相同。

`ports` 为整型向量，AMD 行情服务器端口列表，需要和 IP 列表数量相同。

`options` 可选参数。是字典类型，表示扩展参数。当前键支持 receivedTime 和 DailyIndex。
其中：

* receivedTime 表示是否获取插件收到行情数据的时间戳。其指定为 dict(["ReceivedTime"], [true]) 时，getSchema 获取的表结构中将包含插件收到行情数据的时间戳列。
* DailyIndex 表示是否添加每天按 channel_no 递增的数据列。其指定为 dict(["DailyIndex"], [true]) 时，getSchema 获取的表结构中将包含插件收到行情数据的按 channel_no 递增的列。

**函数详情**

创建一个和 AMD 行情服务器之间的连接，返回一个句柄。

**amdQuote::subscribe(handle, type, streamTable, marketType, codeList, transform)**

**参数**

`handle` connect 接口返回的句柄。

`type` 字符串标量，表示行情的类型，可取以下值：'snapshot'（股票快照）, 'execution'（股票逐笔成交）, 'order'（股票逐笔委托）, 'index'（指数）, 'orderQueue'（委托队列）, 'fundSnapshot'（基金快照）, 'fundExecution'（基金逐笔成交），'fundOrder'（基金逐笔委托），'bondSnapshot'（债券快照），'bondOrder'（债券逐笔委托），'bondExecution'（债券逐笔成交）。

`streamTable` 表示一个共享流表的表对象。订阅前需要创建一个共享流表，且该流表的 schema 需要和获取的行情数据结构一致。可以通过插件提供的 getSchema 函数来获取行情数据的 schema。

`marketType` 整型标量，可选。表示市场类型。需要和 AMD 中定义的市场类型一致。不传该参数表示订阅所有市场。

`codeList` 字符串向量，可选。表示股票列表。不传该参数表示订阅所有股票。

`transform`: 一元函数（其参数是一个表）。插入到 DolphinDB 表前对表进行转换，例如替换列。请注意，传入的一元函数中不能存在对 DFS 表的操作，例如：读取或写入 DFS 表，获取 DFS 表的 schema 等。

**函数详情**

订阅指定市场、行情数据类型和股票列表的行情数据到 streamTable 流表。

**amdQuote::unsubscribe(handle, type, marketType, codeList)**

**参数**

`handle` connect 接口返回的句柄。

`dataType` 字符串标量，表示行情的类型，可取以下值：'snapshot', 'execution', 'order', 'index', 'orderQueue', 'fundSnapshot', 'fundExecution', 'fundOrder', 'bondSnapshot', 'bondOrder', 'bondExecution' 和 'all'。其中，'all' 表示取消所有订阅。

`marketType` 整型标量，表示市场类型，需要和 AMD 中定义的市场类型一致。

`codeList` 字符串向量，表示股票列表。

**函数详情**

取消对行情数据的订阅。
* 如果 *dataType* 指定为 'all'，表示取消所有订阅，此时无需指定 *marketType* 和 *codeList*。
* 如果 *dataType* 指定非 'all' 的值：
  * 只指定 *marketType*，表示取消 *marketType* 下的所有订阅。
  * 同时指定 *marketType* 和 *codeList*，表示只取消对 *codeList* 的订阅。

**amdQuote::close(handle)**

**参数**

`handle` connect 接口返回的句柄。

**函数详情**
关闭当前连接。  
通过 connect 创建连接时，AMD API 会创建线程在内的一些资源。当用户确定不使用行情数据之后需要手动调用 close 接口释放资源。

### **amdQuote::getSchema(type)**

**参数**

`type` 字符串标量，表示行情的类型，可取以下值：'snapshot', 'execution', 'order', 'index', 'orderQueue', 'fundSnapshot', 'fundExecution', 'fundOrder', 'bondSnapshot', 'bondOrder' 和 'bondExecution'。

**函数详情**

该函数应该在 connect 函数之后调用。获取行情数据的表结构。返回一个表，包含两列：name 和 type，分别表示该行情表结构的名字和类型。通过该表来创建具有相同结构的共享流表。

**amdQuote::getStatus(handle)**

**参数**

`handle` connect 接口返回的句柄。

**函数详情**

获取当前连接下所有订阅的状态。返回一个表，包含三列：datatype, isSubscribed 和 marketType，分别表示订阅的行数数据类型，是否被订阅和订阅的市场类型。  

注：getStatus 不会显示取消订阅部分股票的状态，因此，当取消对部分股票的订阅后，通过 getStatus 查看的结果为该股票所属的行情数据（datatype）被取消订阅（isSubscribed=false）。

**amdQuote::getCodeList()**

**参数**

无

**函数详情**

获取当前连接下的代码表结构。

**amdQuote::getETFCodeList()**

**参数**

无

**函数详情**

获取当前连接下的 ETF 代码表结构。


**amdQuote::enableLatencyStatistics(handle, flag)**

**参数**

`handle` connect 接口返回的句柄。

`flag` 布尔类型，表示是否开启统计耗时功能。

**函数详情**

开启或关闭耗时统计的功能。耗时指从插件接收到 AMD 消息至数据转换后写入流表的时间。开启后，统计信息将输出到 server 端 Log 日志，每隔30秒，进行一次统计输出。

注意，开启耗时统计功能会增加 Log 文件的输出，因此建议仅当 AMD 插件出现性能问题时，才开启该功能排查问题。

## 使用示例

1.使用 loadPlugin 加载插件 
```
loadPlugin("Your_plugin_path/PluginAmdQuote.txt")
```
2.连接 AMD 服务器
```
handle = amdQuote::connect(`admin, `123456, [`119.29.65.231], [8031], dict(["ReceivedTime"], [true]))
```
3.获取对应的表结构
```
snapshotSchema = getSchema(`snapshot); 

executionSchema = getSchema(`execution);

orderSchema = getSchema(`order);
```

4.创建流表
```
snapshotTable = streamTable(10000:0, snapshotSchema[`name], snapshotSchema[`type]);

executionTable = streamTable(10000:0, executionSchema[`name], executionSchema[`type]);

orderTable = streamTable(10000:0, orderSchema[`name], orderSchema[`type]);
```
5.共享并持久化流表
```
enableTableShareAndPersistence(table=snapshotTable, tableName=`snapshot1, cacheSize=10000)

enableTableShareAndPersistence(table=executionTable, tableName=`execution1, cacheSize=10000)

enableTableShareAndPersistence(table=orderTable, tableName=`order1, cacheSize=10000)
```
6.订阅深圳市场全部股票代码 

因为 AMD API 文档中深圳市场的枚举值为102，所以 `subscribe` 的 *marketType* 参数指定为102.

对应的订阅快照，逐笔成交和逐笔委托的示例为： 
```
amdQuote::subscribe(handle, `snapshot, snapshot1, 102) 

amdQuote::subscribe(handle, `execution, execution1, 102)

amdQuote::subscribe(handle, `order, order1, 102)
```
7.取消深圳市场的快照订阅 

```
amdQuote::unsubscribe(handle， `snapshot, 102)  
```
取消逐笔成交，逐笔委托，取消全部数据的订阅分别是： 
```
amdQuote::unsubscribe(handle, `execution, 102) 

amdQuote::unsubscribe(handle, `order, 102) 

amdQuote::unsubscribe(handle, `all, 102)  
```
8.使用完成后，手动调用接口释放资源 

```
amdQuote::close(handle)
```
