# mqtt 插件使用说明

DolphinDB 的 mqtt 插件可用于向 MQTT 服务器发布或订阅消息。

- [1. 编译](#1-编译)
  - [1.1 Linux上编译](#11-linux上编译)
  - [1.2 windows上编译](#12-windows上编译)
- [2. 准备](#2-准备)
  - [2.1 通过函数 loadPlugin 加载](#21-通过函数-loadplugin-加载)
  - [2.2 通过 preloadModules 参数来自动加载](#22-通过-preloadmodules-参数来自动加载)
- [3. 发布功能](#3-发布功能)
  - [3.1 连接](#31-连接)
  - [3.2 发布](#32-发布)
  - [3.3 关闭连接](#33-关闭连接)
- [4. 订阅功能](#4-订阅功能)
  - [4.1 订阅](#41-订阅)
  - [4.2 查询订阅](#42-查询订阅)
  - [4.3 取消订阅](#43-取消订阅)
- [5. 打/解包功能](#5-打解包功能)
  - [5.1 createCsvFormatter](#51-createcsvformatter)
  - [5.2 createCsvParser](#52-createcsvparser)
  - [5.3 createJsonFormatter](#53-createjsonformatter)
  - [5.4 createJsonParser](#54-createjsonparser)
- [6. 一个完整的例子](#6-一个完整的例子)

mqtt插件目前支持版本：[relsease200](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/mqtt/README_CN.md), [release130](https://github.com/dolphindb/DolphinDBPlugin/blob/release130/mqtt/README_CN.md), [relsease120](https://github.com/dolphindb/DolphinDBPlugin/blob/release120/mqtt/README_CN.md), [release110](https://github.com/dolphindb/DolphinDBPlugin/blob/release110/mqtt/README_CN.md)。您当前查看的插件版本为release200，请使用DolphinDB 2.00.X版本server。若使用其它版本server，请切换至相应插件分支。

## 1. 编译
### 1.1 Linux上编译

* 以下步骤在64位 Linux GCC version 5.4.0 下编译测试通过。
* 在编译前需要先安装 [git](https://git-scm.com/) 和 [CMake](https://cmake.org/)。

Ubuntu 用户只需要在命令行输入以下命令即可：

```bash
$ sudo apt-get install git cmake
```

* 在 mqtt 目录下创建 build 目录，进入后运行 `cmake ..` 和 `make`，即可编译生成 'libPluginMQTTClient.so'。

```
mkdir build
cd build
cmake ..
make
```
### 1.2 windows上编译

因为需要通过 CMake 和 MinGW 编译，所以需要先安装 [CMake](https://cmake.org/)和 [MinGW](http://www.mingw.org/) 环境，目前在64位win10上用 MinGW-W64-builds-4.3.3 版本编译通过。把 MinGW 和 CMake 的 bin 目录加入 Windows 系统 Path 路径。 

```
    git clone https://github.com/dolphindb/DolphinDBPlugin.git
    cd DolphinDBPlugin/mqtt
    mkdir build
    cd build
    cmake ..
    copy /YOURPATH/libDolphinDB.dll . 
    make
```

**注意：** 如果需要指定特定的 MinGW 路径，请在 CmakeList.txt 中修改以下语句。

```
    set(MINGW32_LOCATION C://MinGW/MinGW/)  
```
编译之后目录下会产生 libPluginMQTTClient.dll 文件，然后按预编译安装方法导入并加载。

## 2. 准备

DolphinDB 提供两种方式加载插件，二者选一种即可：

* 通过函数 loadPlugin 加载；
* 通过配置参数 preloadModules 预加载。

### 2.1 通过函数 loadPlugin 加载

需要如下例所示先加载插件。用户需要根据具体情况修改其中的路径。

```
loadPlugin("/YOUR_PATH/mqtt/PluginMQTTClient.txt"); 
```

注意：若插件运行于 Linux 系统，则可以指定绝对或相对路径；若插件运行于 Window 系统，则必须指定绝对路径，且路径必须使用"\\\\"或"/"代替"\\"。

### 2.2 通过 preloadModules 参数来自动加载

通过配置参数 preloadModules 预加载插件，server 在启动时会自动加载插件，用户无需再通过 loadPlugin 进行加载。对单机版服务，该参数在 dolphindb.cfg 中配置。对集群版，需要为 controller 和 datanode 加载相同的插件。最简单的方法是在 controller.cfg 和 cluster.cfg 中配置 preloadModules 参数。

配置方法如下，多个插件用逗号分离：
```
preloadModules=plugins::mqtt,plugins::odbc
```

注意：

* 1.20.x 以上版本 server 才支持设置 preloadModules。
* preloadModules 用于指定预加载的模块类型（plugins 或 modules），插件文件路径需要通过 pluginDir 配置。详情参考[配置项说明](https://www.dolphindb.cn/cn/help/DatabaseandDistributedComputing/Configuration/StandaloneMode.html)。

## 3. 发布功能
### 3.1 连接

```
mqtt::connect(host, port,[QoS=0],[formatter],[batchSize=0],[username],[password])
```
建立一个与 MQTT server/broker 的连接。返回一个 connection。可以显式的调用 `close` 函数去关闭，也可以在 reference count 为0时自动释放。

**参数**

- 'host' 是一个字符串，表示 MQTT server/broker 的 IP 地址。
- 'port' 是一个整数，表示 MQTT server/broker 的端口号。
- 'QoS' 表示是一个整数，表示消息发布服务质量。0：至多一次；1：至少一次；2：只有一次。它是可选参数，默认是0。
- 'formatter' 是一个函数，用于对发布的数据按 CSV 或 JSON 格式进行打包。目前支持的函数由 `createJsonFormatter` 或 `createCsvFormatter` 创建。
- 'batchSize' 是一个整数。当待发布内容是一个表时，可以分批发送，batchSize 表示每次发送的记录行数。
- 'username' 是一个字符串，用于登录 MQTT server/broker 的用户名。
- 'password' 是一个字符串，用于登录 MQTT server/broker 的密码。

**例子**

```
f=mqtt::createJsonFormatter()
conn=connect("test.mosquitto.org",1883,0,f,50)
```

### 3.2 发布

```
mqtt::publish(conn,topic,obj)
```
向 MQTT server/broker 发布消息。

**参数**

- 'conn' 是 `connect` 函数返回的值。
- 'topic' 是一个字符串，表示主题。
- 'obj' 是表或字符串或字符串数组，表示待发布的消息内容。

**例子**

```
mqtt::publish(conn,"dolphindb/topic1","welcome")
mqtt::publish(conn,"devStr/sensor001",["hello world","welcome"])
t=table(take(0..99,50) as hardwareId ,take(now(),
		50) as ts,rand(20..41,50) as temperature,
		rand(50,50) as humidity,rand(500..1000,
		50) as voltage)
mqtt::publish(conn,"dolphindb/device",t)		
``` 

### 3.3 关闭连接
```
mqtt::close(conn)
```

断开与 MQTT server/broker 的连接。

**参数**

- 'conn' 是 connect 函数返回的值。

**例子**

```
mqtt::close(conn)
```

## 4. 订阅功能

### 4.1 订阅

```
mqtt::subscribe(host, port, topic, [parser], handler,[username],[password])
```

向 MQTT server/broker 订阅消息。返回一个连接。

**参数**

- 'host' 是一个字符串，表示 MQTT server/broker 的 IP 地址。
- 'port' 是一个整数，表示 MQTT server/broker 的端口号。
- 'topic' 是一个字符串，表示订阅主题。
- 'parser' 是一个函数，用于对订阅的消息按 CSV 或 JSON 格式进行解析，目前支持的函数由 createJsonParser 或 createCsvParser 创建。
- 'handler' 是一个函数或表，用于处理从 MQTT server/broker 接收的消息。
- 'username' 是一个字符串，用于登录 MQTT server/broker 的用户名。
- 'password' 是一个字符串，用于登录 MQTT server/broker 的密码。

**例子**

```
p = createCsvParser([INT, TIMESTAMP, DOUBLE, DOUBLE,DOUBLE], ',', ';' )
sensorInfoTable = table( 10000:0,`deviceID`send_time`temperature`humidity`voltage ,[INT, TIMESTAMP, DOUBLE, DOUBLE,DOUBLE])
conn = mqtt::subscribe("192.168.1.201",1883,"sensor/#",p,sensorInfoTable)
```

### 4.2 查询订阅

```
mqtt::getSubscriberStat()    
```

查询所有订阅信息。返回的结果是一个包含7列的表，分别是："subscriptionId" 表示订阅标识符；"user" 表示建立订阅的会话用户; "host" 表示 MQTT server/broker 的 IP； "port" 表示 MQTT server/broker 的端口号；"topic" 表示订阅主题；"createTimestamp" 表示可以订阅建立时间；"receivedPackets" 表示订阅收到的消息报文数。

**参数**

无。

**例子**

```
mqtt::getSubscriberStat()    
```

### 4.3 取消订阅

```
mqtt::unsubscribe(subscription)    
```

取消订阅MQTT server/broker。

**参数**

- 'subscription' 是 `subscribe` 函数返回的值或 `getSubscriberStat` 返回的订阅标识符。

**例子**

```
mqtt::unsubscribe(sub1) 
mqtt::unsubscribe("350555232l")   
```

## 5. 打/解包功能

### 5.1 createCsvFormatter

```
mqtt::createCsvFormatter([format], [delimiter=','], [rowDelimiter=';'])
```
创建一个 CSV 格式的 Formatter 函数。

**参数**

- 'format' 是一个字符串向量。
- 'delimiter' 是列之间的分隔符，默认是','。
- 'rowDelimiter' 是行之间的分隔符，默认是';'。

**例子**

```
def createT(n) {
    return table(take([false, true], n) as bool, take('a'..'z', n) as char, take(short(-5..5), n) as short, take(-5..5, n) as int, take(-5..5, n) as long, take(2001.01.01..2010.01.01, n) as date, take(2001.01M..2010.01M, n) as month, take(time(now()), n) as time, take(minute(now()), n) as minute, take(second(now()), n) as second, take(datetime(now()), n) as datetime, take(now(), n) as timestamp, take(nanotime(now()), n) as nanotime, take(nanotimestamp(now()), n) as nanotimestamp, take(3.1415, n) as float, take(3.1415, n) as double, take(`AAPL`IBM, n) as string, take(`AAPL`IBM, n) as symbol)
}
t = createT(100)
MyFormat = take("", 18)
MyFormat[2] = "0.000"
MyFormat[5] = "yyyy.MM.dd"
f = mqtt::createCsvFormatter(MyFormat)
f(t)
```

### 5.2 createCsvParser

```
mqtt::createCsvParser(schema, [delimiter=','], [rowDelimiter=';'])
```
该函数创建一个 CSV 格式的 Parser 函数。

**参数**

- 'schema' 是一个列的数据类型的向量。
- 'delimiter' 是列之间的分隔符，默认是','。
- 'rowDelimiter' 是行之间的分隔符，默认是';'。

**例子**

```
def createT(n) {
    return table(take([false, true], n) as bool, take('a'..'z', n) as char, take(short(-5..5), n) as short, take(-5..5, n) as int, take(-5..5, n) as long, take(2001.01.01..2010.01.01, n) as date, take(2001.01M..2010.01M, n) as month, take(time(now()), n) as time, take(minute(now()), n) as minute, take(second(now()), n) as second, take(datetime(now()), n) as datetime, take(now(), n) as timestamp, take(nanotime(now()), n) as nanotime, take(nanotimestamp(now()), n) as nanotimestamp, take(3.1415, n) as float, take(3.1415, n) as double, take(`AAPL`IBM, n) as string, take(`AAPL`IBM, n) as symbol)
}
t = createT(100)
f = mqtt::createCsvFormatter([BOOL,CHAR,SHORT,INT,LONG,DATE,MONTH,TIME,MINUTE,SECOND,DATETIME,TIMESTAMP,NANOTIME,NANOTIMESTAMP,FLOAT,DOUBLE,STRING,SYMBOL])
s=f(t)
p = mqtt::createCsvParser([BOOL,CHAR,SHORT,INT,LONG,DATE,MONTH,TIME,MINUTE,SECOND,DATETIME,TIMESTAMP,NANOTIME,NANOTIMESTAMP,FLOAT,DOUBLE,STRING,SYMBOL])
p(s)
```

### 5.3 createJsonFormatter

```
mqtt::createJsonFormatter()
```
该函数创建一个 JSON 格式的 Formatter 函数。

**参数**

无。

**例子**

```
def createT(n) {
    return table(take([false, true], n) as bool, take('a'..'z', n) as char, take(short(-5..5), n) as short, take(-5..5, n) as int, take(-5..5, n) as long, take(2001.01.01..2010.01.01, n) as date, take(2001.01M..2010.01M, n) as month, take(time(now()), n) as time, take(minute(now()), n) as minute, take(second(now()), n) as second, take(datetime(now()), n) as datetime, take(now(), n) as timestamp, take(nanotime(now()), n) as nanotime, take(nanotimestamp(now()), n) as nanotimestamp, take(3.1415, n) as float, take(3.1415, n) as double, take(`AAPL`IBM, n) as string, take(`AAPL`IBM, n) as symbol)
}
t = createT(100)
f = mqtt::createJsonFormatter()
f(t)
```
### 5.4 createJsonParser

```
mqtt::createJsonParser(schema, colNames)
```
该函数创建一个 JSON 格式的 Parser 函数。

**参数**

- 'schema' 是一个向量，表示列的数据类型。
- 'colNames' 是一个向量，表示列名。

**例子**

```
def createT(n) {
    return table(take([false, true], n) as bool, take('a'..'z', n) as char, take(short(-5..5), n) as short, take(-5..5, n) as int, take(-5..5, n) as long, take(2001.01.01..2010.01.01, n) as date, take(2001.01M..2010.01M, n) as month, take(time(now()), n) as time, take(minute(now()), n) as minute, take(second(now()), n) as second, take(datetime(now()), n) as datetime, take(now(), n) as timestamp, take(nanotime(now()), n) as nanotime, take(nanotimestamp(now()), n) as nanotimestamp, take(3.1415, n) as float, take(3.1415, n) as double, take(`AAPL`IBM, n) as string, take(`AAPL`IBM, n) as symbol)
}
t = createT(100)
f = mqtt::createJsonFormatter()
p = createJsonParser([BOOL,CHAR,SHORT,INT,LONG,DATE,MONTH,TIME,MINUTE,SECOND,DATETIME,TIMESTAMP,NANOTIME,NANOTIMESTAMP,FLOAT,DOUBLE,STRING,SYMBOL],
`bool`char`short`int`long`date`month`time`minute`second`datetime`timestamp`nanotime`nanotimestamp`float`double`string`symbol)
s=f(t)
x=p(s)

```

## 6. 一个完整的例子
```
loadPlugin("./plugins/mqtt/bin/PluginMQTTClient.txt"); 
use mqtt; 

//***************************publish a table****************************************//
MyFormat = take("", 5)
MyFormat[2] = "0.000"
f = createCsvFormatter(MyFormat, ',', ';')

//create a record for every device
def writeData(hardwareVector){
	hardwareNumber = size(hardwareVector)
	return table(take(hardwareVector,hardwareNumber) as hardwareId ,take(now(),
		hardwareNumber) as ts,rand(20..41,hardwareNumber) as temperature,
		rand(50,hardwareNumber) as humidity,rand(500..1000,
		hardwareNumber) as voltage)
}
def publishTableData(server,topic,iterations,hardwareVector,interval,f){
    conn=connect(server,1883,0,f,100)
    for(i in 0:iterations){
	   t=writeData(hardwareVector)
	   publish(conn,topic,t)
	   sleep(interval)
    }
    close(conn)
         
}
host="192.168.1.201"
submitJob("submit_pub1", "submit_p1", publishTableData{host,"sensor/s001",10,100..149,100,f})
publishTableData(host,"sensor/s001",100,0..99,100,f)


//*******************************subscribe : handler is a table************************************************//
p = createCsvParser([INT, TIMESTAMP, DOUBLE, DOUBLE,DOUBLE], ',', ';' )
sensorInfoTable = table( 10000:0,`deviceID`send_time`temperature`humidity`voltage ,[INT, TIMESTAMP, DOUBLE, DOUBLE,DOUBLE])
conn = mqtt::subscribe("192.168.1.201",1883,"sensor/#",p,sensorInfoTable)
```
# ReleaseNotes

## 故障修复

* 当 MQTT 服务器关闭时，通过 mqtt::connect 进行连接将收到错误提示。（**2.00.10**）
* 优化了 connect 函数中关于参数 batchsize 默认值的报错信息。（**2.00.10**）
* 修复了当 mqtt::publish, mqtt::createCsvFormatter 参数的输入值为 NULL 时可能出现的宕机或卡住的问题。（**2.00.10**）
* 修复了若发布消息中包含空值，订阅端无法接收到数据的问题。（**2.00.10**）
* 修复了若发布数据中包含类型为 CHAR, MONTH 数据时，订阅端会接收到错误类型数据的问题。（**2.00.10**）

# 功能优化

* 优化了部分场景下的报错信息。（**2.00.10**）
