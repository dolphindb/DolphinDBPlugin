DolphinDB HBase Plugin

本插件通过 Thrift 接口连接到 HBase，并读取数据。推荐版本：HBase 版本为 1.2.0，Thrift 版本为 0.14.0。

HBase插件目前支持版本：[relsease200](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/hbase/README.md), [release130](https://github.com/dolphindb/DolphinDBPlugin/blob/release130/hbase/README.md), [release120](https://github.com/dolphindb/DolphinDBPlugin/blob/release120/hbase/README.md)。您当前查看的插件版本为release200，请使用DolphinDB 2.00.X版本server。若使用其它版本server，请切换至相应插件分支。

## 1 安装

### 1.1 预编译安装

可以导入 bin 目录下预编译好的 HBase 插件。

#### Linux

(1) 添加插件所在路径到 LIB 搜索路径 LD_LIBRARY_PATH

```
export LD_LIBRARY_PATH=path_to_hbase_plugin/:$LD_LIBRARY_PATH
```

(2) 启动 DolphinDB server 并导入插件

```
loadPlugin("path_to_hbase_plugin/PluginHBase.txt")
```

### 1.2 编译安装

通过以下方法编译 HBase 插件，编译成功后通过以上方法导入插件。

#### 在 Linux 下安装

**使用 cmake 构建**

安装 cmake：

```
sudo apt-get install cmake
```

安装 OpenSSL：

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

### 1.3 开启 Thrift server

通过以下命令开启 Thrift server，并指定端口 9090：

```
$HBASE_HOME/bin/hbase-daenom.sh start thrift -p 9090
```

通过以下命令关闭 Thrift：

```
$HBASE_HOME/bin/hbase-daemon.sh stop thrift
```

## 2 用户接口

### 2.1 hbase::connect

#### 语法

hbase::connect(host, port, [isFramed], [timeout])

#### 参数

- host: 要连接的 Thrift server 的 IP 地址，类型为 STRING。
- port: 要连接 Thrift server 的端口号，类型为 INT。
- isFramed: 布尔值，默认为 false，表示通过 TBufferedTransport 进行传输。若设置为 true，则表示通过 TFramedTransport 进行传输。
- timeout: 建立连接（ConnTimeout）与接收回复（RecvTimeout）的最长等待时间，单位为毫秒，默认为5000ms，类型为 INT。

#### 详情

通过 Thrift server 与 HBase 建立一个连接，返回一个 HBase 连接的句柄。

#### 例子

```
conn = hbase::connect("192.168.1.114", 9090)
```

**注意**：如果该连接长时间（默认为 1min）没有操作，HBase 会自动关闭这个连接。此时再通过该连接进行后续操作时，会报 `No more data to read` 的错误，需要执行 `hbase::connect` 重新进行连接。通过 HBase 的配置文件（conf/hbase-site.xml）可修改超时时间。若添加如下配置，则表示一天没有操作时将自动关闭连接：

修改 `hbase.thrift.server.socket.read.timeout` 和 `hbase.thrift.connection.max-idletime`

```
<property>
         <name>hbase.thrift.server.socket.read.timeout</name>
         <value>86400000</value>
         <description>eg:milisecond</description>
</property>
```

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

- hbaseConnection: 通过 hbase::connect 获得的 HBase 句柄。

#### 详情

显示已连接的数据库中所有表的表名。

#### 例子

```
conn = hbase::connect("192.168.1.114", 9090)
hbase::showTables(conn)
```

### 2.3 hbase::deleteTable

#### 语法

hbase::deleteTable(hbaseConnection, tableName)

#### 参数

- hbaseConnection: 通过 hbase::connect 获得的 HBase 句柄。
- tableName: 要删除的表的名字，类型为 STRING 或者 STRING vector。

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

- hbaseConnection: 通过 hbase::connect 获得的 HBase 句柄。
- tableName: 需要读取数据的表的名字，类型为 STRING
- rowKey: 需要读取的 row 的索引，类型为 STRING。
- columnName：需要获取的列名，若不指定默认读取所有列数据，类型为 STRING 或者 STRING vector。

#### 详情

读取 rowKey 所对应的数据。

#### 例子

```
conn = hbase::connect("192.168.1.114", 9090)
hbase::getRow(conn, "test", "row1")
```

### 2.5 hbase::load

#### 语法

hbase::load(hbaseConnection, tableName, [schema])

#### 参数

- hbaseConnection: 通过 hbase::connect 获得的 HBase 句柄。
- tableName: 需要读取数据的表的名字，类型为 STRING。
- schema: 包含列名和列的数据类型的表。由于 HBase 中数据以字节形式存储，没有指定数据类型。若不指定 schema，插件会尝试以第一行数据为基准进行建表，返回的 DolphinDB 表中每列数据类型都为 STRING。请注意，需要保证表中每行数据具有相同的列数，否则会出错。指定 schema 则可以指定每列的数据类型。此时，schema 中的列名需要与 HBase 中所要读取的列名完全一致。

#### 详情

将 HBase 的查询结果导入 DolphinDB 中的内存表。schema 中支持的数据格式见第3小节。

#### 例子

```
conn = hbase::connect("192.168.1.114", 9090)
t =  table(["cf:a","cf:b", "cf:c", "cf:time"] as name, ["STRING", "INT", "FLOAT", "TIMESTAMP"] as type)
t1 = hbase::load(conn, "test", t)
```

## 3 支持的数据格式

schema 中支持的数据类型如下表所示。HBase 中存储的数据格式需要与下表相同，才能将 HBase 中的数据转成 DolphinDB 中对应数据类型，否则无法转换，且会返回空值。

| Type          | HBase 数据                                                    | DolphinDB 数据                                                |
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


# ReleaseNotes:

## 故障修复

* 避免下载数据时对非法格式的 minute 类型数据进行解析。（**2.00.10**）
* 修复在使用 hbase::load 导入 disable table 捕获到异常后未中止运行，导致后续 server 宕机的问题。（**2.00.10**）
* 增加下载数据时对 CHAR 类型数据的转换限制，若输入 string 值的长度超过1，则将返回空值。（**2.00.10**）
* 增加下载数据时对 SECOND 类型转换的检查。（**2.00.10**）
* 增加对连接有效性的检查。（**2.00.10**）
* connect 函数增加对参数 isFramed 非法输入值的检查。（**2.00.10**）

# 功能优化

* 增强了多线程并行时的稳定性。（**2.00.10**）
