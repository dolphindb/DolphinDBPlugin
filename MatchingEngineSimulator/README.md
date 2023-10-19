# 模拟撮合引擎插件使用说明

模拟撮合引擎插件（Matching Engine Simulator）用于模拟用户在某个时间点发出或取消订单的操作，并获取相应的交易结果。

该插件以行情数据（快照数据或逐笔数据）和用户委托订单（买方或卖方）作为输入，根据订单撮合规则实现模拟撮合后，将订单成交结果（含部分成交结果、拒绝订单和已撤订单）输出至交易明细输出表，未成交部分等待与后续行情撮合成交，或者等待用户撤单。

- [在插件市场安装插件](#在插件市场安装插件)
  - [版本要求](#版本要求)
  - [安装步骤](#安装步骤)
- [接口说明](#接口说明)
  - [MatchingEngineSimulator::createMatchEngine](#matchingenginesimulatorcreatematchengine)
  - [MatchingEngineSimulator::getOpenOrders](#matchingenginesimulatorgetopenorders)
  - [MatchingEngineSimulator::resetMatchEngine](#matchingenginesimulatorresetmatchengine)
  - [MatchingEngineSimulator::getEngineList](#matchingenginesimulatorgetenginelist)
  - [MatchingEngineSimulator::getSymbolList](#matchingenginesimulatorgetsymbollist)
  - [MatchingEngineSimulator::stopMatchEngine](#matchingenginesimulatorstopmatchengine)
  - [MatchingEngineSimulator::extractInfo](#matchingenginesimulatorextractinfo)
  - [MatchingEngineSimulator::insertMsg](#matchingenginesimulatorinsertmsg)
  - [MatchingEngineSimulator::setLimitPrice](#matchingenginesimulatorsetlimitprice)
  - [MatchingEngineSimulator::getSnapshot](#matchingenginesimulatorgetsnapshot)
- [例子](#例子)
  - [例子：通过命名函数指定模拟撮合交易细节](#例子通过命名函数指定模拟撮合交易细节)
  - [例子：快照模式下以模式二模拟撮合委托订单](#例子快照模式下以模式二模拟撮合委托订单)
- [参考](#参考)


## 在插件市场安装插件

### 版本要求

- DolphinDB Server: 2.00.9及更高版本

### 安装步骤

1. 在DolphinDB 客户端中使用 `listRemotePlugins` 命令查看插件仓库中的插件信息。
    
    ```
    login("admin", "123456")
    listRemotePlugins(, "http://plugins.dolphindb.cn/plugins/")
    ```
    
1. 使用 `installPlugin` 命令完成插件安装。
    
    ```
    installPlugin("MatchingEngineSimulator")
    ```
    
    返回：<path_to_MatchingEngineSimulator_plugin>/PluginMatchingEngineSimulator.txt
    
    
1. 使用 `loadPlugin` 命令加载插件（即上一步返回的.txt文件）。

    ```
    loadPlugin("<path_to_MatchingEngineSimulator_plugin>/PluginMatchingEngineSimulator.txt")
    ```
    
**注意**：使用2.00.9版本server的用户，可从[二进制插件包](https://gitee.com/dolphindb/DolphinDBPlugin/tree/release200.9/MatchingEngineSimulator/bin/linux) 中获取二进制插件包，并用 `loadPlugin` 命令加载后使用。

## 接口说明

### MatchingEngineSimulator::createMatchEngine

#### 详情

创建一个模拟撮合引擎。

#### 语法

```
MatchingEngineSimulator::createMatchEngine(name, 
	exchange, 
	config, 
	dummyQuotationTable,
	quotationColMap, 
	dummyUserOrderTable,
	userOrderColMap, 
	tradeOutputTable,
	[compositeOutputTable], 
	[snapshotOutputTable])	
```


#### 参数

- `name`：名称，字符串标量，全局唯一。
- `exchange`：交易所标识，深交所"XSHE"或上交所"XSHG"。
- `config`：是一个dictionary的标量，包含配置的键值对，类型为(STRING, DOUBLE)。

    | **字典key**            | **含义**                                                     |
    | ---------------------- | ------------------------------------------------------------ |
    | dataType               | 行情类别：0表示逐笔，1表示快照                               |
    | depth                  | 匹配的订单簿深度,5 - 50                                      |
    | outputOrderBook        | 是否输出订单簿。0：不输出， 1：输出                          |
    | orderBookInterval      | 如果需要输出订单簿，输出订单簿的最小时间间隔，单位为毫秒     |
    | latency                | 模拟时延，单位为毫秒，用来模拟用户订单从发出到被处理的时延。（用户订单的撤单无时延） |
    | orderBookMatchingRatio | 成交百分比                                                   |
    | matchingMode           | 快照模式下，匹配的两种模式，可设置为1或者2，分别对应按模式一撮合订单或按照模式二撮合订单                   |
    | matchingRatio          | 快照模式下，快照的区间成交百分比，默认情况下和成交百分比orderBookMatchingRatio相等 |
    | mergeOutputs           | 是否需要输出到复合输出表                                     |
    | timeDetail             | 成交输出表中是否需要订单收到时的行情最新时间，”匹配开始时间“”匹配完成时间“0：不需要，1：需要 |
    | cpuId                  | 绑定到的CPU核的ID，只在第一次appendMsg的时候绑定该线程       |



- `dummyQuotationTable`：插入行情数据的表的实际结构，对于一些引擎内部使用到的列，由参数quotationColMap提供列名映射关系
- `quotationColMap`：行情表的列名映射关系，一个字典标量，类型为(STRING, STRING)。

    其中，对于逐笔模式，行情表必须提供的列有：
    
    | **名称**     | **类型**  | **含义**                                                     |
    | ------------ | --------- | ------------------------------------------------------------ |
    | symbol       | SYMBOL    | 股票标的                                                     |
    | symbolSource | SYMBOL    | 证券市场：深交所、上交所                                     |
    | time         | TIMESTAMP | 时间                                                         |
    | sourceType   | INT       | 0代表order；1代表transaction                                 |
    | orderType    | INT       | order：<br>1：市价；<br>2：限价；<br>3：本方最优；<br>10：撤单（仅上交所，即上交所撤单记录在order中）<br>transaction：<br>0：成交；<br>1：撤单（仅深交所，即深交所撤单记录在transaction中） |
    | price        | DOUBLE    | 订单价格                                                     |
    | qty          | LONG      | 订单数量                                                     |
    | buyNo        | LONG      | transaction对应其原始数据；order填充OrderNO，无意义，深交所数据为了补全上交所数据格式增加的冗余列 |
    | sellNo       | LONG      | transaction对应其原始数据；order填充OrderNO，无意义，深交所数据为了补全上交所数据格式增加的冗余列 |
    | direction    | INT       | 1（买 ）or 2（卖）                                           |
    | seqNum       | LONG      | 逐笔数据序号                                                 |

    对于快照模式，行情表必须提供的列有（成交价格和成交数量列表字段在撮合模式二必须字提供，撮合模式一时可以不包含这2列）
    
    | **名称**        | **类型**  | **含义**                 |
    | --------------- | --------- | ------------------------ |
    | symbol          | SYMBOL    | 股票标的                 |
    | symbolSource    | SYMBOL    | 证券市场：深交所、上交所 |
    | time            | TIMESTAMP | 时间                     |
    | lastPrice       | DOUBLE    | 最新价                   |
    | upperLimitPrice | DOUBLE    | 涨跌停板价               |
    | lowerLimitPrice | DOUBLE    | 跌停板价                 |
    | totalBidQty     | LONG      | 区间买单成交数量总和     |
    | totalOfferQty   | LONG      | 区间卖单成交数量总和     |
    | bidPrice        | DOUBLE\[]  | 买单价格列表             |
    | bidQty          | LONG\[]    | 买单数量列表             |
    | offerPrice      | DOUBLE\[]  | 卖单价格列表             |
    | offerQty        | LONG\[]    | 卖单数量列表             |
    | tradePrice      | DOUBLE\[]  | 成交价格列表             |
    | tradeQty        | LONG\[]    | 成交数量列表             |

- `dummyUserOrderTable`：插入用户订单数据的表的实际结构，对于一些引擎内部使用到的列，由参数userOrderColMap提供列名映射关系。
- `userOrderColMap`：用户订单表的列名映射关系，一个字典标量，类型为(STRING, DOUBLE)。
  用户订单表必须提供的列有

    | **名称**  | **类型**  | **含义**                                                     |
    | --------- | --------- | ------------------------------------------------------------ |
    | symbol    | STRING    | 股票标的。取消订单时股票标的无效，以orderID为准。            |
    | time      | TIMESTAMP | 时间                                                         |
    | orderType | INT       | 上交所：<br>0：市价单中最优5档即时成交剩余撤销委托订单<br>1：市价单中最优5档即时成交剩余转限价委托订单<br>2：市价单中本方最优价格委托订单<br>3：市价单中对手方最优价格委托订单<br>5：限价单<br>6：撤单<br>深交所：<br>0：市价单中最优五档即时成交剩余撤销委托订单<br>1：市价单中即时成交剩余撤销委托订单<br>2：市价单中本方最优价格委托订单<br>3：市价单中对手方最优价格委托订单<br>4：市价单中全额成交或撤销委托订单<br>5：限价单<br>6：撤单 |
    | price     | DOUBLE    | 订单委托价格                                                 |
    | qty       | LONG      | 委托数量                                                     |
    | direction | INT       | 买 or 卖                                                     |
    | orderID   | LONG      | 用户订单ID，仅撤单时起作用。                                 |

- `compositeOutputTable`：复合输出表，包含订单簿及成交情况。需要调用extractInfo接口来解析出具体数据

    | **名称** | **类型** | **含义**                       |
    | -------- | -------- | ------------------------------ |
    | msgType  | INT      | 成交结果（1） 或 订单簿（2） |
    | content  | BLOB     | 具体数据                       |

- `tradeOutputTable`：成交结果输出表。注意，后3列只在配置项timeDetail为1时启用

    | **名称**            | **类型**      | **含义**                                                     |
    | ------------------- | ------------- | ------------------------------------------------------------ |
    | orderID             | LONG          | 成交的用户订单ID                                             |
    | symbol              | STRING        | 股票标的                                                     |
    | direction           | INT           | 1（买） 或 2（卖）                                           |
    | sendingTime         | TIMESTAMP     | 订单发送时间                                                 |
    | limitPrice          | DOUBLE        | 委托价格                                                     |
    | volumeTotalOriginal | LONG          | 订单委托数量                                                 |
    | tradeTime           | TIMESTAMP     | 成交时间                                                     |
    | tradePrice          | DOUBLE        | 成交价格                                                     |
    | volumeTraded        | LONG          | 成交量                                                       |
    | orderStatus         | INT           | 用户订单是否完全成交。<br>-1：表示订单被拒绝 <br>0：表示订单部分成交 <br>1：表示订单完全成交 <br>2：表示订单被撤单 |
    | orderReceiveTime    | NANOTIMESTAMP | 订单收到时的时间（系统时间）                                 |
    | insertTime          | TIMESTAMP     | 订单收到时的行情最新时间                                     |
    | startMatchTime      | NANOTIMESTAMP | 匹配开始时间                                                 |
    | endMatchTime        | NANOTIMESTAMP | 匹配完成时间                                                 |

- `snapshotOutputTable`：行情快照输出表

    | **名称**      | **类型**  | **含义**         |
    | ------------- | --------- | ---------------- |
    | symbol        | STRING    | 股票标的         |
    | time          | TIMESTAMP | 时间             |
    | avgBidPrice   | DOUBLE    | 买单成交均价     |
    | avgOfferPrice | DOUBLE    | 卖单成交均价     |
    | totalBidQty   | LONG      | 买单成交数量总和 |
    | totalOfferQty | LONG      | 卖单成交数量总和 |
    | bidPrice      | DOUBLE\[]  | 买单价格列表     |
    | bidQty        | LONG\[]    | 买单数量列表     |
    | offerPrice    | DOUBLE\[]  | 卖单价格列表     |
    | offerQty      | LONG\[]    | 卖单数量列表     |
    | lastPrice     | DOUBLE    | 最新价           |
    | highPrice     | DOUBLE    | 最高价           |
    | lowPrice      | DOUBLE    | 最低价           |


#### 返回值

无

### MatchingEngineSimulator::getOpenOrders

#### 详情

获取未成交的用户订单信息

#### 语法

```
MatchingEngineSimulator::getOpenOrders(engine, [symbol]);
```

#### 参数

- `engine`： 通过createMatchEngine接口创建的撮合引擎。
- `symbol`： 股票标的，字符串标量，可以不填，表示获取所有的未成交订单。

#### 返回值

返回值是一个表，包含如下列：

| **名称**  | **类型**  | **含义**       |
| --------- | --------- | -------------- |
| orderId   | LONG      | 订单id         |
| time      | TIMESTAMP | 时间           |
| symbol    | STRING    | 股票标的       |
| price     | DOUBLE    | 委托价格       |
| totalQty  | LONG      | 用户订单数量   |
| remainQty | LONG      | 用户订单余量   |
| direction | INT       | 买（1）卖（2） |

### MatchingEngineSimulator::resetMatchEngine

#### 详情

清空内部缓存的所有订单信息以及行情信息。

#### 语法

```
MatchingEngineSimulator::resetMatchEngine(engine)
```

#### 参数

- `engine`： 通过createMatchEngine接口创建的撮合引擎

#### 返回值

无

 

### MatchingEngineSimulator::getEngineList

#### 详情

获取所有的engine。

#### 语法

```
MatchingEngineSimulator::getEngineList();
```

#### 返回值

返回一个engine列表，类型为dictionary, key为id, value为engine对象

 

### MatchingEngineSimulator::getSymbolList

#### 详情

获取某引擎中当前存在的股票symbol列表。

#### 语法

```
MatchingEngineSimulator::getSymbolList();
```

#### 参数

- `engine`： 是通过createMatchEngine接口创建的撮合引擎

#### 返回值

返回一个STRING类型的vector，包含当前存在的股票标的。

 

### MatchingEngineSimulator::stopMatchEngine

#### 详情

停止模拟撮合引擎。

#### 语法

```
MatchingEngineSimulator::stopMatchEngine(engine, [symbol]);
```

#### 参数

- `engine`：通过createMatchEngine接口创建的撮合引擎
- `symbol`：可选参数，字符串标量，表示需要停止模拟的股票，若不填则停止所有股票模拟

 

### MatchingEngineSimulator::extractInfo

#### 详情

解析compositeOutputTable表中的内容。

#### 语法

```
MatchingEngineSimulator::extractInfo(msgType, msg, [table]);
```

#### 参数

- `msgType`： 1 成交结果 2 订单簿
- `msg`： 待解析的字符串
- `table`： 与compositeOutputTable同结构的表

#### 返回值

如果指定 table，返回一个table，并将解析结果append到表的末尾。



### MatchingEngineSimulator::insertMsg

#### 详情

插入行情和用户订单数据

#### 语法

```
MatchingEngineSimulator::insertMsg(engine, msgBody, msgId);
```

#### 参数

- `engine`： 通过createMatchEngine接口创建的撮合引擎。
- `msgBody`：行情或用户订单数据。
- `msgId`：数据类型。1是行情，2是用户订单。

#### 返回值

- 如果插入的是行情数据，返回Void；
- 如果插入的是用户订单数据，返回用户的订单id（LONG类型的向量）。

### MatchingEngineSimulator::setLimitPrice

#### 详情

设置模拟撮合引擎的涨停价和跌停价。

**注意**：价格超过涨停价和跌停价范围内的用户订单都会被拒绝。

#### 语法

```
MatchingEngineSimulator::setLimitPrice(engine, data);
```

#### 参数

- `engine`： 通过createMatchEngine接口创建的撮合引擎。
- `data`：一个表。包含3列(symbol，upperLimitPrice，lowerLimitPrice)，分别是STRING，DOUBLE，DOUBLE类型。

#### 返回值

如果设置成功，返回true。

### MatchingEngineSimulator::getSnapshot

#### 详情

获取逐笔引擎中的行情快照信息。

#### 语法

```
MatchingEngineSimulator::getSnapshot(engine, [symbolList]);
```

#### 参数

- `engine`： 是通过createMatchEngine接口创建的逐笔撮合引擎。
- `symbolList`： 股票标的，字符串向量。如果没有填该参数，表示获取所有股票标的快照。

#### 返回值

返回一个表 ，结构如下：

| **名称**      | **类型**  | **含义**         |
| ------------- | --------- | ---------------- |
| symbol        | STRING    | 股票标的         |
| time          | TIMESTAMP | 时间             |
| avgBidPrice   | DOUBLE    | 买单成交均价     |
| avgOfferPrice | DOUBLE    | 卖单成交均价     |
| totalBidQty   | LONG      | 买单成交数量总和 |
| totalOfferQty | LONG      | 卖单成交数量总和 |
| bidPrice      | DOUBLE\[]  | 买单价格列表     |
| bidQty        | LONG\[]    | 买单数量列表     |
| offerPrice    | DOUBLE\[]  | 卖单价格列表     |
| offerQty      | LONG[]    | 卖单数量列表     |
| lastPrice     | DOUBLE    | 最新价           |
| highPrice     | DOUBLE    | 最高价           |
| lowPrice      | DOUBLE    | 最低价           |

## 例子

### 例子：通过命名函数指定模拟撮合交易细节

```
login("admin", "123456") //登录
loadPlugin("/DolphinDBPlugin/MatchingEngineSimulator/bin/PluginMatchingEngineSimulator.txt") //加载插件
go

ORDER_SEL = 2
ORDER_BUY = 1
//行情订单
TYPE_ORDER = 0
TYPE_TRADE = 1

//行情订单类型
HQ_COUNTERPARTY_BEST = 1
HQ_LIMIT_ORDER = 2 
HQ_OUR_BEST = 3
HQ_ORDER_CANCEL = 10

//行情交易类型
TRADE_ORDER_DEAL = 0	
TRADE_ORDER_CANCEL = 1

//用户订单
MARKET_ORDER_1 = 0
MARKET_ORDER_2 = 1
MARKET_ORDER_3 = 2
MARKET_ORDER_4 = 3
MARKET_ORDER_5 = 4
LIMIT_ORDER = 5
ORDER_CANCEL = 6
TIME_FORWARD = 7

ORDER_SEL = 2
ORDER_BUY = 1

//行情订单
TYPE_ORDER = 0
TYPE_TRADE = 1

//行情订单类型
HQ_COUNTERPARTY_BEST = 1
HQ_LIMIT_ORDER = 2
HQ_OUR_BEST = 3
HQ_ORDER_CANCEL = 10

//行情交易类型
TRADE_ORDER_DEAL = 0
TRADE_ORDER_CANCEL = 1

//用户订单
MARKET_OTHER_BEST = 3
MARKET_THIS_BEST = 2
MARKET_BEST_FIVE = 0
MARKET_INSTANT_DEAL = 1
MARKET_ALL_DEAL = 4
LIMIT_ORDER = 5
ORDER_CANCEL = 6
TIME_FORWARD = 7

go
def getMgs(name,exchange,config){
    
    dummyQuotationTable = table(1:0, `symbol`symbolSource`time`lastPrice`highestPrice2`lowestPrice2`highestPrice`lowestPrice`openPrice`preClosePrice`upperLimitPrice`lowerLimitPrice`avgBidPrice`avgOfferPrice`totalBidQty`totalOfferQty`bidPrice`bidQty`offerPrice`offerQty, [STRING, STRING, TIMESTAMP,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE, DOUBLE, LONG, LONG, DOUBLE[], LONG[], DOUBLE[], LONG[]])
    quotationColMap = dict( `symbol`symbolSource`time`lastPrice`highestPrice2`lowestPrice2`highestPrice`lowestPrice`openPrice`preClosePrice`upperLimitPrice`lowerLimitPrice`avgBidPrice`avgOfferPrice`totalBidQty`totalOfferQty`bidPrice`bidQty`offerPrice`offerQty, `symbol`symbolSource`time`lastPrice`highestPrice2`lowestPrice2`highestPrice`lowestPrice`openPrice`preClosePrice`upperLimitPrice`lowerLimitPrice`avgBidPrice`avgOfferPrice`totalBidQty`totalOfferQty`bidPrice`bidQty`offerPrice`offerQty)   
    dummyUserOrderTable = table(1:0, `symbol`time`orderType`price`qty`direction`orderID, [STRING, TIMESTAMP, INT, DOUBLE, LONG, INT, LONG])
    userOrderColMap = dict( `symbol`time`orderType`price`qty`direction`orderID, `symbol`time`orderType`price`qty`direction`orderID)
    compositeOutputTable = table(1:0, `msgType`content, [INT, BLOB])
    tradeOutputTable  = table(1:0, `OrderSysID`Symbol`Direction`sendingTime`LimitPrice`VolumeTotalOriginal`TradeTime`TradePrice`VolumeTraded`OrderStatus`orderReceiveTime, [LONG, STRING, INT,TIMESTAMP,DOUBLE,LONG, TIMESTAMP,DOUBLE,LONG, INT,NANOTIMESTAMP])
    snapshotOutputTable  = table(1:0, `symbol`time`avgBidPrice`avgOfferPrice`totalBidQty`totalOfferQty`bidPrice`bidQty`offerPrice`offerQty, [STRING, TIMESTAMP,DOUBLE,DOUBLE, LONG, LONG,DOUBLE[],LONG[], DOUBLE[], LONG[]])
    engine = MatchingEngineSimulator::createMatchEngine(name, exchange, config, dummyQuotationTable, quotationColMap, dummyUserOrderTable, userOrderColMap, compositeOutputTable, tradeOutputTable, snapshotOutputTable)
    return [engine,tradeOutputTable,compositeOutputTable,snapshotOutputTable]
}

def initArgsSnapshot(){
	
    dummyQuotationTable = table(1:0, `symbol`symbolSource`time`lastPrice`upperLimitPrice`lowerLimitPrice`totalBidQty`totalOfferQty`bidPrice`bidQty`offerPrice`offerQty`tradePrice`tradeQty, [STRING, STRING, TIMESTAMP,DOUBLE,DOUBLE, DOUBLE, LONG, LONG, DOUBLE[], LONG[], DOUBLE[], LONG[], DOUBLE[], LONG[]])
    quotationColMap = dict( `symbol`symbolSource`time`lastPrice`upperLimitPrice`lowerLimitPrice`totalBidQty`totalOfferQty`bidPrice`bidQty`offerPrice`offerQty`tradePrice`tradeQty, `symbol`symbolSource`time`lastPrice`upperLimitPrice`lowerLimitPrice`totalBidQty`totalOfferQty`bidPrice`bidQty`offerPrice`offerQty`tradePrice`tradeQty)	
    dummyUserOrderTable = table(1:0, `symbol`time`orderType`price`qty`direction`orderID, [STRING, TIMESTAMP, INT, DOUBLE, LONG, INT, LONG])
    userOrderColMap = dict( `symbol`time`orderType`price`qty`direction`orderID, `symbol`time`orderType`price`qty`direction`orderID)	
    compositeOutputTable = table(1:0, `msgType`content, [INT, BLOB])	
    tradeOutputTable  = table(1:0, `OrderSysID`Symbol`Direction`sendingTime`LimitPrice`VolumeTotalOriginal`TradeTime`TradePrice`VolumeTraded`OrderStatus`orderReceiveTime, [LONG, STRING, INT,TIMESTAMP,DOUBLE,LONG, TIMESTAMP,DOUBLE,LONG, INT,NANOTIMESTAMP])	
    snapshotOutputTable  = table(1:0, `symbol`time`avgBidPrice`avgOfferPrice`totalBidQty`totalOfferQty`bidPrice`bidQty`offerPrice`offerQty`lastPrice`highPrice`lowPrice, [STRING, TIMESTAMP,DOUBLE,DOUBLE, LONG, LONG,DOUBLE[],LONG[], DOUBLE[], LONG[], DOUBLE, DOUBLE, DOUBLE])
    return [dummyQuotationTable,quotationColMap,dummyUserOrderTable,userOrderColMap,tradeOutputTable,compositeOutputTable,snapshotOutputTable]
}
def initArgsTick(){
    
    dummyQuotationTable = table(1:0, `symbol`symbolSource`time`sourceType`orderType`price`qty`buyNo`sellNo`direction`seqNum, [STRING, STRING, TIMESTAMP,INT, INT, DOUBLE, LONG, LONG, LONG, INT, LONG])    
    quotationColMap = dict( `symbol`symbolSource`time`sourceType`orderType`price`qty`buyNo`sellNo`direction`seqNum, `symbol`symbolSource`time`sourceType`orderType`price`qty`buyNo`sellNo`direction`seqNum)    
    dummyUserOrderTable = table(1:0, `symbol`time`orderType`price`qty`direction`orderID, [STRING, TIMESTAMP, INT, DOUBLE, LONG, INT, LONG])    
    userOrderColMap = dict( `symbol`time`orderType`price`qty`direction`orderID, `symbol`time`orderType`price`qty`direction`orderID)    
    compositeOutputTable = table(1:0, `msgType`content, [INT, BLOB])    
    tradeOutputTable  = table(1:0, `OrderSysID`Symbol`Direction`sendingTime`LimitPrice`VolumeTotalOriginal`TradeTime`TradePrice`VolumeTraded`OrderStatus`orderReceiveTime, [LONG, STRING, INT,TIMESTAMP,DOUBLE,LONG, TIMESTAMP,DOUBLE,LONG, INT,NANOTIMESTAMP])    
    snapshotOutputTable  = table(1:0, `symbol`time`avgBidPrice`avgOfferPrice`totalBidQty`totalOfferQty`bidPrice`bidQty`offerPrice`offerQty`lastPrice`highPrice`lowPrice, [STRING, TIMESTAMP,DOUBLE,DOUBLE, LONG, LONG,DOUBLE[],LONG[], DOUBLE[], LONG[], DOUBLE, DOUBLE, DOUBLE])           
    return [dummyQuotationTable,quotationColMap,dummyUserOrderTable,userOrderColMap,tradeOutputTable,compositeOutputTable,snapshotOutputTable]
}

def initArgsSnapshotmode2(){

    dummyQuotationTable = table(1:0, `symbol`symbolSource`time`lastPrice`highestPrice2`lowestPrice2`highestPrice`lowestPrice`openPrice`preClosePrice`upperLimitPrice`lowerLimitPrice`avgBidPrice`avgOfferPrice`totalBidQty`totalOfferQty`bidPrice`bidQty`offerPrice`offerQty`tradePrice`tradeQty, [STRING, STRING, TIMESTAMP,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE, DOUBLE, LONG, LONG, DOUBLE[], LONG[], DOUBLE[], LONG[],DOUBLE[],LONG[]])
    quotationColMap = dict( `symbol`symbolSource`time`lastPrice`highestPrice`lowestPrice`highestPrice2`lowestPrice2`openPrice`preClosePrice`upperLimitPrice`lowerLimitPrice`avgBidPrice`avgOfferPrice`totalBidQty`totalOfferQty`bidPrice`bidQty`offerPrice`offerQty`tradePrice`tradeQty, `symbol`symbolSource`time`lastPrice`highestPrice`lowestPrice`highestPrice2`lowestPrice2`openPrice`preClosePrice`upperLimitPrice`lowerLimitPrice`avgBidPrice`avgOfferPrice`totalBidQty`totalOfferQty`bidPrice`bidQty`offerPrice`offerQty`tradePrice`tradeQty)        
    dummyUserOrderTable = table(1:0, `symbol`time`orderType`price`qty`direction`orderID, [STRING, TIMESTAMP, INT, DOUBLE, LONG, INT, LONG])        
    userOrderColMap = dict( `symbol`time`orderType`price`qty`direction`orderID, `symbol`time`orderType`price`qty`direction`orderID)        
    compositeOutputTable = table(1:0, `msgType`content, [INT, BLOB])        
    tradeOutputTable  = table(1:0, `OrderSysID`Symbol`Direction`sendingTime`LimitPrice`VolumeTotalOriginal`TradeTime`TradePrice`VolumeTraded`OrderStatus`orderReceiveTime, [LONG, STRING, INT,TIMESTAMP,DOUBLE,LONG, TIMESTAMP,DOUBLE,LONG, INT,NANOTIMESTAMP])        
    snapshotOutputTable  = table(1:0, `symbol`time`avgBidPrice`avgOfferPrice`totalBidQty`totalOfferQty`bidPrice`bidQty`offerPrice`offerQty`lastPrice`highPrice`lowPrice, [STRING, TIMESTAMP,DOUBLE,DOUBLE, LONG, LONG,DOUBLE[],LONG[], DOUBLE[], LONG[],DOUBLE,DOUBLE,DOUBLE])
    return [dummyQuotationTable,quotationColMap,dummyUserOrderTable,userOrderColMap,tradeOutputTable,compositeOutputTable,snapshotOutputTable]
}

```

在以上例子代码中，

1. 登录DolphinDB客户端(`login("admin", "123456")`)并使用 `loadPlugin` 载入位于 */DolphinDBPlugin/MatchingEngineSimulator/bin/PluginMatchingEngineSimulator.txt* 的插件。
1. 使用 `go` 语句后指定行情订单类型、行情交易类型等交易要求。
1. 使用 `def` 定义了 `getMsg()`、`initArgsSnapshot`、`initArgsTick`、`initArgsSnapshotmode2` 等命名函数，并在花括号中为各个命名函数指定需要执行的语句。例如：

    `initArgsTick`返回一个包含多个表格和字典的列表，其中：
    - `dummyQuotationTable` ：报价表
    - `quotationColMap` ：字典，用于映射表格中的列名
    - `dummyUserOrderTable`、`tradeOutputTable`、`snapshotOutputTable` 分别代表了用户订单、交易输出、快照输出。

### 例子：快照模式下以模式二模拟撮合委托订单

以下这个例子展示了在快照模式下以模式二（`matchingMode` = 2）模拟撮合证券交易市场委托订单的关键过程。

```
config = dict(STRING, DOUBLE); //定义了一个包含多个配置参数的字典
config["latency"] = 0; //模拟时延
config["orderBookMatchingRatio"] = 0.12; //成交百分比
config["dataType"] = 1; // 行情类别为快照
config["outputOrderBook"] = 1; //是否输出订单，此处1表示输出
config["depth"] = 6; //匹配的订单簿的深度，区间为5到50，此处为6
config["matchingMode"] = 2; //快照模式下的匹配模式，此处为按照模式二撮合订单

name = "engine_snapshot" //指定引擎名称
exchange = "XSHE" //指定交易所名称
symbol = "AAA123" //指定证券代码

try{dropStreamEngine(name)}catch(ex){}
args = initArgsSnapshotmode2()
engine = MatchingEngineSimulator::createMatchEngine(name, exchange, config, args[0], args[1], args[2], args[3], args[4], args[5], args[6])
MatchingEngineSimulator::resetMatchEngine(engine)
appendMsg(engine, (symbol, "XSHE", 2021.01.08 09:14:01.400,23.5,,,25.0,23.0,,,,, 23.51, 23.59, 100, 100, [23.45  23.4  23.3  23.2  23.1  23.0],  [1000 1000 1000 1000 1000 1000], [23.72  23.78  23.8  23.9  24.0 24.1], [1000 1000 1000 1000 1000 1000],[23.441 23.1235 23.653 23.771],[100 100 100 100]), 1)
appendMsg(engine, (symbol, 2021.01.08 09:14:01.400, MARKET_THIS_BEST, 23.2, 200, ORDER_SEL, 1), 2)   // 23.72
appendMsg(engine, (symbol, 2021.01.08 09:14:01.400, MARKET_THIS_BEST, 23.3, 400, ORDER_SEL, 2), 2)  // 272
res = MatchingEngineSimulator::getOpenOrders(engine)
ex = table([1,2] as id,[2021.01.08 09:14:01.400,2021.01.08 09:14:01.400] as time,["AAA123","AAA123"] as symbol,[23.72,23.72] as price,[200,400] as totalQty,[200,400] as remainQty,[2,2] as BsFlag)
assert 1, each(eqObj, res.values(), ex.values())
appendMsg(engine, (symbol, 2021.01.08 09:14:01.450, LIMIT_ORDER, 23.68, 400, ORDER_SEL, 2), 2)  // 23.68
appendMsg(engine, (symbol, "XSHE", 2021.01.08 09:14:01.500,24.3,,,25.0,23.0,,,,, 23.51, 23.59, 1000, 1000, [23.45  23.4  23.3  23.2  23.1  23.0],  [1000 1000 1000 1000 1000 1000], [23.72  23.78  23.8  23.9  24.0 24.1], [1000 1000 1000 1000 1000 1000],[23.65 23.81 23.73 23.72 23.45],[100 100 100 200 100]), 1)
appendMsg(engine, (symbol, "XSHE", 2021.01.08 09:14:01.600,24.3,,,25.0,23.0,,,,, 23.51, 23.59, 1000, 1000, [23.45  23.4  23.3  23.2  23.1  23.0],  [1000 1000 1000 1000 1000 1000], [23.67  23.78  23.8  23.9  24.0 24.1], [1000 1000 1000 1000 1000 1000],[23.72 0],[1200 0]), 1)
res = select orderSysID, symbol, Direction, sendingTime,  LimitPrice, VolumeTotalOriginal,TradeTime,TradePrice,VolumeTraded,OrderStatus from args[4]
```
- 定义了一个名为`config`的字典，它包含了一些配置参数，如模拟时延、成交百分比、行情类别、是否输出订单、订单簿深度等。
- 指定了引擎名称、交易所名称和证券代码。
- 还定义了一些函数，如`appendMsg`和`getOpenOrders`，用于向引擎中添加消息和获取当前未完成的订单。
- 使用`assert`函数来验证结果是否符合预期。

其中，函数`dropStreamEngine`用于释放已创建的流数据引擎的定义，例如上述例子中的 `name = "engine_snapshot"`。

## 参考

[模拟撮合引擎应用教程](https://gitee.com/dolphindb/Tutorials_CN/blob/master/matching_engine_simulator.md)


