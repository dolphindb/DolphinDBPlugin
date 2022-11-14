# DolphinDB ODBC plugin

通过 ODBC Plugin，可以将其它数据源数据导入到 DolphinDB 数据库，或将 DolphinDB 内存表追加到其它数据库。

## 1. Prerequisites

ODBC 插件支持的数据源见下表：

| 需要连接的数据库 | 支持情况                                                     | 备注 |
| ---------------- | ------------------------------------------------------------ | ---- |
| MySQL          | centos7 稳定，连接时指定 “MYSQL”                         |      |
| PostgreSQL     | centos7 稳定，连接时指定 “PostgreSQL”                    |      |
| SQLServer      | centos7 稳定，连接时指定 “SQLServer”                     |      |
| Clickhouse     | centos7 稳定，连接时指定 “Clickhouse”    |      |
| SQLite         | centos7 稳定，连接时指定 “SQLite”                                               |      |
| Oracle         | centos7 稳定，连接时指定 “Oracle”                                              |      |

使用该插件前，请根据操作系统和数据库安装相应的 ODBC 驱动。

### 1.1 Ubuntu
```
# 安装 unixODBC 库
apt-get install unixodbc unixodbc-dev

# 安装 SQL Server ODBC 驱动
apt-get install tdsodbc

# 安装 PostgreSQL ODBC 驱动
apt-get install odbc-postgresql

# 安装 MySQL ODBC 驱动
apt-get install libmyodbc

# 安装 SQLite ODBC 驱动
apt-get install libsqliteodbc
```

### 1.2 CentOS
```
# 安装 unixODBC 库
yum install unixODBC  unixODBC-devel

# 安装 MySQL ODBC 驱动
yum install mysql-connector
```

### 1.3 Windows

* 从官网下载并安装 MySQL 或其他数据库的 ODBC 驱动程序。示例如下：

MySQL: https://dev.mysql.com/downloads/connector/odbc/

MS SQL Server: https://www.microsoft.com/en-us/download/details.aspx?id=53339

PostgreSQL: https://www.postgresql.org/ftp/odbc/versions/msi/

* Configure an ODBC data source. For an example of configuring MySQL's ODBC data source, please refer to [MySQL manual](https://dev.mysql.com/doc/connector-odbc/en/connector-odbc-configuration-dsn-windows-5-2.html).

## 2. 编译

### 2.1 编译unixODBC-2.3.11
```
wget https://src.fedoraproject.org/repo/pkgs/unixODBC/unixODBC-2.3.11.tar.gz/sha512/dddc32f90a7962e6988e1130a8093c6fb8b9ff532cad270d572250324aecbc739f45f9d8021d217313910bab25b08e69009b4f87456575535e93be1f46f5f13d/unixODBC-2.3.11.tar.gz
tar -zxvf unixODBC-2.3.11.tar.gz
LDFLAGS="-lrt" CFLAGS="-fPIC"  ./configure --prefix=/hdd1/gitlab/DolphinDBPlugin/unixodbc2.3.11Lib --enable-static=yes --enable-shared=no --sysconfdir=/etc/ --with-included-ltdl=yes
make -j
make install
```

### 2.2 编译odbc插件
```
cd <plugin_odbc_dir>
mkdir build
cd build
cmake ..  -DUNIXODBCDIR=/hdd1/gitlab/DolphinDBPlugin/unixodbc2.3.11Lib
make -j
```

编译生成插件 ```libPluginODBC.so```。

### 2.3 编译freetds odbc
```
wget -c http://ibiblio.org/pub/Linux/ALPHA/freetds/stable/freetds-stable.tgz
./configure --prefix=/usr/local/freetds --with-tdsver=8.0 --enable-msdblib
make -j
make install
```

如果需要将编译好的freetds odbc放到其他机器上使用，需要将/usr/local/freetds/lib下的freetds.conf, locales.conf, pool.conf放到目标机器上的/usr/local/freetds/lib目录下，需要将/usr/local/freetds/lib/ibtdsodbc.so.0.0.0拷到目标机器的/usr/local/freetds/lib目录下。


