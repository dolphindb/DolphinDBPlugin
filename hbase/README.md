DolphinDB HBase Plugin

 本插件通过thrift接口连接到HBase，并读取数据。HBase版本为1.2.0，thrift版本为0.14.0。

HBase插件目前支持版本：[relsease200](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/hbase/README.md), [release130](https://github.com/dolphindb/DolphinDBPlugin/blob/release130/hbase/README.md), [release120](https://github.com/dolphindb/DolphinDBPlugin/blob/release120/hbase/README.md)。您当前查看的插件版本为release200，请使用DolphinDB 2.00.X版本server。若使用其它版本server，请切换至相应插件分支。

## 1 安装

### 1.1 预编译安装

可以导入DolphinDB安装包中或者bin目录下预编译好的HBase插件。

#### Linux

(1) 添加插件所在路径到LIB搜索路径 LD_LIBRARY_PATH

```
export LD_LIBRARY_PATH=path_to_hbase_plugin/:$LD_LIBRARY_PATH
```

(2) 启动DolphinDB server并导入插件

```
loadPlugin("path_to_hbase_plugin/PluginHBase.txt")
```

### 1.2 编译安装

可以使用以下方法编译HBase插件，编译成功后通过以上方法导入。

#### 在Linux下安装

**使用cmake构建**

安装cmake：

```
sudo apt-get install cmake
```

安装OpenSSL：

```
sudo apt-get install openssl
```

编译整个项目：

```
mkdir build
cd build
cmake ../
make
```

### 1.3 开启thrift server

可用以下命令开启thrift server，并指定端口9090：

```
$HBASE_HOME/bin/hbase-daenom.sh start thrift -p 9090
```

使用完毕后，使用以下命令关闭thrift：

```
$HBASE_HOME/bin/hbase-daemon.sh stop thrift
```

## 2 用户接口

### 2.1 hbase::connect

#### 语法

hbase::connect(host, port, [isFramed])

#### 参数

- host: 要连接的thrift server的IP地址，类型为string。
- port: 要连接thrift server的端口号，类型为int。
- isFramed: 为true表示通过TFramedTransport进行传输，为false表示通过TBufferedTransport进行传输，默认为false。

#### 详情

通过thrift server与hbase建立一个连接，返回一个hbase连接的句柄用于后续操作。

#### 例子

```
conn = hbase::connect("192.168.1.114", 9090)
```

**注意**：如果这个连接长时间没有操作（默认为1min），hbase会自动关闭这个连接，用这个连接进行后续操作时会报`No more data to read`的错误，需要再次执行`hbase::connect`重新进行连接。这个时间可以在hbase的配置文件中进行修改，在conf/hbase-site.xml中添加如下配置表示一天没有操作会自动关闭：

```
<property>
         <name>hbase.thrift.server.socket.read.timeout</name>
         <value>86400000</value>
         <description>eg:milisecond</description>
</property>
```

此外，还需要修改conf/hbase-site.xml中`hbase.thrift.connection.max-idletime`的配置，将这个时间修改成上面同样的时间:

```
<property>
         <name>hbase.thrift.connection.max-idletime</name>
         <value>86400000</value>
</property>
```

### 2.2 hbase::showTables

#### 语法

hbase::showTables(hbaseConnection)

#### 参数

- hbaseConnection: 通过hbase::connect获得的hbase句柄。

#### 详情

显示已连接的数据库中所有的表名。

#### 例子

```
conn = hbase::connect("192.168.1.114", 9090)
hbase::showTables(conn)
```

### 2.3 hbase::deleteTable

#### 语法

hbase::deleteTable(hbaseConnection, tableName)

#### 参数

- hbaseConnection: 通过hbase::connect获得的hbase句柄。
- tableName: 要删除的表的名字，类型为string或者string vector。

#### 详情

删除数据库中存在的表。

#### 例子

```
conn = hbase::connect("192.168.1.114", 9090)
hbase::deleteTable(conn, "demo_table")
```

### 2.4 hbase::getRow

#### 语法

hbase::getRow(hbaseConnection, tableName, rowKey, [columnName])

#### 参数

- hbaseConnection: 通过hbase::connect获得的hbase句柄。
- tableName: 需要读取数据的表的名字，类型为string。
- rowKey: 需要读取的row的索引，类型为string。
- columnName：需要获取的列名，若不指定默认读取所有列数据，类型为string或者string vector。

#### 详情

根据rowKey读取对应的数据。

#### 例子

```
conn = hbase::connect("192.168.1.114", 9090)
hbase::getRow(conn, "test", "row1")
```

### 2.5 hbase::load

#### 语法

hbase::load(hbaseConnection, tableName, [schema])

#### 参数

- hbaseConnection: 通过hbase::connect获得的hbase句柄。
- tableName: 需要读取数据的表的名字，类型为string。
- schema: 包含列名和列的数据类型的表。由于hbase中数据没有数据类型，全是以字节形式存储，若不指定schema会尝试以第一行数据为基准进行建表，需要保证表中每行的列数相同，否则会出错，此时返回的dolphindb表中每列数据类型都为string。若需要改变数据类型就需要指定schema，schema中的列名需要与hbase中所要读取的列名完全一致。

#### 详情

将hbase的查询结果导入DolphinDB中的内存表。schema中支持的数据格式见第3小节。

#### 例子

```
conn = hbase::connect("192.168.1.114", 9090)
t =  table( ["cf:a","cf:b", "cf:c", "cf:time"] as name, ["STRING", "INT", "FLOAT", "TIMESTAMP"] as type)
t1 = hbase::load(conn, "test", t)
```

## 3 支持的数据格式

schema中支持的数据类型如下表所示，如果需要将HBase中的数据转成DolphinDB中对应数据类型，HBase中存储的数据格式需要与下表相同，否则无法转换，会返回空值。

| Type          | HBase数据                                                    | DolphinDB数据                                                |
| ------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| BOOL          | true, 1, FALSE                                               | true, true, false                                            |
| CHAR          | a                                                            | a                                                            |
| SHORT         | 1                                                            | 1                                                            |
| INT           | 21                                                           | 21                                                           |
| LONG          | 112                                                          | 112                                                          |
| FLOAT         | 1.2                                                          | 1.2                                                          |
| DOUBLE        | 3.5                                                          | 3.5                                                          |
| SYMBOL        | s0                                                           | "s0"                                                         |
| STRING        | name                                                         | "name"                                                       |
| DATE          | 20210102, 2021.01.02                                         | 2021.01.02, 2021.01.02                                       |
| MONTH         | 201206, 2012.12                                              | 2012.06M, 2021.12M                                           |
| TIME          | 052013140, 05:20:01.999                                      | 05:20:13.140, 05:20:01.999                                   |
| MINUTE        | 1230, 13:30                                                  | 12:30m, 13:30m                                               |
| SECOND        | 123010, 13:30:10                                             | 12:30:10, 13:30:10                                           |
| DATETIME      | 20120613133010,  2012.06.13 13:30:10, 2012.06.13T13:30:10    | 2012.06.13T13:30:10, 2012.06.13T13:30:10, 2012.06.13T13:30:10 |
| TIMESTAMP     | 20210218051701000, 2012.06.13 13:30:10.008, 2012.06.13T13:30:10.008 | 2021.02.18T05:17:01.000, 2012.06.13T13:30:10.008, 2012.06.13T13:30:10.008 |
| NANOTIME      | 133010008007006, 13:30:10.008007006                          | 13:30:10.008007006, 13:30:10.008007006                       |
| NANOTIMESTAMP | 20120613133010008007006,  2012.06.13 13:30:10.008007006, 2012.06.13T13:30:10.008007006 | 2012.06.13T13:30:10.008007006, 2012.06.13T13:30:10.008007006, 2012.06.13T13:30:10.008007006 |

