# MQTT Client Plugin

The DolphinDB MQTT Client plugin has different branches, such as release200 and release130. Each branch corresponds to a DolphinDB server version. Please make sure you are in the correct branch of the plugin documentation.

- [MQTT Client Plugin](#mqtt-client-plugin)
  - [1. Load Precompiled Plugin](#1-load-precompiled-plugin)
    - [1.1 Function `loadPlugin`](#11-function-loadplugin)
    - [1.2 Configuration Parameter *preloadModules*](#12-configuration-parameter-preloadmodules)
  - [2. (Optional) Manually Compile Plugin](#2-optional-manually-compile-plugin)
    - [2.1 Linux](#21-linux)
    - [2.2 Windows](#22-windows)
  - [3. Publish](#3-publish)
    - [3.1 Connect to a MQTT server/broker](#31-connect-to-a-mqtt-serverbroker)
    - [3.2 Publish](#32-publish)
    - [3.3 Simplify the release process](#33-simplify-the-release-process)
    - [3.4 Close the connection](#34-close-the-connection)
  - [4. Subscribe/Unsubscribe](#4-subscribeunsubscribe)
    - [4.1 Subscribe](#41-subscribe)
    - [4.2 Check Subscription](#42-check-subscription)
    - [4.3 Unsubscribe](#43-unsubscribe)
  - [5. Formatter/Parser](#5-formatterparser)
    - [5.1 createCsvFormatter](#51-createcsvformatter)
    - [5.2 createCsvParser](#52-createcsvparser)
    - [5.3 createJsonFormatter](#53-createjsonformatter)
    - [5.4 createJsonParser](#54-createjsonparser)
  - [6. An example](#6-an-example)


## 1. Load Precompiled Plugin

There are two options to load the MQTT client plugin in DolphinDB:

* Using the function `loadPlugin`
* Specifying the configuration parameter *preloadModules*

### 1.1 Function `loadPlugin` 

```
loadPlugin("/YOUR_PATH/mqtt/PluginMQTTClient.txt"); 
```

Note: You can modify the path as appropriate. With a Windows OS, make sure to specify an absolute path and replace "\\" with "\\\\" or "/".

### 1.2 Configuration Parameter *preloadModules*

Alternatively, you can preload the plugin during server startup by specifying the configuration parameter *preloadModules*, so you don't have to call `loadPlugin` afterwards. For single-machine deployment, configure the parameter in *dolphindb.cfg*. For cluster deployment, the plugin must be loaded on both the controller and the associated data node(s), and the simplest way is to configure the parameter in both *controller.cfg* and *cluster.cfg*.

```
preloadModules=plugins::mqtt,plugins::odbc
```

Separate plugins with a comma.

Note:

* The configuration parameter *preloadModules* is only supported on server version 1.20.x and higher.
* *preloadModules* is used to preload plugins and modules in DolphinDB. The directory for the plugin files are specified by the configuration parameter *pluginDir*. For more information, see [documentation](https://dolphindb.com/help200/DatabaseandDistributedComputing/Configuration/StandaloneMode.html).

## 2. (Optional) Manually Compile Plugin
### 2.1 Linux

* This plugin has been successfully compiled with GCC (version 5.4.0) on 64-bits Linux operating system.
* Before compiling, install [git](https://git-scm.com/) and [CMake](https://cmake.org/).

For Ubuntu users, just type

```bash
$ sudo apt-get install git cmake
```
* Create a 'build' directory. Under the directory, run `cmake ..` and `make` to generate 'libPluginMQTTClient.so'.

```
mkdir build
cd build
cmake ..
make
```

### 2.2 Windows
This plugin has been successfully compiled with MinGW-W64-builds-4.3.3 on 64-bits Windows operating system. Install [cmake](https://cmake.org/) and [MinGW](http://www.mingw.org/) on your machine. Add the "bin" directories of MinGW and cmake to your PATH on Windows.

```
    git clone https://github.com/dolphindb/DolphinDBPlugin.git
    cd DolphinDBPlugin/mqtt
    mkdir build
    cd build
    cmake ..
    copy /YOURPATH/libDolphinDB.dll . 
    make
```

**Note:** To specify a different path for MinGW, modify the following line in *CmakeList.txt*:

```
    set(MINGW32_LOCATION C://MinGW/MinGW/)  
```

The *libPluginMQTTClient.dll* file is generated after compilation. Next, refer to the procedure described in Section 1 "Load Precompiled Plugin" to load the compiled plugin. 

## 3. Publish

### 3.1 Connect to a MQTT server/broker

**Syntax**

```
mqtt::connect(host, port, [QoS=0], [formatter], [batchSize=0], [username], [password], [sendbufSize=40960])
```
The function connect to a MQTT server/broker. It returns a connection object which can be explicitly called to close with the `close` function, or it can be automatically released when the reference count is 0.

**Arguments**

- 'host' is a string indicating the IP address of MQTT server/broker.

- 'port' is an integer indicating the port number of MQTT server/broker.

- 'Qos' is an integer indicating the quality of service. 0: at most once; 1: at least once; 2: only once. It is optional and the default value is 0.

- 'formatter' is a function used to package published data in a format. Currently supported functions are ``createJsonFormatter`` and ``createCsvFormatter``.

- 'batchSize' is an integer. When the content to be published is a table, it can be sent in batches, and *batchSize* indicates the number of rows sent each time.

- 'username' and 'password' are user credentials to the MQTT server/broker.

- 'sendbufSize' is an integer that defaults to 40960 when omitted, and it is used to specify the size of the send buffer.

**Example**
```
f=mqtt::createJsonFormatter()
conn=connect("test.mosquitto.org",1883,0,f,50)
```

### 3.2 Publish

**Syntax**

```
mqtt::publish(conn,topic,obj)
```

This function posts one or more messages to the MQTT server/broker

**Arguments**

- 'conn' is an object generated by function

- 'topic' is a string indicating the subscription topic.

- 'obj' is the content of the message to be published, which can be a table or a string or an array of strings.


**Example**

```
mqtt::publish(conn,"dolphindb/topic1","welcome")
mqtt::publish(conn,"devStr/sensor001",["hello world","welcome"])
t=table(take(0..99,50) as hardwareId ,take(now(),
		50) as ts,rand(20..41,50) as temperature,
		rand(50,50) as humidity,rand(500..1000,
		50) as voltage)
mqtt::publish(conn,"dolphindb/device",t)		

``` 

### 3.3 Simplify the release process

```
mqtt::createPublisher(conn,topic,colNames,colTypes)
```
Create an object that can publish messages by writing data to it

**参数**

- 'conn' is an object generated by function connect
- 'topic' is a topic string
- 'colNames' is an array of column names in the structure of the published table
- 'colTypes' s an array of column types in the structure of the published table

**例子**

```
MyFormat = take("", 3)
MyFormat[2] = "0.000"
f = createCsvFormatter(MyFormat, ',', ';')
pubConn = connect("127.0.0.1",1883,0,f,100)

colNames = [`ts, `hardwareId, `val]
colTypes = [TIMESTAMP, SYMBOL, INT]
publisher = createPublisher(pubConn, "sensor/s001", colNames, colTypes)

// example 1: by appending data to 'publisher' to publish it, the 'tb' here must be a table
append!(publisher, tb)

// example 2: by using insert SQL sentence
insert into publisher values([2023.08.25 10:57:47.961, 2023.08.25 10:57:47.961], symbol([`bb,`cc]), [22,33])

// example 3: by setting handler=append!{publisher} when subscribing a streaming table
share streamTable(1000:0, `time`sym`val, [TIMESTAMP, SYMBOL, INT]) as trades
subscribeTable(tableName="trades", actionName="engine1", offset=0, handler=append!{publisher}, msgAsTable=true);

insert into trades values(2018.10.08T01:01:01.785,`dd,44)
insert into trades values(2018.10.08T01:01:02.125,`ee,55)
insert into trades values(2018.10.08T01:01:10.263,`ff,66)
``` 

### 3.4 Close the connection

**Syntax**

```
mqtt::close(conn)
```

This function disconnects from the server/broker.

**Arguments**

- 'conn' is an object generated by function.

**Example**
```
mqtt::close(conn)
```

## 4. Subscribe/Unsubscribe

### 4.1 Subscribe

**Syntax**

```
mqtt::subscribe(host, port, topic, [parser], handler, [username], [password], [recvbufSize=20480])
```

**Arguments**

- 'host' is a string indicating the IP address of MQTT server/broker.

- 'port' is an integer indicating the port number of MQTT server/broker.

- 'topic' is a string indicating the subscription topic.

- 'parser' is a function for parsing subscribed messages. Currently supported functions are createJsonParser and createCsvParser

- 'handler' is a function or a table to process the subscribed data.

- 'username' and 'password' are strings indicating user credentials to the MQTT server/broker.

- 'recvbufSize' is an integer that defaults to 20480 when omitted, and it is used to specify the size of the receive buffer.

**Details**

Subscribe to a MQTT server/broker. It returns a connection object.

**Example**

```
p = createCsvParser([INT, TIMESTAMP, DOUBLE, DOUBLE,DOUBLE], ',', ';' )
sensorInfoTable = table( 10000:0,`deviceID`send_time`temperature`humidity`voltage,[INT, TIMESTAMP, DOUBLE, DOUBLE,DOUBLE])
conn = mqtt::subscribe("192.168.1.201",1883,"sensor/#",p,sensorInfoTable)
```

### 4.2 Check Subscription

```
mqtt::getSubscriberStat()    
```
Get the information on all subscriptions. Return a table with the following columns:

- "subscriptionId" - ID of a subscription
- "user" - the session user who created the subscription
- "host" - IP address for the MQTT server/broker
- "port" - Port number of the MQTT server/broker
- "topic" - the subscription topic
- "createTimestamp" - the time when the subscription was created
- "receivedPackets" - the number of messages received

### 4.3 Unsubscribe

**Syntax**

```
mqtt::unsubcribe(subscription)  
```
Cancel subscription to the MQTT server/broker.

**Arguments**

- 'subscription' is the value returned by the `subscribe` function or the subscription ID returned by `getSubscriberStat`.

**Example**

```
mqtt::unsubscribe(sub1) 
mqtt::unsubscribe("350555232l")    
```

## 5. Formatter/Parser

### 5.1 createCsvFormatter

**Syntax**

```
mqtt::createCsvFormatter([format], [delimiter=','], [rowDelimiter=';'])
```
This function creates a Formatter function in CSV format.

**Arguments**

- 'format'is a string array。
- 'delimiter'is the separator between columns, the default is ','
- 'rowDelimiter' is the separator between the lines, the default is ';'

The return value is a function.

**Example**
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

**Syntax**
```
mqtt::createCsvParser(schema, [delimiter=','], [rowDelimiter=';'])
```
This function creates a Parser function in CSV format.

**Arguments**

- 'schema' is an array of column data types
- 'delimiter'is the separator between columns, the default is ','
- 'rowDelimiter' is the separator between the lines, the default is ';'


**Example**
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

**Syntax**
```
mqtt::createJsonFormatter()
```
This function creates a Formatter function in JSON format

**Arguments**
    None

The return value is a function.

**Example**
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
This function creates a Parser function in JSON format.

**Arguments**
- 'schema' is a vector of data types for all columns.
- 'colNames' is a column name vector

**Example**
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

## 6. An example
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
