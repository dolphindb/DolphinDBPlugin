# DolphinDB ODBC plugin

通过ODBC的Plugin可以将其它数据源数据导入到DolphinDB数据库。

## 1. Prerequisites

根据不同的操作系统和数据库，该插件需要以下用于ODBC的数据库驱动程序。

### 1.1 Ubuntu
```
# 安装unixODBC库
apt-get install unixodbc unixodbc-dev

# 安装SQL Server ODBC驱动
apt-get install tdsodbc

# 安装PostgreSQL ODBC驱动
apt-get install odbc-postgresql

# 安装MySQL ODBC驱动
apt-get install libmyodbc

# 安装SQLite ODBC驱动
apt-get install libsqliteodbc
```

### 1.2 CentOS
```
# 安装unixODBC库
yum install unixODBC  unixODBC-devel

# 安装MySQL ODBC驱动
yum install mysql-connector
```

当使用loadPlugin加载lib文件时，可能会显示一条错误消息 **libodbc.so.1: cannot open shared object file: No such file or directory**.

可以使用以下脚本解决问题：:
```
cd /usr/lib64
ln -s libodbc.so.2.0.0.0 libodbc.so.1
```

请使用正确的lib文件名代替 ```libodbc.so.2.0.0```.

### 1.3 Windows

* 从其网站下载并安装mysql或其他数据库的odbc驱动程序。举些例子：

MySQL: https://dev.mysql.com/downloads/connector/odbc/

MS SQL Server: https://www.microsoft.com/en-us/download/details.aspx?id=53339

PostgreSQL: https://www.postgresql.org/ftp/odbc/versions/msi/

* Configure an ODBC data source. For an example of configuring MySQL's ODBC data source, please refer to [MySQL manual](https://dev.mysql.com/doc/connector-odbc/en/connector-odbc-configuration-dsn-windows-5-2.html).

## 2. 编译


要编译插件，请使用以下命令

```
make
```

这会将插件编译到名为 ```libPluginODBC.so```。


## 3. 将插件加载到DolphinDB中


使用DolphinDB函数`loadPlugin`加载插件。它唯一的参数是插件描述文件。例如，下面的DolphinDB脚本会加载插件： ```PluginODBC.txt```:

```
loadPlugin("./plugins/odbc/PluginODBC.txt")
```

## 4. 使用插件

您可以通过使用语句```use odbc```导入ODBC模块名称空间来省略前缀“ odbc ::”。但是，如果函数名称与其他模块中的函数名称冲突，则需要在函数名称中添加前缀```odbc ::```。


```
use odbc;
```

该插件提供以下5个功能：


### 4.1 odbc::connect()

#### 语法
* odbc::connect(connStr, [dataBaseType])

