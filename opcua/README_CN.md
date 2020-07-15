## 1. 安装构建

### 环境准备
#### Ubuntu
**Note:** [cmake](https://cmake.org/) 是一个流行的项目构建工具,可以帮你轻松的解决第三方依赖的问题。

安装cmake
```
sudo apt-get install cmake
```

安装mbedtls
```
sudo apt-get install libmbedtls-dev
``` 

可以使用我们预先编译的1.0.0版本的`libopen62541.so`文件，在./lib/ubuntu目录下执行以下操作：
```
ln -s libopen62541.so.0 libopen62541.so
```

也可以自己编译，在open62541的[官方网站](https://open62541.org/)下载源代码，注意版本须为1.0。按照[open62541帮助文档](https://github.com/open62541/open62541/blob/1.0/doc/building.rst) 编译，在编译时启用`-DUA_ENABLE_SUBSCRIPTIONS=ON`, `-DBUILD_SHARED_LIBS=ON`, `-DUA_ENABLE_ENCRYPTION=ON` 参数。
```
cd open62541
mkdir build
cd build
cmake .. -DUA_ENABLE_SUBSCRIPTIONS=ON -DBUILD_SHARED_LIBS=ON -DUA_ENABLE_ENCRYPTION=ON
make
```
将open62541/build/bin目录下的文件复制到./lib/ubuntu目录下。

#### Centos

安装mbedtls
```
yum install mbedtls-devel
``` 

在open62541的[官方网站](https://open62541.org/)下载源代码，注意版本为1.0。按照[open62541帮助文档](https://github.com/open62541/open62541/blob/1.0/doc/building.rst)  编译，在编译时启用`-DUA_ENABLE_SUBSCRIPTIONS=ON`, `-DBUILD_SHARED_LIBS=ON`, `-DUA_ENABLE_ENCRYPTION=ON` 参数
```
cd open62541
mkdir build
cd build
cmake .. -DUA_ENABLE_SUBSCRIPTIONS=ON -DBUILD_SHARED_LIBS=ON -DUA_ENABLE_ENCRYPTION=ON
make
```
将open62541/build/bin目录下的文件复制到./lib/centos目录下。

### 使用cmake构建libPluginOPCUA

修改CMakeLists.txt中"link_directories(./lib/centos)"，确保相应操作系统的libopen62541在gcc可搜索的路径中。修改CMakeLists.txt中"link_directories(path to libDolphindb.so)",确保libDolphinDB.so在gcc可搜索的路径中，或者使用`LD_LIBRARY_PATH`指定其路径。
```
mkdir build
cd build
cmake ../path_to_OPCUA_plugin/
make
```

## 2. API

目前不支持异步读写。

### 2.1 获取OPC Server

语法
```
opcua::getOpcServerList(serverUrl)
```
参数
- `serverUrl`是字符串，表示server地址，如: opc.tcp://LOCALHOST:4840/。

详情

获取OPC server。返回的结果是一个包含四列的表，分别是："ServerUri", 表示server的标识符； "ServerName", 表示server的名字； "ProductUri", 表示server的product标识符； "Type", 表示server的类型； "DiscoveryUrl"， 表示可以用来连接server的url， 如果有多个，用'; '分割。

例子
```
opcua::getOpcServerList("opc.tcp://LOCALHOST:4840/");
```

### 2.2 获取OPC Server Endpoints

语法
```
opcua::getOpcEndPointList(serverUrl)
```
参数
- `serverUrl`是字符串，表示server地址，如: opc.tcp://LOCALHOST:4840/。

详情

获取OPC server。返回的结果是一个包含四列的表，分别是"endpointUrl", 表示server可以用来连接的端点的url； "transportProfileUri", 表示该端点传输配置文件的标识符； "securityMode", 表示该端点的安全模式； "securityPolicyUri", 表示该端点安全策略的标识符； "securityLevel"， 表示该端点安全等级。

例子
```
opcua::getOpcEndPointList("opc.tcp://LOCALHOST:4840/");
```

### 2.3 连接

语法
```
opcua::connect(endPointUrl,clientUri,[userName],[userPassword],[securityMode],[securityPolicy],[certificatePath],[privateKeyPath])
```
参数
- `endPointUrl`是字符串，表示要连接的endpoint的url。

- `clientUri`是字符串，表示client的标识符，若指定certificate，则需要与certificate里的URI保持一致。

- `userName`是字符串，用户名，若server未设置，可省略。

- `userPassword`是字符串，用户密码，和userName一起使用。

- `securityMode`是字符串，连接的安全模式，必须是"None"、"Sign"、"SignAndEncrypt"中的一个，默认为"None"。

- `securityPolicy`是字符串，连接的安全策略，必须是"None", "Basic256", "Basic128Rsa15", "Basic256Sha256"中的一个，默认为"None"。若采用 "Basic256", "Basic128Rsa15", "Basic256Sha256"加密，则需要指定certificate和privateKey。可以使用./open62541/cert/目录下的certificate和privateKey，也可以使用自己生成的证书（open62541项目tool目录下的工具可用于[生成证书](https://github.com/open62541/open62541/tree/master/tools/certs)）。

- `certificatePath`是字符串，指定证书路径，若不采用加密通讯可不指定。

- `privateKeyPath`是字符串，指定私钥路径，若不采用加密通讯可不指定。

详情

连接OPC server。返回的结果是一个connection，可以显式的调用close函数去关闭。若采用加密通讯，服务端需要信任指定的证书。


例子

```
connection=opcua::connect("opc.tcp://LOCALHOST:4840/","myClient");
connection=opcua::connect("opc.tcp://LOCALHOST:4840/","myClient","user1","123456");
connection=opcua::connect("opc.tcp://LOCALHOST:4840","urn:opcua.client.application","user1","123456","SignAndEncrypt","Basic128Rsa15","./open62541/cert/client_cert.der","./open62541/cert/client_key.der");
```

### 2.3 查看所有Node

语法
```
opcua::browseNode(connection)
```
参数
- `connection`是connect函数返回的值。

详情

查看所有Node。返回的结果是一个table，包含两列，一列是nodeNamespace，一列是nodeIdString。


例子

```
opcua::browseNode("opc.tcp://LOCALHOST:4840/");
```

### 2.4. 读取Node（同步）

语法
```
opcua::readNode(connection, nodeNamespace, nodeIdString, [table])
```
参数
- `connection`是connect函数返回的值。
- `nodeNamespace`是int或int的数组，表示node的Namespace。
- `nodeIdString`是字符串或字符串的数组，表示node的字符串形式的ID。
- `table`是表或表的数组（为数组时，表的个数须与node个数相同），用于存放读取的结果，若表是一个，就把所有node的值都插入这张表中，若表多个，每个node读取的值分别插入这些表中。若不输入表，则返回值是一张表，表的记录是读取的node值。

详情

读取一个node值，使用前需要先建立一个OPC连接。每个node返回的结果包含四个值，分别是"node id", 用" : "连接nodeNamespace和nodeIdString，表示Node的ID; "value", 表示node的值; "timestamp", 表示source timestamp（本地时间）; "status", 表示node的value状态。

例子

```
t1 = table(200:0,`nodeID`value`timestamp`status, [SYMBOL, INT, TIMESTAMP, SYMBOL])
opcua::readNode(conn, 3, "Counter",t1)
opcua::readNode(conn, 3, ["Counter","Expression","Random","Sawtooth"],t1)
t2 = table(200:0,`nodeID`value`timestamp`status`nodeID`value`sourceTimestamp`status,[SYMBOL, INT, TIMESTAMP, SYMBOL，SYMBOL, INT, TIMESTAMP, SYMBOL])
opcua::readNode(conn, 1, ["test1","test4"],t2)
t3 = table(200:0,`nodeID`value`timestamp`status, [SYMBOL, INT, TIMESTAMP, SYMBOL])
t4 = table(200:0,`nodeID`value`timestamp`status, [SYMBOL, INT, TIMESTAMP, SYMBOL])
t5 = table(200:0,`nodeID`value`timestamp`status, [SYMBOL, INT, TIMESTAMP, SYMBOL])
opc::readNode(conn, 1, ["test1","test4", "test9"],[t3,t4,t5]) 
```

### 2.5. 写入Node（同步）
语法
```
opc::writeTag(connection, nodeNamespace, nodeIdString, value)
```
参数
- `connection`是connect函数返回的值。
- `nodeNamespace`是int或int的数组，表示node的Namespace。
- `nodeIdString`是字符串或字符串的数组，表示node的字符串形式的ID。
- `value`是Node的值或数组。

详情

写入一个或一组Node的值。如果写入类型错误，会抛出异常。

例子
```
opcua::writeNode(conn,1,"testwrite.test1",1)
opcua::writeNode(conn,1,["testwrite.test5","testwrite.test6"],[33,11])
opcua::writeNode(conn,1,"testwrite.test2",[1,2,3,4])//one-dimensional array
m = matrix([[1,2,3],[1,2,3]])
opcua::writeNode(conn,1,"testwrite.test3",m)//two-dimensional array
```

### 2.6 订阅

语法
```
opcua::subscribe(connection, nodeNamespace, nodeIdString, handler)
```
参数
- `connection`是connect函数返回的值。
- `nodeNamespace`是int或int的数组，表示node的Namespace。
- `nodeIdString`是字符串或字符串的数组，表示node的字符串形式的ID。
- `handler`是数据发生变化时调用的回调函数或表。

详情

订阅Node的值,需要注意的是目前一个订阅需要独占一个connection连接，即若一个连接调用subscribe，不能再用这个连接去做readNode和writeNode等操作。


例子

```
t1 = table(200:0,`nodeID`value`timestamp`status, [SYMBOL, INT, TIMESTAMP, SYMBOL])
conn1=opcua::connect("opc.tcp://LOCALHOST:4840/"，"myClient")
opcua::subscribe(conn1,1,"test.subscribe",t1)
t2 = table(200:0,`nodeID`value`timestamp`status, [STRING, INT, TIMESTAMP, STRING])
conn2=opcua::connect("opc.tcp://LOCALHOST:4840/"，"myClient")
opcua::subscribe(conn2, 3, ["Counter","Expression","Random","Sawtooth"],t2)
t3 = table(200:0,`nodeID`value`timestamp`status, [SYMBOL, BOOL, TIMESTAMP, SYMBOL])
def callback1(mutable t, d) {
	t.append!(d)
}
conn3=opcua::connect("opc.tcp://LOCALHOST:4840/"，"myClient")
opcua::subscribe(conn3,2, "testsubscribe",callback1{t3})
```

### 2.7 取消订阅

语法
```
opcua::unsubscribe(connection)
```
参数
- `connection`是connect函数返回的值。

详情

取消client的订阅。

例子
```
opcua::unsubscribe(connection)
```

### 2.8 关闭连接
语法
```
opcua::close(connection)
```
参数
- `connection`是connect函数返回的值。
详情

断开与OPC server的连接

例子
```
opcua::close(connection)
```
