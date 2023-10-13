# DolphinDB OPCUA Plugin

OPC是自动化行业与其他行业用于数据安全交换的互操作性标准。OPC DA只可用于Windows操作系统，OPC UA则可以独立于平台。本插件实现了DolphinDB与OPC UA服务器之间的数据传输。

## 1. 安装

### 1.1 预编译安装

可以直接使用已编译好的 libPluginOPCUA.dll 或 libPluginOPCUA.so：

执行Linux命令，指定插件运行时需要的动态库路径
``` shell
export LD_LIBRARY_PATH=/path/to/PluginOPCUA:$LD_LIBRARY_PATH
```
启动DolphinDB服务，并执行DolphinDB插件加载脚本
``` shell
loadPlugin("/path/to/PluginOPCUA/PluginOPCUA.txt")
```

请注意，若使用 Windows 插件，加载时必须指定绝对路径，且路径中使用"\\\\"或"/"代替"\\"。

### 1.2 编译安装

#### 环境准备

需要先编译mbedtls静态库和open62541动态库。步骤如下：

#### Windows

安装mbedtls

* 从GitHub上下载最新的mbedtls项目：
    ```
    git clone https://github.com/ARMmbed/mbedtls.git
    ```

* 使用CMake编译为静态库：
    ```
    cd mbedtls
    mkdir build
    cd build
    cmake .. -G "MinGW Makefiles" -DENABLE_PROGRAMS=OFF
    make
    ```
/libmbedcrypto.a $(LIB_DIR)/libmbedx509.a $(LIB_DIR)/libmbedtls.a
编译得到静态库位于mbedtls/build/library下，分别是libmbedcrypto.a、libmbedx509.a、libmbedtls.a。

安装open62541

* 从GitHub上下载1.0版本的open62541项目：
```
git clone https://github.com/open62541/open62541.git
git submodule update --init --recursive
cd open62541
git checkout 1.0
```

* 使用CMake编译为动态库：
```
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DUA_ENABLE_SUBSCRIPTIONS=ON -DBUILD_SHARED_LIBS=ON -DUA_ENABLE_ENCRYPTION=ON -DMBEDTLS_INCLUDE_DIRS="path_to_mbedtls/include" -DMBEDTLS_LIBRARIES="path_to_mbedtls/build/library" -DMBEDTLS_FOLDER_INCLUDE="path_to_mbedtls/include" -DMBEDTLS_FOLDER_LIBRARY="path_to_mbedtls/build/library"
make
```
**请注意**：用户需要根据实际情况替换路径 -DMBEDTLS_INCLUDE_DIRS 和 -DMBEDTLS_LIBRARIES。

#### Linux Ubuntu

安装mbedtls
```
sudo apt-get install libmbedtls-dev
```

安装open62541方法与Windows一致，可不指定 -DMBEDTLS_INCLUDE_DIRS 和 -DMBEDTLS_LIBRARIES。

#### Linux Centos

安装mbedtls
```
yum install mbedtls-devel
```

安装open62541方法与Windows一致，可不指定 -DMBEDTLS_INCLUDE_DIRS 和 -DMBEDTLS_LIBRARIES。

### 使用cmake构建libPluginOPCUA

* 复制mbedtls/build/library目录下的libmbedcrypto.a、libmbedx509.a、libmbedtls.a到./lib目录下；复制mbedtls/include目录下的mbedtls和psa文件夹到./include目录下（linux系统可跳过这一步）。

* 复制open62541/build/bin目录下的.dll文件，或者所有.so文件到./lib目录下；复制open62541/build/src_generated/、open62541/include/、open62541/plugins/include/下的文件夹到./include目录下，open62541/arch/下的文件夹到./include/open62541目录下。

* 使用cmake构建libPluginOPCUA，linux不需要指定-G。
```
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DLIBDOLPHINDB="path_to_libdolphindb"
make
```
**请注意**：用户需要根据实际情况替换路径 -DLIBDOLPHINDB。

* 将 libopen62541.dll 或 libopen62541.so 复制到build目录下。

## 2. API

