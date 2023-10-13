# DolphinDB ODBC plugin

With DolphinDB ODBC plugin, you can import data from databases that support ODBC interface.

The DolphinDB ODBC plugin has the branches [release 200](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/odbc/README.md) and [release130](https://github.com/dolphindb/DolphinDBPlugin/blob/release130/odbc/README.md). Each plugin version corresponds to a DolphinDB server version. If you use a different DolphinDB server version, please refer to the corresponding branch of the plugin documentation.

- [DolphinDB ODBC plugin](#dolphindb-odbc-plugin)
  - [1. Prerequisites](#1-prerequisites)
    - [1.1 Ubuntu](#11-ubuntu)
    - [1.2 CentOS](#12-centos)
    - [1.3 Windows](#13-windows)
  - [2. Compilation](#2-compilation)
    - [2.1 Compile unixODBC-2.3.11](#21-compile-unixodbc-2311)
    - [2.2 Compile the ODBC Plugin](#22-compile-the-odbc-plugin)
    - [2.3 Compile freetds odbc](#23-compile-freetds-odbc)
  - [3. Load the plugin](#3-load-the-plugin)
  - [4. Methods](#4-methods)
    - [4.1 odbc::connect](#41-odbcconnect)
    - [4.2 odbc::close](#42-odbcclose)
    - [4.3 odbc::query](#43-odbcquery)
    - [4.4 odbc::execute](#44-odbcexecute)
    - [4.5 odbc::append](#45-odbcappend)
  - [5 Data Type Mappings](#5-data-type-mappings)
    - [5.1 For Queries](#51-for-queries)
    - [5.2 For Data Conversion](#52-for-data-conversion)
  - [6. FAQ](#6-faq)

## 1. Prerequisites

The ODBC plugin supports the following databases stably in CentOS 7: MySQL, PostgreSQL, SQLServer, Clickhouse, SQLite, Oracle. 
> When connecting, you need to specify the database name, such as "MySQL".

Install the ODBC driver that corresponds to your operating system and database. 

### 1.1 Ubuntu
```
# install unixODBC library
apt-get install unixodbc unixodbc-dev

# SQL Server ODBC Drivers
apt-get install tdsodbc

# PostgreSQL ODBC ODBC Drivers
apt-get install odbc-postgresql

# MySQL ODBC Drivers
apt-get install libmyodbc

# SQLite ODBC Drivers
apt-get install libsqliteodbc
```

### 1.2 CentOS
```
# install unixODBC library
yum install unixODBC  unixODBC-devel

# MySQL ODBC Drivers
yum install mysql-connector
```

### 1.3 Windows

* Download and install odbc driver for mysql or other databases from their websites. For examples:

MySQL: https://dev.mysql.com/downloads/connector/odbc/

MS SQL Server: https://www.microsoft.com/en-us/download/details.aspx?id=53339

PostgreSQL: https://www.postgresql.org/ftp/odbc/versions/msi/

* Configure an ODBC data source. For an example of configuring MySQL's ODBC data source, please refer to [MySQL manual](https://dev.mysql.com/doc/connector-odbc/en/connector-odbc-configuration-dsn-windows-5-2.html).

## 2. Compilation

### 2.1 Compile unixODBC-2.3.11

```
wget https://src.fedoraproject.org/repo/pkgs/unixODBC/unixODBC-2.3.11.tar.gz/sha512/dddc32f90a7962e6988e1130a8093c6fb8b9ff532cad270d572250324aecbc739f45f9d8021d217313910bab25b08e69009b4f87456575535e93be1f46f5f13d/unixODBC-2.3.11.tar.gz
tar -zxvf unixODBC-2.3.11.tar.gz
LDFLAGS="-lrt" CFLAGS="-fPIC"  ./configure --prefix=/hdd1/gitlab/DolphinDBPlugin/unixodbc2.3.11Lib --enable-static=yes --enable-shared=no --sysconfdir=/etc/ --with-included-ltdl=yes
make -j
make install
```
 
### 2.2 Compile the ODBC Plugin

```
cd <plugin_odbc_dir>
mkdir build
cd build
cmake ..  -DUNIXODBCDIR=/hdd1/gitlab/DolphinDBPlugin/unixodbc2.3.11Lib
make -j
```
The plugin is compiled to a shared library named "libPluginODBC.so".

### 2.3 Compile freetds odbc

To connect to a SQL server database, you need to compile freetds odbc.

```
wget -c http://ibiblio.org/pub/Linux/ALPHA/freetds/stable/freetds-stable.tgz
tar -zxvf freetds-stable.tgz
cd freetds
./configure --prefix=/usr/local/freetds --with-tdsver=8.0 --enable-msdblib
make -j
make install
```

If the plugin is not running on the machine where it is compiled , copy the compiled freetds to the target machine:

* Copy "freetds.conf", "locales.conf", "pool.conf" under */usr/local/freetds/lib* to the */usr/local/freetds/lib* directory of the target machine;
* Copy */usr/local/freetds/lib/ibtdsodbc.so.0.0.0* to the */usr/local/freetds/lib* directory of the target machine.

## 3. Load the plugin

Use DolphinDB function `loadPlugin` to load the plugin. Its sole parameter is a plugin description file. For example, the follwing DolphinDB script loads the plugin ```PluginODBC.txt```:
```
loadPlugin("./plugins/odbc/PluginODBC.txt")
```

Note that if you load plugin on a Windows OS, you must specify an absolute path and replace "\\" with "\\\\" or "/".

## 4. Methods

You can omit prefix ```odbc::``` by importing ODBC module namespace with statement "use odbc". However, if the function name conflicts with a function name from a different module, you need to add prefix ```odbc::``` to the function name.

```
use odbc;
```

In detail, the plugin provides the following 5 functions:

### 4.1 odbc::connect

**Syntax**
* odbc::connect(connStr, [dataBaseType])

**Parameters**
* connStr: an ODBC connection string. For more information regarding the format of the connection string, refer to [the Connection Strings Reference](https://www.connectionstrings.com). ODBC DSN must be created by the system administrator. Its connection strings can be referenced [DSN connection strings](https://www.connectionstrings.com/dsn/). We can also create DSN-Less connections to the database. Rather than relying on information stored in a file or in the system registry, DSN-less connections specify the driver name and all driver-specific information in the connection string. For examples: [SQL server's DSN-less connection string](https://www.connectionstrings.com/sql-server/) and [MySQL's DSN-less connection string](https://www.connectionstrings.com/mysql/). Please note that the driver name could be different depending on the version of ODBC installed.
* dataBaseType: the type of the database, e.g., "MYSQL", "SQLServer", "PostgreSQL". It is recommended to specify this parameter to avoid errors when writing data.

> **Note:**
> * The driver name could be different depending on the installed ODBC version.
> * If the database server port is not specified correctly, a server crash will occur.
> * You must connect to the Oracle database using Data Source Name (DSN), otherwise the user name and password validation may fail. If you change the database and password configured by DSN in */etc/odbc.ini*, you need to commit the new configuration at the Oracle command prompt before you can connect via the new configuration (command line tool isql can also be used to verify whether the new configuration takes effect).
> * When accessing the database via freeTDS, ensure that the DSN configuration in freetds.conf is correct, otherwise a freeTDS crash may occur.

**Details**

Create a connection to the database server. Return a database connection handle, which will be used to access the database server later.

**Example**
```
conn1 = odbc::connect("Dsn=mysqlOdbcDsn")  //mysqlOdbcDsn is the name of data source name
conn2 = odbc::connect("Driver={MySQL ODBC 8.0 UNICODE Driver};Server=127.0.0.1;Database=ecimp_ver3;User=newuser;Password=dolphindb123;Option=3;") 
conn3 = odbc::connect("Driver=SQL Server;Server=localhost;Database=zyb_test;User =sa;Password=DolphinDB123;")  
```

### 4.2 odbc::close

**Syntax**
* odbc::close(conn)

**Parameters**
* conn: a connection handle created with `odbc::connect`.

**Details**

Close an ODBC connection.

**Example**
```
conn1 = odbc::connect("Dsn=mysqlOdbcDsn") 
odbc::close(conn1)
```

### 4.3 odbc::query

**Syntax**
* odbc::query(connHandle|connStr, querySql, [t], [batchSize], [transform])

**Parameters**
* connHandle|connStr: the connection handle or the connection string.    
* querySql: a string indicating the query.
* t: a optional user-provided table. If specified, query results will be appended to the table. Note that the table schema must be compatible with the results returned from ODBC or an exception will be thrown.
* batchSize: When the number of rows queried from ODBC reaches *batchSize*, the currently loaded data is appended to table t. The default value is 262,144.
* transform: a unary function and the input argument must be a table. If it is specified, a partitioned table must be created before loading the file. The method will first apply the specified function to the data, and then save the result to the partitioned table.

**Details**

```odbc::query``` queries the database via connHandle or connStr and return a DolphinDB table.

**Example**
```
t=odbc::query(conn1,"SELECT max(time),min(time) FROM ecimp_ver3.tbl_monitor;")
```

### 4.4 odbc::execute

**Syntax**
* odbc::execute(connHandle|connStr, SQLstatements)

**Parameters**
* connHandle|connStr: the connection handle or the connection string.    
* SQLstatements: the SQL statements.   

**Details**

```odbc::execute``` executes the SQL statements. It returns nothing. 

**Example**
```
odbc::execute(conn1,"delete from ecimp_ver3.tbl_monitor where `timestamp` BETWEEN '2013-03-26 00:00:01' AND '2013-03-26 23:59:59'")
```  

### 4.5 odbc::append

**Syntax**
 * odbc::append(connHandle, tableData, tablename, [createTableIfNotExist], [insertIgnore])


**Parameters**
* connHandle: connection handle.
* tableData: a table in DolphinDB.    
* tablename: the name of the table in the connected database.  
* createTableIfNotExist: a Boolean value. True means a new table is to be created. The default value is true.
* insertIgnore: a Boolean value. True means to use insert ignore. The default value is false.

**Details**

Append a DolphinDB table to the connected database.

**Example**
```
t=table(1..10 as id,take(now(),10) as time,rand(1..100,10) as value)
odbc::append(conn1, t,"ddbtale" ,true)
odbc::query(conn1,"SELECT * FROM ecimp_ver3.ddbtale")
```

## 5. Data Type Mappings
### 5.1 For Queries
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
|other types|STRING|

### 5.2 For Data Conversion
| DolphinDB     |  PostgreSQL| ClickHouse|Oracle| SQL Server|SQLite|MySQL|
| --------------------------- |-----------------|----|--|--|--|--|
|   BOOL   | boolean|Bool|char(1) | bit| bit| bit|
|CHAR|char(1)|char(1) |char(1) |char(1) |char(1) |char(1) |
|SHORT|smallint|smallint|smallint|smallint|smallint|smallint|
|INT|int|int|int|int|int|int|
|LONG|bigint|bigint|number|bigint|bigint|bigint
|DATE|date|date|date|date|date|date|
|MONTH|date|date|date|date|date|date|
|TIME|time|time|time|time|time|time|
|MINUTE|time|time|time|time|time|time|
|SECOND|time|time|time|time|time|time|
|DATETIME|timestamp|datetime64|date|datetime|datetime|datetime|
|TIMESTAMP|timestamp|datetime64|timestamp|datetime|datetime|datetime|
|NANOTIME|time|time|time|time|time|time|
NANOTIMESTAMP|timestamp|datetime64|timestamp|datetime|datetime|datetime|
|FLOAT|float|float|float|float(24)|float|float|
|DOUBLE|double precision|double|binary_double|float(53)|double|double|
|SYMBOL|varchar(255)|varchar(255)|varchar(255)|varchar(255)|varchar(255)|varchar(255)|
|STRING|varchar(255)|varchar(255)|varchar(255)|varchar(255)|varchar(255)|varchar(255)|

## 6. FAQ

**Problem:** When reading data from ClickHouse, data of datetime type returns null or wrong value.

**Cause:** Prior to version 1.1.10, ODBC driver for ClickHouse returns datetime as a string type with shorter length.

**Solution:** Update the driver to version 1.1.10 or higher.