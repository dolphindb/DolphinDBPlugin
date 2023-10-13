# DolphinDB INSIGHT Plugin

为对接华泰 INSIGHT 行情服务软件，DolphinDB 开发了 INSIGHT 插件。通过该插件获取交易所的行情。

> 注意：INSIGHT 插件仅支持 Linux 系统

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
insight::connect(handles, ip, port, user, password, [workPoolThreadCount])
```

#### 参数

`handles`：类型为 Dictionary，Dictionary 的键为'StockTick', 'IndexTick', 'FuturesTick', 'StockTransaction' 或 'StockOrder'，值为 table。请注意，table 不能为 DFS 表。

`ip`：服务器地址，类型为字符串标量。

`port`：服务器端口，类型为整型标量。

`user`：用户名，类型为字符串标量。

`password`：密码，类型为字符串标量。

`workPoolThreadCount`：可选，处理线程池的线程数，类型为整型标量，默认为 5。

#### 详情

注册消息接收接口并连接服务器，返回表示 tcpClient 的句柄。

### subscribe

#### 语法

```
insight::subscribe(tcpClient, marketDataTypes, securityIDSource, securityType)
```

#### 参数

`tcpClient`：connect 的返回值。

`marketDataTypes`：字符串向量，表示行情数据类型，支持以下值：'MD_TICK', 'MD_ORDER' 和 'MD_TRANSACTION'。

`securityIDSource`：字符串标量，表示交易所类型，支持以下值：'XSHE', 'XSHG' 'CCFX', 'CSI'。

`securityType`：字符串标量，表示产品类型，支持以下值：'StockType', 'IndexType', 'FuturesType'。

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
insight::getSchema(dataType)
```

#### 参数

`dataType`：'StockTick', 'IndexTick', 'FuturesTick', 'StockTransaction' 或 'StockOrder'。

#### 详情

获取对应表结构。返回一个表，包含 name 和 type 两列。

### getStatus(tcpClient)

#### 语法
```
insight::getStatus(tcpClient)
```

#### 参数

`tcpClient`：connect 的返回值。

#### 详情

返回保存订阅信息的表，包含三列，分别是 marketType, securityIdSource 和 securityType。

### unsubscribe(tcpClient)

#### 语法
```
insight::unsubscribe(tcpClient)
```

#### 参数

`tcpClient`：connect 的返回值。

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

# ReleaseNotes:

## 故障修复

* 修复了在断网时取消订阅失败的问题。（**1.30.22**）
* 修复了在执行 insight::close 后，再次执行 insight::getStatus 时 server 宕机的问题。（**1.30.22**）
* 修复了当首次连接时输入错误密码，后续连接一直报错的问题。（**1.30.22**）
