## 1. 编译
### 1.1 Linux上编译

* 以下步骤在64位Linux GCC version 5.4.0下编译测试通过。
* 在编译前需要先安装 [git](https://git-scm.com/) 和 [CMake](https://cmake.org/)。

Ubuntu用户只需要在命令行输入以下命令即可：

```bash
$ sudo apt-get install git cmake
```

* 在mqtt目录下创建build目录，进入后运行`cmake ..`和`make`，即可编译生成'libPluginMQTTClient.so'。

```
mkdir build
cd build
cmake ..
make
```
### 1.2 windows上编译
通过cmake和MinGW编译。因此需要先安装[cmake](https://cmake.org/)和[MinGW](http://www.mingw.org/)环境，目前在64位win10上用MinGW-W64-builds-4.3.3版本编译通过。把MinGW和cmake的bin目录加入Windows系统Path路径。 

```
    git clone https://github.com/dolphindb/DolphinDBPlugin.git
    cd DolphinDBPlugin/mqtt
    mkdir build
    cd build
    cmake ..
    copy /YOURPATH/libDolphinDB.dll . 
    make
```

**注意：** 如果需要指定特定的MingW路径，请在CmakeList.txt中修改以下语句。

```
    set(MINGW32_LOCATION C://MinGW/MinGW/)  
```
编译之后目录下会产生libPluginMQTTClient.dll文件，然后按预编译安装方法导入并加载。


## 2. 准备

### 2.1 通过函数loadPlugin加载

需要如下例所示先加载插件。用户需要根据具体情况修改其中的路径。请注意，若使用 Windows 插件，加载时必须指定绝对路径，且路径中使用"\\\\"或"/"代替"\"。

```
loadPlugin("/YOUR_PATH/mqtt/PluginMQTTClient.txt"); 
```

### 2.2 1.20.x 版本后通过 preloadModules参数来自动加载

前提是server的版本>=1.20; 需要预先加载的插件存在。否则sever启动的时候会有异常。多个插件用逗号分离。
```
preloadModules=plugins::mqtt,plugins::odbc
```


## 3. 发布功能

### 3.1 连接

```
mqtt::connect(host, port,[QoS=0],[formatter],[batchSize=0],[username],[password])
```
建立一个与MQTT server/broker的连接。返回一个connection。可以显式的调用`close`函数去关闭，也可以在reference count为0的时候自动释放。

参数：
- 'host'是一个字符串，表示MQTT server/broker的IP地址。
- 'port'是一个整数，表示MQTT server/broker的端口号。
- 'QoS'表示是一个整数，表示消息发布服务质量。0：至多一次；1：至少一次；2：只有一次。它是可选参数，默认是0。
- 'formatter'是一个函数，用于对发布的数据按CSV或JSON格式进行打包。目前支持的函数由`createJsonFormatter`或`createCsvFormatter`创建。
- 'batchSize'是一个整数。当待发布内容是一个表时，可以分批发送，batchSize表示每次发送的记录行数。
- 'username'是一个字符串，用于登录MQTT server/broker的用户名。
- 'password'是一个字符串，用于登录MQTT server/broker的密码。

例子：
```
f=mqtt::createJsonFormatter()
conn=connect("test.mosquitto.org",1883,0,f,50)
```

### 3.2 发布

```
mqtt::publish(conn,topic,obj)
```
向MQTT server/broker发布消息。

参数：
- 'conn'是`connect`函数返回的值。
- 'topic'是一个字符串，表示主题。
- 'obj'是表或字符串或字符串数组，表示待发布的消息内容。

例子：
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

断开与MQTT server/broker的连接。

参数：
- 'conn'是connect函数返回的值。

例子：
```
mqtt::close(conn)
```

## 4. 订阅功能

### 4.1 订阅

```
mqtt::subscribe(host, port, topic, [parser], handler,[username],[password])
```

向MQTT server/broker订阅消息。返回一个连接。

参数：
- 'host'是一个字符串，表示MQTT server/broker的IP地址。
- 'port'是一个整数，表示MQTT server/broker的端口号。
- 'topic'是一个字符串，表示订阅主题。
- 'parser'是一个函数，用于对订阅的消息按CSV或JSON格式进行解析，目前支持的函数由createJsonParser或createCsvParser创建。
- 'handler'是一个函数或表，用于处理从MQTT server/broker接收的消息。
- 'username'是一个字符串，用于登录MQTT server/broker的用户名。
- 'password'是一个字符串，用于登录MQTT server/broker的密码。

例子：

```
p = createCsvParser([INT, TIMESTAMP, DOUBLE, DOUBLE,DOUBLE], ',', ';' )
sensorInfoTable = table( 10000:0,`deviceID`send_time`temperature`humidity`voltage ,[INT, TIMESTAMP, DOUBLE, DOUBLE,DOUBLE])
conn = mqtt::subscribe("192.168.1.201",1883,"sensor/#",p,sensorInfoTable)
```

### 4.2 查询订阅

```
mqtt::getSubscriberStat()    
```

查询所有订阅信息。返回的结果是一个包含7列的表，分别是："subscriptionId", 表示订阅标识符；"user",表示建立订阅的会话用户; "host", 表示MQTT server/broker的IP； "port", 表示MQTT server/broker的端口号； "topic", 表示订阅主题； "createTimestamp"， 表示可以订阅建立时间；"receivedPackets",表示订阅收到的消息报文数。

例子：

```
mqtt::getSubscriberStat()    
```

### 4.3 取消订阅

```
mqtt::unsubscribe(subscription)    
```

取消订阅MQTT server/broker。

参数：

- 'subscription'是`subscribe`函数返回的值或`getSubscriberStat`返回的订阅标识符。

例子：

```
mqtt::unsubscribe(sub1) 
mqtt::unsubscribe("350555232l")   
```

## 5. 打/解包功能

### 5.1 createCsvFormatter

```
mqtt::createCsvFormatter([format], [delimiter=','], [rowDelimiter=';'])
```
创建一个CSV格式的Formatter函数。

参数：
- 'format' 是一个string向量。
- 'delimiter'是列之间的分隔符，默认是','。
- 'rowDelimiter'是行之间的分隔符，默认是';'。

例子：
```
def createT(n) {
    return table(take([false, true], n) as bool, take('a'..'z', n) as char, take(short(-5..5), n) as short, take(-5..5, n) as int, take(-5..5, n) as long, take(2001.01.01..2010.01.01, n) as date, take(2001.01M..2010.01M, n) as month, take(time(now()), n) as time, take(minute(now()), n) as minute, take(second(now()), n) as second, take(datetime(now()), n) as datetime, take(now(), n) as timestamp, take(nanotime(now()), n) as nanotime, take(nanotimestamp(now()), n) as nanotimestamp, take(3.1415, n) as float, take(3.1415, n) as double, take(`AAPL`IBM, n) as string, take(`AAPL`IBM, n) as symbol)
}
t = createT(100)
f = mqtt::createCsvFormatter([BOOL,CHAR,SHORT,INT,LONG,DATE,MONTH,TIME,MINUTE,SECOND,DATETIME,TIMESTAMP,NANOTIME,NANOTIMESTAMP,FLOAT,DOUBLE,STRING,SYMBOL])
f(t)
```

### 5.2 createCsvParser

```
mqtt::createCsvParser(schema, [delimiter=','], [rowDelimiter=';'])
```
该函数创建一个CSV格式的Parser函数。

参数：
- 'schema' 是一个列的数据类型的向量。
- 'delimiter'是列之间的分隔符，默认是','。
- 'rowDelimiter'是行之间的分隔符，默认是';'。

例子：
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
该函数创建一个JSON格式的Formatter函数。

参数：无。

例子：
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
该函数创建一个JSON格式的Parser函数。

参数：
- 'schema' 是一个向量，表示列的数据类型。
- 'colNames' 是一个向量，表示列名。

例子：
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