# DolphinDB HBase Plugin

The DolphinDB Hbase plugin can establish a connection to HBase via Thrift and load data from the HBase database. Currently, HBase Plugin is only supported on Linux.

Recommended version: 

- HBase: version 1.2.0
- Thrift: version 0.14.0

- [DolphinDB HBase Plugin](#dolphindb-hbase-plugin)
  - [1. Install with `installPlugin`](#1-install-with-installplugin)
    - [Installation Steps](#installation-steps)
    - [Start Thrift server](#start-thrift-server)
  - [2. Methods](#2-methods)
    - [2.1 hbase::connect](#21-hbaseconnect)
    - [2.2 hbase::showTables](#22-hbaseshowtables)
    - [2.3 hbase::deleteTable](#23-hbasedeletetable)
    - [2.4 hbase::getRow](#24-hbasegetrow)
    - [2.5 hbase::load](#25-hbaseload)
  - [3. Data Type Mappings](#3-data-type-mappings)
  - [Appendix: Manual Installation](#appendix-manual-installation)
    - [Use Precompiled Package](#use-precompiled-package)
    - [Compile from Source](#compile-from-source)


## 1. Install with `installPlugin`

Required server version: DolphinDB 2.00.10.8/1.30.22.8 or higher

### Installation Steps

(1) Use `listRemotePlugins` to check plugin information in the plugin repository

```
login("admin", "123456")
listRemotePlugins(, "http://plugins.dolphindb.cn/plugins/")
```

(2) Invoke `installPlugin` for plugin installation

```
installPlugin("hbase")
```

It returns `<path_to_HBase_plugintxt>/pluginHBase.txt`.

(3) Load the plugin with `loadPlugin` (which takes the above return value as its input). 

```
loadPlugin("<path_to_HBase_plugintxt>/pluginHBase.txt")
```

### Start Thrift server

Run the following command to start the Thrift server with port specified as 9090:

```
$HBASE_HOME/bin/hbase-daenom.sh start thrift -p 9090
```

You can close the Thrift server with the following command:

```
$HBASE_HOME/bin/hbase-daemon.sh stop thrift
```

## 2. Methods

### 2.1 hbase::connect

**Syntax**

hbase::connect(host, port, [isFramed], [timeout])

**Arguments**

- host: *STRING*. The server address to connect to.
- port: *INT*. The port number of the Thrift server. 
- isFramed: *BOOL, default False*. Whether to transport using `TBufferedTransport` (default) or `TFramedTransport` (true).
- timeout: *INT, default 5000ms*. The maximum time for connection and a receive call to wait before timeout.

**Details**

Build a connection to HBase via Thrift server and return an HBase handle.

**Examples**

```
conn = hbase::connect("192.168.1.114", 9090)
```

**Note**: If the connection remains inactive for a while (1min by default), HBase will automatically close it. If you try to operate through this connection, the `No more data to read` error will be reported. In such case, you have to execute `hbase::connect` to reconnect.

You can configure with *hbase.thrift.server.socket.read.timeout* and *hbase.thrift.connection.max-idletime* to change the timeout.

The following configuration parameters change the timeout to 1 day.

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

**Syntax**

hbase::showTables(hbaseConnection)

**Arguments**

- hbaseConnection: The handle returned by `hbase::connect`.

**Details**

Return all table names of the connected database.

**Examples**

```
conn = hbase::connect("192.168.1.114", 9090)
hbase::showTables(conn)
```

### 2.3 hbase::deleteTable

**Syntax**

hbase::deleteTable(hbaseConnection, tableName)

**Arguments**

- hbaseConnection: The handle returned by hbase::connect.
- tableName: STRING or STRING vector. The name of the table to be deleted.

**Details**

Delete tables in the database.

**Examples**

```
conn = hbase::connect("192.168.1.114", 9090)
hbase::deleteTable(conn, "demo_table")
```

### 2.4 hbase::getRow

**Syntax**

hbase::getRow(hbaseConnection, tableName, rowKey, [columnName])

**Arguments**

- hbaseConnection: The handle returned by `hbase::connect`.
- tableName: *STRING*. The name of the table to be read.
- rowKey: *STRING*. The index of the row to be read.
- columnName: *STRING or STRING vector*. The name of the column to be read. If not specified, all columns are read by default.

**Details**

Return the specific record with *rowKey*.

**Examples**

```
conn = hbase::connect("192.168.1.114", 9090)
hbase::getRow(conn, "test", "row1")
```

### 2.5 hbase::load

**Syntax**

hbase::load(hbaseConnection, tableName, [schema])

**Arguments**

- hbaseConnection: The handle returned by `hbase::connect`.
- tableName: *STRING*. The name of the table to be loaded.
- schema: *optional*. If specified, it is a table containing names of the columns to be imported and their data types. The column names speicified in schema must be consistent with the HBase column names. If not specified, the table will be created based on the first row with each column of STRING type. Note that each row must have the same size.

**Details**

Import the HBase results into a DolphinDB in-memory table. The data types supported for schema are described in chapter 3.

**Examples**

```
conn = hbase::connect("192.168.1.114", 9090)
t =  table(["cf:a","cf:b", "cf:c", "cf:time"] as name, ["STRING", "INT", "FLOAT", "TIMESTAMP"] as type)
t1 = hbase::load(conn, "test", t)
```

## 3. Data Type Mappings

The following is the data type mappings when an HBase table is imported to DolphinDB. Data stored in HBase must conform to the types specified in the table below, otherwise Null values will be returned.

| Type          | HBase                                                        | DolphinDB                                                    |
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

## Appendix: Manual Installation

In addition to installing the plugin with function installPlugin, you can also install through precompiled binaries or compile from source. These files can be accessed from our [GitHub repository](https://github.com/dolphindb/DolphinDBPlugin/tree/master) by switching to the appropriate version branch.

### Use Precompiled Package

You can use the pre-built binaries `libPluginOPCUA.so`.

(1) Add the plugin path to the library search path `LD_LIBRARY_PATH`

```
export LD_LIBRARY_PATH=path_to_hbase_plugin/:$LD_LIBRARY_PATH
```

(2) Start the DolphinDB server and load the plugin.

```
loadPlugin("path_to_hbase_plugin/PluginHBase.txt")
```

### Compile from Source

You can also manually compile an HBase plugin with [CMake](https://cmake.org/) on Linux following the instructions:

(1) Install CMake

```
sudo apt-get install cmake
```

(2) Install OpenSSL

```
sudo apt-get install openssl
```

(3) Build the entire project

```
mkdir build
cd build
cmake ../
make
```
