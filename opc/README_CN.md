## 1. 通过cmake和MinGW编译

安装[cmake](https://cmake.org/)。 cmake一个流行的项目构建工具,可以帮你轻松的解决第三方依赖的问题。  

安装[MinGW](http://www.mingw.org/)环境，带有com库（应该尽量选择新的版本），目前在64位win10上用MinGW-W64-builds-4.3.3版本编译通过。

把MingGW和cmake的bin目录加入Windows系统Path路径。 

```
    git clone https://github.com/dolphindb/DolphinDBPlugin.git
    cd DolphinDBPlugin
    mkdir build
    cd build
    cmake ../opc -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
    copy /YOURPATH/libDolphinDB.dll . 
    mingw32-make clean
    mingw32-make
```

**注意：** 如果需要指定特定的MingW路径，请在CmakeList.txt中修改以下语句。

```
    set(MINGW32_LOCATION C://MinGW/MinGW/)  
```

## 2. API

当前只支持OPC 2.0协议，且不支持异步读写。

### 2.1 获取OPC Server

语法
```
opc::getServerList(host)
```
参数
- `host`是字符串，表示IP地址，如127.0.0.1。

详情

获取OPC server。返回的结果是一个包含两列的表，一列是progID，表示server的标志符，另一列是该server对应的CLSID。注意获取远程OPC服务器前，需要对OPC Server和OPC Client两侧都进行DCOM配置。详细配置可搜索网上的教程，比如[远程连接opc服务器设置](https://blog.csdn.net/qq_35573625/article/details/85267487)。

例子
```
opc::getOpcServerList("desk9")
```

### 2.2 连接

语法
```
opc::connect(host, serverName，[reqUpdateRate_ms=100])
```
参数
- `host`是字符串，表示IP地址。

- `serverName`是字符串，表示OPC Server的名称。

- `reqUpdateRate_ms`请求更新频率（毫秒）,可选参数，默认为100。

详情

连接OPC server。返回的结果是一个connection，可以显式的调用close函数去关闭，也可以在reference count为0的时候自动释放。注意连接远程OPC服务器前，需要对OPC Server和OPC Client两侧都进行DCOM配置。详细配置可搜索网上的教程，比如[远程连接opc服务器设置](https://blog.csdn.net/qq_35573625/article/details/85267487)。


例子

```
connection=opc::connect(`127.0.0.1,`Matrikon.OPC.Simulation.1,100)
```

### 2.3. 读取tag（同步）

语法
```
opc::readTag(connection, tagName,[table])
```
参数
- `connection`是connect函数返回的值。
- `tagName`是字符串或字符串的数组，表示tag的名称。
- `table`是表或表的数组（为数组时，表的个数须与tag个数相同），用于存放读取的结果，若表是一个，就把所有tag的值都插入这张表中，若表多个，每个tag读取的值分别插入这些表中。若不输入表，则返回值是一张表，表的记录是读取的tag值。

详情

读取一个tag的值，使用前需要先建立一个OPC连接。

例子

```
t = table(200:0,`tag`time`value`quality, [SYMBOL,TIMESTAMP, DOUBLE, INT])
opc::readTag(conn, "testwrite.test9",t)
opc::readTag(conn, ["testwrite.test5","testwrite.test8", "testwrite.test9"],t) 
tm = table(200:0,`time`tag1`quality1`tag2`quality2, [TIMESTAMP,STRING, INT,INT,INT])
opc::readTag(conn, ["testwrite.test1","testwrite.test4"],tm) 
t1 = table(200:0,`tag`time`value`quality, [SYMBOL,TIMESTAMP, STRING, INT])
t2 = table(200:0,`tag`time`value`quality, [SYMBOL,TIMESTAMP, INT, INT])
t3 = table(200:0,`tag`time`value`quality, [SYMBOL,TIMESTAMP, DOUBLE, INT])
opc::readTag(conn, ["testwrite.test1","testwrite.test4", "testwrite.test9"],[t1,t2,t3]) 
```

### 2.4. 写入tag（同步）
语法
```
opc::writeTag(connection, tagName, value)
```
参数
- `connection`是connect函数返回的值。
- `tagName`是字符串或数组，表示tag的名称。
- `value`是tag的值或数组。

详情

写入一个或一组tag的值。如果写入类型错误，会报出异常。

例子
```
opc::writeTag(conn,"testwrite.test1",[1.112,0.123,0.1234])
opc::writeTag(conn,["testwrite.test5","testwrite.test6"],[33,11])
```

### 2.5 订阅

语法
```
opc::subscribe(connection, tagName, handler)
```
参数
- `connection`是connect函数返回的值。
- `tagName`是一个字符串或字符串数组，表示tag的名称。
- `handler`是数据发生变化时调用的回调函数或表。

详情

订阅tag的值


例子

```
t1 = table(200:0,`tag`time`value`quality, [SYMBOL,TIMESTAMP, STRING, INT])
conn1=opc::connect(`127.0.0.1,`Matrikon.OPC.Simulation.1,100)
opc::subscribe(conn1,".testString",  t1)

t2 = table(200:0,`tag`time`value`quality, [SYMBOL,TIMESTAMP, DOUBLE, INT])
conn2=opc::connect(`127.0.0.1,`Matrikon.OPC.Simulation.1,100)
opc::subscribe(conn2,[".testReal8",".testReal4"],  t5)

def callback1(mutable t, d) {
	t.append!(d)
}
t3 = table(200:0,`tag`time`value`quality, [SYMBOL,TIMESTAMP, BOOL, INT])
conn10 = opc::connect(`127.0.0.1,`Matrikon.OPC.Simulation.1,10)
opc::subscribe(conn10,".testBool",   callback1{t3})
```

### 2.6 取消订阅

语法
```
opc::unsubcribe(connection)
```
参数
- `connection`是connect函数返回的值。

详情

取消client的订阅。

例子
```
opc::unsubcribe(connection)
```

### 2.7 关闭连接
语法
```
opc::close(connection)
```
参数
- `connection`是connect函数返回的值。
详情

断开与OPC server的连接

例子
```
opc::close(connection)
```