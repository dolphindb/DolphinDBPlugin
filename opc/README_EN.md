# OPC Plugin

## 1. Build

1. Install [MinGW](http://www.mingw.org/). Your version of MinGW should contain the COM library.
2. Move libDolphinDB.dll to the 'build' direcotry.
3. Move CMakeLists.txt to the 'DolphinDBPlugin' directory and build with CLion.

## 2. API 

OPC Plugin supports OPC 2.0 protocol. It does not support asynchronous read/write. 

### 2.1 Get OPC Server

```
OPC::getServerList(host)
```
- 'host' is a string indicating the IP address or the server name. 

Get the OPC server. It returns a table of two columns. The first column is progID indicating the identification of server and the second column is CLSID.

For example:

```
opc::getOpcServerList("desk9")
```

### 2.2 Connect to OPC Server

```
opc::connect(host, serverName, [reqUpdateRate_ms=100])
```
- 'host' is a string indicating the IP address or the server name. 

- 'serverName' is a string indicating the name of the OPC server.

- 'reqUpdateRate_ms' is a positive integer indicating the frequency of attempts               in terms of milliseconds. It is an optional parameter and the default value is 100. 

Connect to OPC Server. It returns an integer indicating the clientID which starts from 0.

For example:

```
connection=opc::connect(`127.0.0.1,`Matrikon.OPC.Simulation.1,100)
```

### 2.3 Read Tags Synchronously

```
opc::readTag(connection, tagName, [table])
```
- 'connection' is the object returned by function `connect`.

- 'tagName' is a string scalar/vector indicating tag names.

- 'table' is a table or a tuple of tables with the same number of tables as the number of tags to save the read results. If 'table' is one table, all results are saved in it. If 'table' is a tuple of tables, each tag is saved in a separate table. If 'table' is unspecified, return a table with all tags. 

Read tags synchronously. It returns a table.

For example:

```
t = table(200:0,`time`value`quality, [TIMESTAMP, DOUBLE, INT])
opc::readTag(conn, "testwrite.test9",t)
tm = table(200:0,`time`tag1`quality1`tag2`quality2, [TIMESTAMP,STRING, INT,INT,INT])
opc::readTag(conn, ["testwrite.test1","testwrite.test4"],tm) 
t1 = table(200:0,`time`value`quality, [TIMESTAMP, STRING, INT])
t2 = table(200:0,`time`value`quality, [TIMESTAMP, INT, INT])
t3 = table(200:0,`time`value`quality, [TIMESTAMP, DOUBLE, INT])
opc::readTag(conn, ["testwrite.test1","testwrite.test4", "testwrite.test9"],[t1,t2,t3]) 
```

### 2.4 Write Tags Synchronously

```
opc::writeTag(connection, tagName, value)
```
- 'connection' is the object returned by function `connect`.

- 'tagName' is a string scalar/vector indicating tag names.

- 'value' is a scalar/vector indicating the value of the tags.

Write tags synchronously. If the data type is incompatible, it will throw an exception.

For example:

```
opc::writeTag(conn,"testwrite.test1",[1.112,0.123,0.1234])
opc::writeTag(conn,["testwrite.test5","testwrite.test6"],[33,11])

```

### 2.5 Subscribe

```
opc::subscribe(connection, tagName, handler)
```
- 'connection' is the object returned by function `connect`.

- 'tagName' is a string scalar/vector indicating tag names.

- 'handler' is a unary function or a table. It is used to process the subscribed data. 

> Note that the function will block the thread. We recommend to use `submitJob` to submit multiple subscription tasks.

For example,

```
//loadPlugin("PluginOPC.txt")
OPC::connectOpcServer(`127.0.0.1,`Matrikon.OPC.Simulation.1)

t1 = table(200:0,`time`quality`value, [DATETIME, INT, STRING])
t2 = table(200:0,`time`quality`value, [DATETIME, INT, INT])
def callback1(mutable t1, d) {
	t1.append!(d)
}
callback_1 = callback1{t1}
callback_2 = callback1{t2}

def suba(tag, callback) {
	cid = OPC::connectOpcServer(`127.0.0.1,`Matrikon.OPC.Simulation.1)
	print("mycid:" + cid);
	OPC::subscribeTag(cid,tag, callback)
}
id1 = submitJob("i1", "subtest12" , suba, ".test12", callback_2)
sleep(1000);
id2 = submitJob("i2", "subtest13" , suba, ".test13", callback_1)
```

If we submit subscription tasks with function `submitJob`, we can get the status of the task with the function `getJobStatus` and get the output of the task with function `getJobMessage`.

### 2.6 Unsubscribe

```
opc::unsubcribe(connection)
```
- 'connection' is the object returned by function `connect`.

Unsubscribe from the OPC server.

For example:

```
opc::unsubcribe(connection)
```

### 2.7 Close a connection

```
opc::close(connection)
```