**请注意**：使用API前需使用 loadPlugin("/path/to/PluginOPCUA/PluginOPCUA.txt") 导入插件。

### 测试用例

opc.tcp://118.24.36.220:62547/DataAccessServer 为一个在线服务端的serverUrl，可用来测试插件的连接、读写node、订阅等功能。

[prosys opc-ua-simulation-server](https://downloads.prosysopc.com/opc-ua-simulation-server-downloads.php) 提供了一个本地服务器，按照[使用手册](https://downloads.prosysopc.com/opcua/apps/JavaServer/dist/4.0.2-108/Prosys_OPC_UA_Simulation_Server_UserManual.pdf)可指定端点，加密策略，用户令牌，管理证书等，可用来测试插件的加密通信等功能。

### 2.1 获取OPC Server

语法
```
opcua::getOpcServerList(serverUrl)
```

参数

- `serverUrl`是字符串，表示server地址。例如: opc.tcp://127.0.0.1:53530/OPCUA/SimulationServer/。

详情

> 获取OPC server。返回的结果是一个包含5列的表，分别是："ServerUri"表示server的标识符；"ServerName"表示server的名字；"ProductUri"表示server的product标识符；"Type"表示server的类型；"DiscoveryUrl"表示可以用来连接server的url（若有多个，用';'分割）。

例子
```
opcua::getOpcServerList(serverUrl);
```

### 2.2 获取OPC Server Endpoints

语法
```
opcua::getOpcEndPointList(serverUrl)
```

参数

- serverUrl是字符串，表示server地址，例如: opc.tcp://127.0.0.1:53530/OPCUA/SimulationServer/。

详情

> 获取OPC server。返回的结果是一个包含5列的表，分别是："endpointUrl"表示server可以用来连接的端点的url；"transportProfileUri"表示该端点传输配置文件的标识符；"securityMode"表示该端点的安全模式；"securityPolicyUri"表示该端点安全策略的标识符；"securityLevel"表示该端点安全等级。

例子
```
opcua::getOpcEndPointList(serverUrl);
```

### 2.3 连接

语法
```
opcua::connect(endPointUrl,clientUri,[userName],[userPassword],[securityMode],[securityPolicy],[certificatePath],[privateKeyPath])
```

参数

- endPointUrl是字符串，表示要连接的endpoint的url。

- clientUri是字符串，表示client的标识符，若指定certificate，则需要与certificate里的URI保持一致。

- userName是字符串，表示用户名。若server未设置，可省略。

- userPassword是字符串，表示用户密码，与userName一起使用。

- securityMode是字符串，表示连接的安全模式，必须是"None", "Sign", "SignAndEncrypt"中的一个，默认为"None"。

- securityPolicy是字符串，连接的安全策略，必须是"None", "Basic256", "Basic128Rsa15", "Basic256Sha256"中的一个，默认为"None"。若采用 "Basic256", "Basic128Rsa15", "Basic256Sha256"加密，则需要指定certificate和privateKey。可以使用./open62541/cert/目录下的certificate和privateKey，也可以使用自己生成的证书（open62541项目tool目录下的工具可用于[生成证书](https://github.com/open62541/open62541/tree/master/tools/certs)）。

- certificatePath是字符串，指定证书路径，若不采用加密通讯可不指定。

- privateKeyPath是字符串，指定私钥路径，若不采用加密通讯可不指定。

详情

> 连接OPC server。返回的结果是一个connection，可以显式的调用close函数去关闭。若采用加密通讯，服务端需要信任指定的证书。如果加密通信，使用Prosys作为本地模拟服务器。
需要在Users界面下添加用户名和密码。例如: 用户名: "user1"和 密码: "123456"。然后，在Certificates界面下，信任`open62541server@localhost`。

例子

```
connection=opcua::connect(serverUrl,"myClient");
connection=opcua::connect(serverUrl,"myClient","user1","123456");
connection=opcua::connect(serverUrl,"urn:opcua.client.application","user1","123456","SignAndEncrypt","Basic128Rsa15","./open62541/cert/client_cert.der","./open62541/cert/client_key.der");
```

### 2.3 查看所有Node

语法
```
opcua::browseNode(connection)
```
参数
- connection是connect函数返回的值。

详情

> 查看所有Node。返回的结果是一个table，包含两列，一列是nodeNamespace，一列是nodeIdString。

例子

```
connection=opcua::connect(serverUrl,"myClient");
opcua::browseNode(connection);
```

### 2.4. 读取Node（同步）

语法
```
opcua::readNode(connection, nodeNamespace, nodeIdString, [table])
```

参数
- connection是connect函数返回的值。
- nodeNamespace是int的标量或数组，表示node的Namespace。
- nodeIdString是字符串的标量或数组，表示node的字符串形式的ID。
- table是表或表的数组（为数组时，表的个数须与node个数相同），用于存放读取的结果。若是一个表，将所有node的值都插入这张表中；若多个表，每个node读取的值分别插入这些表中。若不输入表，则返回值是一张表，表的记录是读取的node值。

详情

> 读取一个node值，使用前需要先建立一个OPC连接。每个node返回的结果包含四个值，分别是"node id"，表示Node的ID， 用":"连接nodeNamespace和nodeIdString；"value"表示node的值；"timestamp"表示source timestamp（本地时间）；"status"表示node的value状态。


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
- connection是connect函数返回的值。
- nodeNamespace是int的标量或数组，表示node的Namespace。
- nodeIdString是字符串的标量或数组，表示node的字符串形式的ID。
- value是Node的值或数组。

详情

> 写入一个或一组Node的值。如果写入类型错误，会抛出异常。

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
- connection是connect函数返回的值。
- nodeNamespace是int的标量或数组，表示node的Namespace。
- nodeIdString是字符串或字符串的数组，表示node的字符串形式的ID。
- handler是数据发生变化时调用的回调函数或表。

详情

> 订阅Node的值,需要注意的是目前一个订阅需要独占一个connection连接，即若一个连接调用subscribe，不能再用这个连接去做readNode和writeNode等操作。

例子

```
t1 = table(200:0,`nodeID`value`timestamp`status, [SYMBOL, INT, TIMESTAMP, SYMBOL])
conn1=opcua::connect(serverUrl，"myClient")
opcua::subscribe(conn1,1,"test.subscribe",t1)
t2 = table(200:0,`nodeID`value`timestamp`status, [STRING, INT, TIMESTAMP, STRING])
conn2=opcua::connect(serverUrl，"myClient")
opcua::subscribe(conn2, 3, ["Counter","Expression","Random","Sawtooth"],t2)
t3 = table(200:0,`nodeID`value`timestamp`status, [SYMBOL, BOOL, TIMESTAMP, SYMBOL])
def callback1(mutable t, d) {
	t.append!(d)
}
conn3=opcua::connect(serverUrl，"myClient")
opcua::subscribe(conn3,2, "testsubscribe",callback1{t3})
```

### 2.7 取消订阅

语法
```
opcua::unsubscribe(subscription)
```
参数
- subscription是`connect`函数返回的值或`getSubscriberStat`返回的订阅标识符。

详情

> 取消client的订阅。

例子
```
opcua::unsubscribe(subscription)
```

### 2.8 查看订阅状态

语法
```
opcua::getSubscriberStat()
```

详情

> 查看当前所有的订阅状态, 包括以下信息:"subscriptionId", 表示订阅标识符; "user", 表示建立订阅的会话用户; "endPointUrl", 表示连接的endPointUrl; "clientUri", 表示连接的client标识符; "nodeID", 表示订阅的所有, 用"NodenodeNamespace:nodeIdString"表示每一个Node, 不同的Node用';'分割; "createTimestamp", 表示订阅建立时间; "receivedPackets", 表示订阅收到的消息报文数, "errorMsg", 表示最新处理消息时的异常信息。

例子
```
opcua::getSubscriberStat()
```

### 2.9 关闭连接
语法
```
opcua::close(connection)
```
参数
- connection是connect函数返回的值。

详情

> 断开与OPC server的连接。

例子
```
opcua::close(connection)
```
# ReleaseNotes:

## 故障修复

* 修复了多线程作业相关的 server 宕机的问题。（**1.30.22**）
