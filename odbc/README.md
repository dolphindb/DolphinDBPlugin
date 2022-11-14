# DolphinDB ODBC plugin

With DolphinDB ODBC plugin, you can import data from databases that support ODBC interface.

The DolphinDB ODBC plugin has the branches [release 200](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/odbc/README.md) and [release130](https://github.com/dolphindb/DolphinDBPlugin/blob/release130/odbc/README.md). Each plugin version corresponds to a DolphinDB server version. You're looking at the plugin documentation for release200. If you use a different DolphinDB server version, please refer to the corresponding branch of the plugin documentation.

## 1. Prerequisites

This plugin requires the following database drivers for ODBC. 

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

When we use loadPlugin to load the lib file, we may have an error message **libodbc.so.1: cannot open shared object file: No such file or directory**.

Use the following script to solve the problem:
```
cd /usr/lib64
ln -s libodbc.so.2.0.0 libodbc.so.1
```
Please use the correct lib file name in place of ```libodbc.so.2.0.0```.

### 1.3 Windows

* Download and install odbc driver for mysql or other databases from their websites. For examples:

MySQL: https://dev.mysql.com/downloads/connector/odbc/

MS SQL Server: https://www.microsoft.com/en-us/download/details.aspx?id=53339

PostgreSQL: https://www.postgresql.org/ftp/odbc/versions/msi/

* Configure an ODBC data source. For an example of configuring MySQL's ODBC data source, please refer to [MySQL manual](https://dev.mysql.com/doc/connector-odbc/en/connector-odbc-configuration-dsn-windows-5-2.html).

## 2. Compilation

To compile the plugin, use the following command in a shell:
```
make
```
This will compile the plugin into a shared library named ```libPluginODBC.so```. 

## 3. Load the plugin into DolphinDB

Use DolphinDB function `loadPlugin` to load the plugin. Its sole parameter is a plugin description file. For example, the follwing DolphinDB script loads the plugin ```PluginODBC.txt```:
```
loadPlugin("./plugins/odbc/PluginODBC.txt")
```

## 4. Use the plugin

You can omit prefix ```odbc::``` by importing ODBC module namespace with statement "use odbc". However, if the function name conflicts with a function name from a different module, you need to add prefix ```odbc::``` to the function name.

```
use odbc;
```

In detail, the plugin provides the following 5 functions:

### 4.1 odbc::connect()

#### Syntax
* odbc::connect(connStr, [dataBaseType])

#### Parameters
* connStr: an ODBC connection string. For more information regarding the format of the connection string, please refer to [the Connection Strings Reference](https://www.connectionstrings.com). ODBC DSN must be created by the system administrator. Its connection strings can be referenced [DSN connection strings](https://www.connectionstrings.com/dsn/). We can also create DSN-Less connections to the database. Rather than relying on information stored in a file or in the system registry, DSN-less connections specify the driver name and all driver-specific information in the connection string. For examples: [SQL server's DSN-less connection string](https://www.connectionstrings.com/sql-server/) and [MySQL's DSN-less connection string](https://www.connectionstrings.com/mysql/). Please note that the driver name could be different depending on the version of ODBC installed.
* dataBaseType: the type of the database, e.g., "MYSQL", "SQLServer", "PostgreSQL". It is recommended to specify this parameter to avoid errors when writing data.

#### Details

Create a connection to the database server. Return a database connection handle, which will be used to access the database server later.

#### Example
```
conn1 = odbc::connect("Dsn=mysqlOdbcDsn")  //mysqlOdbcDsn is the name of data source name
conn2 = odbc::connect("Driver={MySQL ODBC 8.0 UNICODE Driver};Server=127.0.0.1;Database=ecimp_ver3;User=newuser;Password=dolphindb123;Option=3;") 
conn3 = odbc::connect("Driver=SQL Server;Server=localhost;Database=zyb_test;User =sa;Password=DolphinDB123;")  
```

### 4.2 odbc::close()

#### Syntax
* odbc::close(conn)

#### Parameters
* conn: a connection handle created with `odbc::connect`.

#### Details
Close an ODBC connection.

#### Example
```
conn1 = odbc::connect("Dsn=mysqlOdbcDsn") 
odbc::close(conn1)
```

### 4.3 odbc::query()

#### Syntax
* odbc::query(connHandle or connStr, querySql, [t])

#### Parameters
* The first argument is the connection handle or the connection string.    
* The second argument is a string indicating the query.
* The last argument is a optional user-provided table. If specified, query results will be appended to the table. Please note that the table schema must be compatible with the results returned from ODBC or an exception will be thrown.

### Details
```odbc::query``` queries the database via connHandle or connStr and return a DolphinDB table.

#### Example
```
t=odbc::query(conn1,"SELECT max(time),min(time) FROM ecimp_ver3.tbl_monitor;")
```

### 4.4 odbc::execute()

#### Syntax
* odbc::execute(connHandle or connStr, SQLstatements)

#### Parameters
* The first argument is the connection handle or the connection string.    
* The second argument is the SQL statements.   

#### Details
```odbc::execute``` executes the SQL statements. It returns nothing. 

#### Example
```
odbc::execute(conn1,"delete from ecimp_ver3.tbl_monitor where `timestamp` BETWEEN '2013-03-26 00:00:01' AND '2013-03-26 23:59:59'")
```  

### 4.5 odbc::append()

#### Syntax
 * odbc::append(connHandle, tableData, tablename, [createTableIfNotExist], [insertIgnore])


#### Parameters
* connHandle: connection handle.
* tableData: a table in DolphinDB.    
* tablename: the name of the table in the connected database.  
* createTableIfNotExist: a Boolean value. True means a new table is to be created. The default value is true.
* insertIgnore: a Boolean value. True means to use insert ignore. The default value is false.

#### Details
Append a DolphinDB table to the connected database.

#### Example
```
t=table(1..10 as id,take(now(),10) as time,rand(1..100,10) as value)
odbc::append(conn1, t,"ddbtale" ,true)
odbc::query(conn1,"SELECT * FROM ecimp_ver3.ddbtale")
```

## 5 Support Data Types
### 5.1 For Queries
| Type in ODBC     | Type in DolphinDB|
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

### 5.2 For Data Conversion
| Type in DolphinDB     | Type in PostgreSQL|
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

## 6 Supported Databases

| Database | Details                                                     |
| ---------------- | ------------------------------------------------------------ |
| MySQL            | stable on CentOS7. It is recommended to specify "MYSQL" for the *dataBaseType* parameter when setting up connection.                         |      
| PostgreSQL       | stable on CentOS7. It is recommended to specify "PostgreSQL" for the *dataBaseType* parameter when setting up connection.                    |      
| SQLServer        | stable on CentOS7. It is recommended to specify "PostgreSQL" for the *dataBaseType* parameter when setting up            |      |
| Clickhouse       | unixODBC-2.3.6 or higher version is required. Lower versions of unixODBC may cause the system to crash after the user enters a query statement with a syntax error.|      
| SQLite           | not recommended                                     |
| Oracle           | not recommended                                          |