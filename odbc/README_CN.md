# DolphinDB ODBC 插件

通过 ODBC 插件，可以连接其它数据源，将数据导入到 DolphinDB 数据库，或将 DolphinDB 内存表导出到其它数据库。

- [DolphinDB ODBC 插件](#dolphindb-odbc-插件)
  - [1. 前置条件](#1-前置条件)
    - [1.1 Ubuntu](#11-ubuntu)
    - [1.2 CentOS](#12-centos)
    - [1.3 Windows](#13-windows)
    - [1.4 Docker 容器环境 （Alpine Linux）](#14-docker-容器环境-alpine-linux)
  - [2. Linux 环境下编译](#2-linux-环境下编译)
    - [2.1 编译 unixODBC-2.3.11](#21-编译-unixodbc-2311)
    - [2.2 编译 ODBC 插件](#22-编译-odbc-插件)
    - [2.3 编译 freetds odbc](#23-编译-freetds-odbc)
  - [3. 将插件加载到 DolphinDB 中](#3-将插件加载到-dolphindb-中)
  - [4. 使用插件](#4-使用插件)
    - [4.1 `odbc::connect`](#41-odbcconnect)
    - [4.2 `odbc::close`](#42-odbcclose)
    - [4.3 `odbc::query`](#43-odbcquery)
    - [描述](#描述)
    - [4.4 `odbc::execute`](#44-odbcexecute)
    - [4.5 `odbc::append`](#45-odbcappend)
  - [5 类型支持](#5-类型支持)
    - [5.1 查询类型支持](#51-查询类型支持)
    - [5.2 转换类型支持](#52-转换类型支持)
  - [6 问题分析与解决](#6-问题分析与解决)



## 1. 前置条件

ODBC 插件可以成功连接以下安装于 CentOS 7 的数据库：MySQL, PostgreSQL, SQLServer, Clickhouse, SQLite, Oracle。连接时需要指定数据库名称，如："MySQL"。

使用该插件前，请根据操作系统和数据库安装相应的 ODBC 驱动。

### 1.1 Ubuntu


- 安装 unixODBC 库

```
apt-get install unixodbc unixodbc-dev
```
- 安装 SQL Server ODBC 驱动

```
apt-get install tdsodbc
```

- 安装 PostgreSQL ODBC 驱动

```
apt-get install odbc-postgresql
```
- 安装 MySQL ODBC 驱动

```
apt-get install libmyodbc
```

- 安装 SQLite ODBC 驱动

```
apt-get install libsqliteodbc
```

### 1.2 CentOS

```
# 安装 unixODBC 库
yum install unixODBC  unixODBC-devel

# 安装 MySQL ODBC 驱动
yum install mysql-connector-odbc
```

### 1.3 Windows

1. 从官网下载并安装 MySQL 或其他数据库的 ODBC 驱动程序。示例如下：

- MySQL：https://dev.mysql.com/downloads/connector/odbc/
- MS SQL Server：https://www.microsoft.com/en-us/download/details.aspx?id=53339
- PostgreSQL：https://www.postgresql.org/ftp/odbc/versions/msi/

* 配置 ODBC 数据源。例如，在 Windows 操作系统中配置 MySQL 的 ODBC 数据源的操作指导，可以参考： [MySQL manual](https://dev.mysql.com/doc/connector-odbc/en/connector-odbc-configuration-dsn-windows-5-2.html).


### 1.4 Docker 容器环境 （Alpine Linux）

运行以下命令安装 unixODBC 库：

```
# 安装 unixODBC 库
apk add unixodbc
apk add unixodbc-dev
```

## 2. Linux 环境下编译

### 2.1 编译 unixODBC-2.3.11

推荐编译 2.3.11 版本的 unixODBC 库。

```
wget https://src.fedoraproject.org/repo/pkgs/unixODBC/unixODBC-2.3.11.tar.gz/sha512/dddc32f90a7962e6988e1130a8093c6fb8b9ff532cad270d572250324aecbc739f45f9d8021d217313910bab25b08e69009b4f87456575535e93be1f46f5f13d/unixODBC-2.3.11.tar.gz
tar -zxvf unixODBC-2.3.11.tar.gz
LDFLAGS="-lrt" CFLAGS="-fPIC"  ./configure --prefix=/hdd1/gitlab/DolphinDBPlugin/unixodbc2.3.11Lib --enable-static=yes --enable-shared=no --sysconfdir=/etc/ --with-included-ltdl=yes
make -j
make install
```

### 2.2 编译 ODBC 插件

运行以下命令编译并生成 ODBC 插件 `libPluginODBC.so`：

```
cd <plugin_odbc_dir>
mkdir build
cd build
cmake ..  -DUNIXODBCDIR=/hdd1/gitlab/DolphinDBPlugin/unixodbc2.3.11Lib
make -j
```


### 2.3 编译 freetds odbc

如需连接 SQLServer 数据源，则需要编译 freetds odbc：

```
wget -c http://ibiblio.org/pub/Linux/ALPHA/freetds/stable/freetds-stable.tgz
tar -zxvf freetds-stable.tgz
cd freetds
./configure --prefix=/usr/local/freetds --with-tdsver=8.0 --enable-msdblib
make -j
make install
```

若插件运行机器与编译机器不是同一个，则需要将编译好的 freetds 拷贝至运行机器上，即：

* 将 `/usr/local/freetds/lib`下的 freetds.conf, locales.conf, pool.conf 拷贝至到目标机器上的 `/usr/local/freetds/lib` 目录
* 将 `/usr/local/freetds/lib/ibtdsodbc.so.0.0.0` 拷贝至目标机器的 `/usr/local/freetds/lib` 目录。

> :bulb:**注意**：
>>若使用Docker容器（即使用Alpine Linux操作系统），直接使用 `apk add freetds` 安装的新版本 freetds odbc 可能会与DolphinDB ODBC插件产生冲突而无法正常使用，因此推荐用户按照本小节给出的步骤下载并手动编译 freetds odbc。

在 Alpine Linux 环境中编译 freetds odbc 前，需要先添加某些库以提供编译环境：

```
wget -c http://ibiblio.org/pub/Linux/ALPHA/freetds/stable/freetds-stable.tgz
tar -zxvf freetds-stable.tgz

# 添加依赖库
apk add gcc
apk add g++
apk add make
apk add linux-headers
```
然后运行以下命令进行编译：

```
# 编译
./configure --prefix=/usr/local/freetds --with-tdsver=8.0 --enable-msdblib --disable-libiconv --disable-apps
make -j
make install
```

## 3. 将插件加载到 DolphinDB 中

使用 DolphinDB 函数 `loadPlugin` 加载插件。它唯一的参数是插件描述文件。例如，下面的 DolphinDB 脚本会加载插件：``PluginODBC.txt``:

```
loadPlugin("./plugins/odbc/PluginODBC.txt")
```

> :bulb:**注意**：
>>- 若使用 Windows 插件，加载时必须指定绝对路径，且路径中使用 `\\\\` 或 `/` 代替 `\\`。
>>- 若在 Alpine Linux 环境中使用插件，加载时可能会出现无法找到依赖库的报错，需要在 DolphinDB 的 server 目录下添加软链接：
>>>>
>>>>  ```
>>>>  ln -s /usr/lib/libodbc.so.2 libodbc.so.1
>>>>  ```

## 4. 使用插件

您可以通过使用语句 `use odbc` 导入 ODBC 模块名称空间来省略前缀 “odbc ::”。但是，如果函数名称与其他模块中的函数名称冲突，则需要在函数名称中添加前缀 `odbc ::`。

```
use odbc;
```

该插件提供以下5个功能：

### 4.1 `odbc::connect`

**语法**

* odbc::connect(connStr, [dataBaseType])

**参数**

* connStr: ODBC 连接字符串。有关连接字符串格式的更多信息，请参阅 [连接字符串参考](https://www.connectionstrings.com)。ODBC DSN 必须由系统管理员创建。
  有关 DSN 连接字符串的更多信息，请参阅 [DSN连接字符串](https://www.connectionstrings.com/dsn/)。我们还可以创建到数据库的 DSN-Less 连接。
  无需依赖存储在文件或系统注册表中的信息，而是在连接字符串中指定驱动程序名称和所有特定于驱动程序的信息。例如: [SQL server 的 DSN-less 连接字符串](https://www.connectionstrings.com/sql-server/) 和[MySQL 的 DSN-less 连接字符串](https://www.connectionstrings.com/mysql/)。
* dataBaseType: 数据库类型。如"MySQL", "SQLServer", "PostgreSQL", "ClickHouse", "SQLite", "Oracle" 不区分大小写。建议连接时指定该参数，否则写入数据时可能出现报错。

> :bulb:**注意**：
>>* 驱动程序名称可能会有所不同，具体取决于安装的 ODBC 版本。
>>* 若数据库连接的端口指定错误，则会出现 server crash。
>>* 必须通过 DSN 方式连接 Oracle 数据源，否则连接时用户名和密码可能校验失败；若修改 `/etc/odbc.ini` 中 DSN 配置的 database 和 password，则需要在 Oracle 命令行中 commit 后才能通过新配置进行连接（也可通过 isql 命令行工具验证配置是否生效）。
>>* 通过 freeTDS 访问数据库时，必须保证 freetds.conf 中的 DSN 配置信息正确，否则可能出现 freeTDS crash 的情况。

**描述**

创建与数据库服务器的连接，返回数据库连接句柄，该句柄将在以后用于访问数据库服务器。

**例子**

```
conn1 = odbc::connect("Dsn=mysqlOdbcDsn")  //mysqlOdbcDsn is the name of data source name
conn2 = odbc::connect("Driver={MySQL ODBC 8.0 UNICODE Driver};Server=127.0.0.1;Database=ecimp_ver3;User=newuser;Password=dolphindb123;Option=3;") 
conn3 = odbc::connect("Driver=SQL Server;Server=localhost;Database=zyb_test;User =sa;Password=DolphinDB123;")  
```

### 4.2 `odbc::close`

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

### 4.3 `odbc::query`

**语法**

* odbc::query(connHandle|connStr, querySql, [t], [batchSize], [transform])

**参数**

* connHandle or connStr: 连接句柄或连接字符串。
* querySql: 表示查询的 SQL 语句。
* t：表对象。若指定，查询结果将保存到该表中。请注意，t 的各字段类型必须与 ODBC 返回的结果兼容（见第5节类型支持），否则将引发异常。
* batchSize: 从 ODBC 查询到的数据行数到达 batchSize 后，会将当前已经读到的数据 append 到表 t 中。默认值为 262,144。
* transform: 一元函数，并且该函数接受的参数必须是一个表。如果指定了 transform 参数，需要先创建分区表，再加载数据。程序会对数据文件中的数据应用 transform 参数指定的函数后，将得到的结果保存到数据库中。

### 描述

`odbc::query` 通过 connHandle 或 connStr 查询数据库并返回 DolphinDB 表。

**例子**

```
t=odbc::query(conn1,"SELECT max(time),min(time) FROM ecimp_ver3.tbl_monitor;")
```

### 4.4 `odbc::execute`

**语法**

* odbc::execute(connHandle or connStr, SQLstatements)

**参数**

* connHandle or connStr: 连接句柄或连接字符串。
* SQLstatements: SQL 语句。

**描述**
`odbc::execute` 执行 SQL 语句。无返回结果。

**例子**

```
odbc::execute(conn1,"delete from ecimp_ver3.tbl_monitor where `timestamp` BETWEEN '2013-03-26 00:00:01' AND '2013-03-26 23:59:59'")
```

### 4.5 `odbc::append`

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

| type in ODBC                                 | Type in DolphinDB |
| -------------------------------------------- | ----------------- |
| SQL_BIT                                      | BOOL              |
| SQL_TINYINT / SQL_SMALLINT                   | SHORT             |
| SQL_INTEGER                                  | INT               |
| SQL_BIGINT                                   | LONG              |
| SQL_REAL                                     | FLOAT             |
| SQL_FLOAT/SQL_DOUBLE/SQL_DECIMAL/SQL_NUMERIC | DOUBLE            |
| SQL_DATE/SQL_TYPE_DATE                       | DATE              |
| SQL_TIME/SQL_TYPE_TIME                       | SECOND            |
| SQL_TIMESTAMP/SQL_TYPE_TIMESTAMP             | NANOTIMESTAMP     |
| SQL_CHAR(len == 1)                           | CHAR              |
| 其它类型                                     | STRING            |

### 5.2 转换类型支持

| DolphinDB     | PostgreSQL       | ClickHouse   | Oracle        | SQL Server   | SQLite       | MySQL        |
| ------------- | ---------------- | ------------ | ------------- | ------------ | ------------ | ------------ |
| BOOL          | boolean          | Bool         | char(1)       | bit          | bit          | bit          |
| CHAR          | char(1)          | char(1)      | char(1)       | char(1)      | char(1)      | char(1)      |
| SHORT         | smallint         | smallint     | smallint      | smallint     | smallint     | smallint     |
| INT           | int              | int          | int           | int          | int          | int          |
| LONG          | bigint           | bigint       | number        | bigint       | bigint       | bigint       |
| DATE          | date             | date         | date          | date         | date         | date         |
| MONTH         | date             | date         | date          | date         | date         | date         |
| TIME          | time             | time         | time          | time         | time         | time         |
| MINUTE        | time             | time         | time          | time         | time         | time         |
| SECOND        | time             | time         | time          | time         | time         | time         |
| DATETIME      | timestamp        | datetime64   | date          | datetime     | datetime     | datetime     |
| TIMESTAMP     | timestamp        | datetime64   | timestamp     | datetime     | datetime     | datetime     |
| NANOTIME      | time             | time         | time          | time         | time         | time         |
| NANOTIMESTAMP | timestamp        | datetime64   | timestamp     | datetime     | datetime     | datetime     |
| FLOAT         | float            | float        | float         | float(24)    | float        | float        |
| DOUBLE        | double precision | double       | binary_double | float(53)    | double       | double       |
| SYMBOL        | varchar(255)     | varchar(255) | varchar(255)  | varchar(255) | varchar(255) | varchar(255) |
| STRING        | varchar(255)     | varchar(255) | varchar(255)  | varchar(255) | varchar(255) | varchar(255) |

## 6 问题分析与解决

1. 连接 Windows 系统的 ClickHouse，查询得到的结果显示中文乱码。

   **解决方法**： 请选择 ANSI 的 ClickHouse ODBC 驱动。

2. 连接 ClickHouse 并读取数据时，`datetime` 类型数据返回空值或错误值。

   **原因**： 低于 1.1.10 版本的 ClickHouse 的 ODBC 驱动将 `datetime` 返回为字符串类型，且返回的数据长度错误（长度过短），导致 ODBC 插件无法读取正确的字符串。

   **解决方法**： 更新驱动到不小于1.1.10的版本。

3. 使用 `yum install mysql-connector-odbc` 命令下载并安装 MySQL ODBC 驱动后，因驱动与 MySQL 数据源版本不一致而导致连接 MySQL 数据源时发生错误。

   **原因**：Yum 仓库未及时更新，通过 `yum install mysql-connector-odbc` 下载及安装的 MySQL ODBC 驱动与 MySQL 数据源的版本不一致。使用 `yum install mysql-connector-odbc` 会根据 Yum 仓库 （Yum Repository）的配置情况下载对应版本的 MySQL ODBC 驱动。当 MySQL 数据源的版本较新，例如 8.0 版本时，请确保您本地的 Yum 仓库配置亦为最新。因此，为避免连接 MySQL 数据源时出现连接超时或无法找到 *libodbc.so.1* 文件等错误，可以通过以下方法获取最新版本的 MySQL ODBC 驱动。

    **解决方法**：

    **方法1**：在运行 `yum install mysql-connector-odbc` 命令前，运行以下命令以确保 MySQL Yum 仓库为最新：

    ```
    $> su root
    $> yum update mysql-community-release
    ```

    有关更多 MySQL Yum 仓库的使用教程， 参考：[Installing Additional MySQL Products and Components with Yum](https://dev.mysql.com/doc/refman/8.0/en/linux-installation-yum-repo.html#yum-install-components)。

    **方法2**：下载指定版本的 MySQL ODBC 驱动，修改 */etc/odbc.ini* 文件后，修改 `conn` 对应语句。例如，当 MySQL 数据源版本为 8.0 时：

    1). 运行以下命令下载对应 MySQL 8.0 版本的 ODBC 驱动：

    ```
    wget https://dev.mysql.com/get/Downloads/Connector-ODBC/8.0/mysql-connector-odbc-8.0.32-1.el7.x86_64.rpm
    rpm -ivh mysql-connector-odbc-8.0.32-1.el7.x86_64.rpm
    rpm -ql mysql-connector-odbc-8.0.32-1.el7.x86_64
    ```

    2). 加载插件时，如遇到 `libodbc.so.1: cannot open shared object file: No such file or directory` 错误，说明依赖库无法找到，则在 *libodbc.so.2* 与 *libodbc.so.1* 之间建立软连接，然后重新加载插件。

    ```
    cd /usr/lib64
    ln -s libodbc.so.2 libodbc.so.1
    ```

    3). 复制 */etc/odbcinst.ini* 中 `[MySQL ODBC 8.0 Unicode Driver]` 下 `Driver` 的指定路径，例如：

    ```
    [MySQL ODBC 8.0 Unicode Driver]
    Driver=/usr/lib64/libmyodbc8w.so
    UsageCount=1
    ```

    4). 使用上一步复制的信息以及连接 MySQL 数据源所需的登录信息修改 */etc/odbc.ini*：

   **解决方法**： 请选择 ANSI 的 ClickHouse ODBC 驱动。

2. 连接 ClickHouse 并读取数据时，datetime 类型数据返回空值或错误值。

   **原因**： 低于 1.1.10 版本的 ClickHouse 的 ODBC 驱动将 datetime 返回为字符串类型，且返回的数据长度错误（长度过短），导致 ODBC 插件无法读取正确的字符串。

   **解决方法**： 更新驱动到不小于1.1.10的版本。

3. 使用 `yum install mysql-connector-odbc` 命令下载并安装 MySQL ODBC 驱动后，因驱动与 MySQL 数据源版本不一致而导致连接 MySQL 数据源时发生错误。

   **原因**：Yum 仓库未及时更新，通过 `yum install mysql-connector-odbc` 下载及安装的 MySQL ODBC 驱动与 MySQL 数据源的版本不一致。使用 `yum install mysql-connector-odbc` 会根据 Yum 仓库 （Yum Repository）的配置情况下载对应版本的 MySQL ODBC 驱动。当 MySQL 数据源的版本较新，例如 8.0 版本时，请确保您本地的 Yum 仓库配置亦为最新。因此，为避免连接 MySQL 数据源时出现连接超时或无法找到 *libodbc.so.1* 文件等错误，可以通过以下方法获取最新版本的 MySQL ODBC 驱动。

    **解决方法**：

    **方法1**：在运行 `yum install mysql-connector-odbc` 命令前，运行以下命令以确保 MySQL Yum 仓库为最新：

    ```
    $> su root
    $> yum update mysql-community-release
    ```

    有关更多 MySQL Yum 仓库的使用教程， 参考：[Installing Additional MySQL Products and Components with Yum](https://dev.mysql.com/doc/refman/8.0/en/linux-installation-yum-repo.html#yum-install-components)。

    **方法2**：下载指定版本的 MySQL ODBC 驱动，修改 */etc/odbc.ini* 文件后，修改 `conn` 对应语句。例如，当 MySQL 数据源版本为 8.0 时：

    1). 运行以下命令下载对应 MySQL 8.0 版本的 ODBC 驱动：

    ```
    wget https://dev.mysql.com/get/Downloads/Connector-ODBC/8.0/mysql-connector-odbc-8.0.32-1.el7.x86_64.rpm
    rpm -ivh mysql-connector-odbc-8.0.32-1.el7.x86_64.rpm
    rpm -ql mysql-connector-odbc-8.0.32-1.el7.x86_64
    ```

    2). 加载插件时，如遇到 `libodbc.so.1: cannot open shared object file: No such file or directory` 错误，说明依赖库无法找到，则在 *libodbc.so.2* 与 *libodbc.so.1* 之间建立软连接，然后重新加载插件。

    ```
    cd /usr/lib64
    ln -s libodbc.so.2 libodbc.so.1
    ```

    3). 复制 */etc/odbcinst.ini* 中 `[MySQL ODBC 8.0 Unicode Driver]` 下 `Driver` 的指定路径，例如：

    ```
    [MySQL ODBC 8.0 Unicode Driver]
    Driver=/usr/lib64/libmyodbc8w.so
    UsageCount=1
    ```

    4). 使用上一步复制的信息以及连接 MySQL 数据源所需的登录信息修改 */etc/odbc.ini*：

    ```
    [root@username/]# cat /etc/odbc.ini
    [mysql8]
    Description=ODBC for MySQL
    Driver=/usr/lib64/libmyodbc8w.so
    Server=172.17.0.10
    Port=3306
    Database=test1db
    User=root
    Password=123456
    ```

    5). 修改 `conn` 连接语句。

    ```
    conn = odbc::connect("Driver=MySQL ODBC 8.0 Unicode Driver;Server=172.17.0.10;Port=3306;Database=testdb;User=root;Password=123456;", "MySQL");
    ```
