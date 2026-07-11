# DolphinDB HBase Plugin

This plugin connects to HBase through Thrift and reads data from HBase.

## 1. APIs

### 1.1 hbase::connect

**Syntax**

hbase::connect(host, port, [isFramed], [timeout])

**Arguments**

- host: *STRING*. The server address to connect to.
- port: *INT*. The port number of the Thrift server. 
- isFramed: *BOOL, optional*. If omitted, the plugin tries both `TBufferedTransport` and `TFramedTransport`. If set to false, only `TBufferedTransport` is tried. If set to true, only `TFramedTransport` is tried.
- timeout: *INT, default 5000ms*. The maximum time for connection and a receive call to wait before timeout.

**Details**

Build a connection to HBase through the Thrift server and return an HBase handle. The plugin automatically tries `TBinaryProtocol` and `TCompactProtocol`.

**Examples**

```
conn = hbase::connect("192.168.1.114", 9090)
```

**Note**: If the connection remains inactive for a while (1 minute by default), HBase closes it automatically. If you continue using this connection, the error `No more data to read` will be reported. In that case, run `hbase::connect` again to reconnect.

You can change the timeout in the HBase configuration file with `hbase.thrift.server.socket.read.timeout` and `hbase.thrift.connection.max-idletime`.

The following configuration changes the timeout to 1 day.

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

### 1.2 hbase::showTables

**Syntax**

hbase::showTables(hbaseConnection)

**Arguments**

- hbaseConnection: The handle returned by `hbase::connect`.

**Details**

Return all table names in the connected database.

**Examples**

```
conn = hbase::connect("192.168.1.114", 9090)
hbase::showTables(conn)
```

### 1.3 hbase::deleteTable

**Syntax**

hbase::deleteTable(hbaseConnection, tableName)

**Arguments**

- hbaseConnection: The handle returned by `hbase::connect`.
- tableName: STRING or STRING vector. The name of the table to be deleted.

**Details**

Delete tables from the database.

**Examples**

```
conn = hbase::connect("192.168.1.114", 9090)
hbase::deleteTable(conn, "demo_table")
```

### 1.4 hbase::getRow

**Syntax**

hbase::getRow(hbaseConnection, tableName, rowKey, [columnName])

**Arguments**

- hbaseConnection: The handle returned by `hbase::connect`.
- tableName: *STRING*. The name of the table to read.
- rowKey: *STRING*. The key of the row to read.
- columnName: *STRING or STRING vector*. The column name to read. If not specified, all columns are read by default.

**Details**

Return the record corresponding to *rowKey*.

**Examples**

```
conn = hbase::connect("192.168.1.114", 9090)
hbase::getRow(conn, "test", "row1")
```

### 1.5 hbase::load

**Syntax**

hbase::load(hbaseConnection, tableName, [schema], [scanOptions])

**Details**

Import query results from HBase into a DolphinDB in-memory table. The data types supported in schema are described in Section 2.

**Arguments**

- hbaseConnection: The handle returned by `hbase::connect`.
- tableName: *STRING*. The name of the table to be loaded.
- schema: *optional*. A table containing the names of the columns to import and their data types. Since HBase stores data as bytes, the returned columns are STRING by default. Use this parameter to specify column types.
- scanOptions: *optional dictionary*. The plugin passes these options to HBase Thrift `TScan`. The following keys are currently supported:
  - `startRow` (STRING scalar)
  - `stopRow` (STRING scalar)
  - `columns` (STRING scalar/vector)
  - `caching` (positive INT scalar). See [MapReduce Scan Caching](https://hbase.apache.org/docs/mapreduce#mapreduce-scan-caching) for the meaning of this option.
  - `filterString` (STRING scalar). See [Filter Language](https://hbase.apache.org/docs/thrift-filter-language) for the meaning of this option.

**Examples**

```
conn = hbase::connect("192.168.1.114", 9090)
t =  table(["cf:a","cf:b", "cf:c", "cf:time"] as name, ["STRING", "INT", "FLOAT", "TIMESTAMP"] as type)
t1 = hbase::load(conn, "test", t)
```

```
// Equivalent to HBase shell/other clients:
// scan 'vehicle_test_data', {FORMATTER => 'toString', LIMIT => 1}
conn = hbase::connect("192.168.1.114", 9090)
scan = {
    "filterString": "PageFilter(1)"
}
t1 = hbase::load(conn, "vehicle_test_data", scan)
```

```
conn = hbase::connect("192.168.1.114", 9090)
scan = {
    "startRow": "row001",
    "stopRow": "row999"
}
t = table(["cf:a","cf:b"] as name, ["STRING", "INT"] as type)
t1 = hbase::load(conn, "test", t, scan)
```

## 2. Supported Data Types

The following table lists the data types supported in schema. Data stored in HBase must conform to the formats shown below so it can be converted to the corresponding DolphinDB data types. Otherwise, null values will be returned.

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