## 3. 将插件加载到 DolphinDB 中


使用 DolphinDB 函数 `loadPlugin` 加载插件。它唯一的参数是插件描述文件。例如，下面的 DolphinDB 脚本会加载插件：```PluginODBC.txt```:

```
loadPlugin("./plugins/odbc/PluginODBC.txt")
```

## 4. 使用插件

您可以通过使用语句 ```use odbc``` 导入 ODBC 模块名称空间来省略前缀 “odbc ::”。但是，如果函数名称与其他模块中的函数名称冲突，则需要在函数名称中添加前缀 ```odbc ::```。

```
use odbc;
```

该插件提供以下5个功能：

### 4.1 odbc::connect

**语法**
* odbc::connect(connStr, [dataBaseType])

**参数**
* connStr: ODBC 连接字符串。有关连接字符串格式的更多信息，请参阅 [连接字符串参考](https://www.connectionstrings.com)。ODBC DSN 必须由系统管理员创建。
有关 DNS 连接字符串的更多信息，请参阅 [DSN连接字符串](https://www.connectionstrings.com/dsn/)。我们还可以创建到数据库的 DSN-Less 连接。
无需依赖存储在文件或系统注册表中的信息，而是在连接字符串中指定驱动程序名称和所有特定于驱动程序的信息。例如: [SQL server 的 DSN-less 连接字符串](https://www.connectionstrings.com/sql-server/)和[MySQL 的 DSN-less 连接字符串](https://www.connectionstrings.com/mysql/)。
* dataBaseType: 数据库类型。如"MySQL", "SQLServer", "PostgreSQL", "ClickHouse", "SQLite", "Oracle" 不区分大小写。建议连接时指定该参数，否则写入数据时可能出现报错。

**请注意**

* 驱动程序名称可能会有所不同，具体取决于安装的 ODBC 版本。
* 若数据库连接的端口指定错误，则会出现 server crash。
* 必须通过 DSN 方式连接 Oracle 数据源，否则连接时用户名和密码可能校验失败；若修改 `/etc/odbc.ini` 中 DSN 配置的 database 和 password，则需要在 Oracle 命令行中 commit 后才能通过新配置进行连接（也可通过 isql 命令行工具进行验证验证配置是否生效）。
* 通过 freeTDS 访问数据库时，必须保证 freetds.conf 中的 DSN 配置信息正确，否则可能出现 freeTDS crash 的情况。

**描述**

创建与数据库服务器的连接，返回数据库连接句柄，该句柄将在以后用于访问数据库服务器。

**例子**
```
conn1 = odbc::connect("Dsn=mysqlOdbcDsn")  //mysqlOdbcDsn is the name of data source name
conn2 = odbc::connect("Driver={MySQL ODBC 8.0 UNICODE Driver};Server=127.0.0.1;Database=ecimp_ver3;User=newuser;Password=dolphindb123;Option=3;") 
conn3 = odbc::connect("Driver=SQL Server;Server=localhost;Database=zyb_test;User =sa;Password=DolphinDB123;")  
```

### 4.2 odbc::close

**语法**
* odbc::close(conn)

**参数**
* conn: 由 odbc::connect 创建的连接句柄。

**描述**
关闭一个 ODBC 连接。

**例子**
```
conn1 = odbc::connect("Dsn=mysqlOdbcDsn") 
odbc::close(conn1)
```

### 4.3 odbc::query

**语法**
* odbc::query(connHandle or connStr, querySql, [t], [batchSize], [tranform])

**参数**
* connHandle or connStr: 连接句柄或连接字符串。  
* querySql: 表示查询的 SQL 语句。
* t：表对象。若指定，查询结果将保存到该表中。请注意，t 的各字段类型必须与 ODBC 返回的结果兼容（见第5节类型支持），否则将引发异常。
* batchSize: 从 ODBC 查询到的数据行数到达 batchSize 后，会将当前已经读到的数据 append 到表 t 中。默认值为 262,144。
* tranform: 一元函数，并且该函数接受的参数必须是一个表。如果指定了 transform 参数，需要先创建分区表，再加载数据。程序会对数据文件中的数据应用 transform 参数指定的函数后，将得到的结果保存到数据库中。

### 描述
```odbc::query``` 通过 connHandle 或 connStr 查询数据库并返回 DolphinDB 表。

**例子**
```
t=odbc::query(conn1,"SELECT max(time),min(time) FROM ecimp_ver3.tbl_monitor;")
```

### 4.4 odbc::execute

**语法**
* odbc::execute(connHandle or connStr, SQLstatements)

**参数**
* connHandle or connStr: 连接句柄或连接字符串。
* SQLstatements: SQL 语句。

**描述**
```odbc::execute``` 执行 SQL 语句。无返回结果。

**例子**
```
odbc::execute(conn1,"delete from ecimp_ver3.tbl_monitor where `timestamp` BETWEEN '2013-03-26 00:00:01' AND '2013-03-26 23:59:59'")
```  

### 4.5 odbc::append

**语法**
 * odbc::append(connHandle, tableData, tablename, [createTableIfNotExist], [insertIgnore])

**参数**
* connHandle: 连接句柄。
* tableData: DolphinDB 表。 
* tablename: 连接的数据库中表的名称。
* createTableIfNotExist: 布尔值。true 表示要创建一个新表。默认值是 true。
* insertIgnore: 布尔值。true 表示不插入重复数据。默认值为 false。

**描述**

将 DolphinDB 表追加到连接的数据库。

**例子**
```
t=table(1..10 as id,take(now(),10) as time,rand(1..100,10) as value)
odbc::append(conn1, t,"ddbtale", true)
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
|SQL_TIMESTAMP/SQL_TYPE_TIMESTAMP|NANOTIMESTAMP|
|SQL_CHAR(len == 1)|CHAR|
|其它类型|STRING|
### 5.2 转换类型支持
| DolphinDB     |  PostgreSQL| ClickHouse|Oracle| SQL Server|SQLite|MySQL|
| --------------------------- |-----------------|----|--|--|--|--|
|   BOOL   | boolean|Bool|char(1) | bit| bit| bit|
|CHAR|char(1)
|SHORT|smallint|
|INT|int|
|LONG|bigint|bigint|number|bigint|bigint|bigint
|DATE|date|
|MONTH|date|
|TIME|time|
|MINUTE|time|
|SECOND|time|
|DATETIME|timestamp|datetime64|date|datetime|datetime|datetime|
|TIMESTAMP|timestamp|datetime64|timestamp|datetime|datetime|datetime|
|NANOTIME|time|
NANOTIMESTAMP|timestamp|datetime64|timestamp|datetime|datetime|datetime|
|FLOAT|float|float|float|float(24)|float|float|
|DOUBLE|double precision|double|binary_double|float(53)|double|double|
|SYMBOL|varchar(255)|varchar(255)|varchar(255)|varchar(255)|varchar(255)|varchar(255)|
|STRING|varchar(255)|varchar(255)|varchar(255)|varchar(255)|varchar(255)|varchar(255)|

## 5 问题分析与解决

1. 连接 windows 系统的 ClickHouse，查询得到的结果显示中文乱码。

   **解决方案：** 请选择 ANSI 的 ClickHouser ODBC 驱动。

2. 连接 ClickHouse 并读取数据时，datetime 类型数据返回空值或错误值。
   
   **原因：** 低于 1.1.10 版本的 ClickHouse 的 ODBC 驱动将 datetime 返回为字符串类型，且返回的数据长度错误（长度过短），导致 ODBC 插件无法读取正确的字符串。

   **解决方案：** 请更新驱动到不小于1.1.10的版本。