#### 参数
* connStr: ODBC连接字符串。有关连接字符串格式的更多信息，请参阅 [连接字符串参考](https://www.connectionstrings.com). ODBC DSN必须由系统管理员创建。
有关连接字符串格式的更多信息，请参阅 [DSN连接字符串](https://www.connectionstrings.com/dsn/). 我们还可以创建到数据库的DSN-Less连接。
无需DSN的连接而不是依赖存储在文件或系统注册表中的信息，而是在连接字符串中指定驱动程序名称和所有特定于驱动程序的信息。例如: [SQL server的DSN-less连接字符串](https://www.connectionstrings.com/sql-server/) 
和[MySQL的DSN-less连接字符串](https://www.connectionstrings.com/mysql/). 
* dataBaseType: 数据库类型。 如"MYSQL", "SQLServer", "PostgreSQL"。

*** 请注意，驱动程序名称可能会有所不同，具体取决于安装的ODBC版本。

#### 描述


创建与数据库服务器的连接，返回数据库连接句柄，该句柄将在以后用于访问数据库服务器。

#### 例子
```
conn1 = odbc::connect("Dsn=mysqlOdbcDsn")  //mysqlOdbcDsn is the name of data source name
conn2 = conn1=odbc::connect("Driver={MySQL ODBC 8.0 UNICODE Driver};Server=127.0.0.1;Database=ecimp_ver3;User=newuser;Password=dolphindb123;Option=3;") 
conn3 = conn1=odbc::connect("Driver=SQL Server;Server=localhost;Database=zyb_test;User =sa;Password=DolphinDB123;")  
```

### 4.2 odbc::close()

#### 语法
* odbc::close(conn)

#### 参数
* conn: a connection handle created with `odbc::connect`.

#### 描述
关闭一个ODBC连接。

#### 例子
```
conn1 = odbc::connect("Dsn=mysqlOdbcDsn") 
odbc::close(conn1)
```

### 4.3 odbc::query()

#### 语法
* odbc::query(connHandle or connStr, querySql, [t])

#### 参数
* 第一个参数是连接句柄或连接字符串。  
* 第二个参数是查询SQL语句。
* 最后一个参数是用户提供的可选表。如果指定，查询结果将附加到表中。请注意，表模式必须与ODBC返回的结果兼容，否则将引发异常。

### 描述
```odbc::query``` 通过connHandle或connStr查询数据库并返回DolphinDB表。

#### 例子
```
t=odbc::query(conn1,"SELECT max(time),min(time) FROM ecimp_ver3.tbl_monitor;")
```

### 4.4 odbc::execute()

#### 语法
* odbc::execute(connHandle or connStr, SQLstatements)

#### 参数
* 第一个参数是连接句柄或连接字符串。
* 第二个参数是SQL语句。

#### 描述
```odbc::execute``` 执行SQL语句。无返回结果。

#### 例子
```
odbc::execute(conn1,"delete from ecimp_ver3.tbl_monitor where `timestamp` BETWEEN '2013-03-26 00:00:01' AND '2013-03-26 23:59:59'")
```  

### 4.5 odbc::append()

#### 语法
 * odbc::append(connHandle, tableData, tablename, [createTableIfNotExist], [insertIgnore])

#### 参数
* connHandle: 连接句柄。
* tableData: DolphinDB表。 
* tablename: 连接的数据库中表的名称。
* createTableIfNotExist: 布尔值。 True表示要创建一个新表。默认值是true。
* insertIgnore: 布尔值。 True如果有重复数据，会忽略插入。默认值为false。

#### 描述

将DolphinDB表附加到连接的数据库。

#### 例子
```
t=table(1..10 as id,take(now(),10) as time,rand(1..100,10) as value)
odbc::append(conn1, t,"ddbtale" ,true)
odbc::query(conn1,"SELECT * FROM ecimp_ver3.ddbtale")
```

## 5 类型支持
### 5.1 查询类型支持
| type in ODBC     | Type in DolphinDB|
| --------------------------- |-----------------|
|   SQL_BIT   | BOOL|
| SQL_TINYINT / SQL_SMALLINT | SHORT|
|SQL_INTEGER|INT|
|SQL_BIGINT|LONG|
|SQL_REAL|FLOAT|
|SQL_FLOAT/SQL_DOUBLE/SQL_DECIMAL/SQL_NUMERIC|DOUBLE|
|SQL_DATE/SQL_TYPE_DATE|DATE|
|SQL_TIME/SQL_TYPE_TIME|SECOND|
|SQL_TIMESTAMP/SQL_TYPE_TIMESTAMP|TIMESTAMP|
|SQL_CHAR(len == 1)|CHAR|
|SQL_CHAR(len <= 30)/SQL_VARCHAR(len <= 30)|SYMBOL|
|...|STRING|
### 5.2 转换类型支持
| type in DolphinDB     | Type in PostgreSQL|
| --------------------------- |-----------------|
|   BOOL   | bit|
|CHAR|char(1)|
|SHORT|smallint|
|INT|int|
|LONG|bigint|
|DATE|date|
|MONTH|date|
|TIME|time|
|MINUTE|time|
|SECOND|time|
|DATETIME|timestamp|
|TIMESTAMP|timestamp|
|NANOTIME|time|
NANOTIMESTAMP|timestamp|
|FLOAT|float|
|DOUBLE|double precision|
|SYMBOL|varchar(255)|
|STRING|varchar(255)|





