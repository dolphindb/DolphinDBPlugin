## DolphinDB ODBC plugin

With this plugin, you can easily pull your data from existing databases that support ODBC interface.

## Prerequisites
This plugin requires corresponding database drivers for ODBC. 

### Ubuntu
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

### CentOS
```
# install unixODBC library
yum install unixODBC  unixODBC-devel

# MySQL ODBC Drivers
yum install mysql-connector
```

Sometimes it will raise **libodbc.so.1: cannot open shared object file: No such file or directory** when you use loadPlugin to load the lib file.

To solve this problem:
```
cd /usr/lib64
ln -s libodbc.so.2.0.0.0 libodbc.so.1
```
The lib file maybe named like ```libodbc.so.2.0.0```,  you should find the correct file name to use.

### windows
* Download and install odbc driver for mysql or other databases from their corresponding websites. For example, the download address of mysql and ms sqlserver is as follows:

mysql:https://dev.mysql.com/downloads/connector/odbc/ . 

MS sqlserver: https://www.microsoft.com/en-us/download/details.aspx?id=53339 .

PostgreSQL:https://www.postgresql.org/ftp/odbc/versions/msi/ .

* Configuring an ODBC Data Source. For an example of configuring MySQL's ODBC data source, see [its manual](https://dev.mysql.com/doc/connector-odbc/en/connector-odbc-configuration-dsn-windows-5-2.html)

## Compilation
To compile the plugin, type following command in a shell.
```
make
```
This will compile the plugin into a shared library named ```  libPluginODBC.so```. 

## Loading plugin into DolphinDB
You can use dolphindb's ``` loadPlugin ``` function to load the plugin. This function takes a plugin description file parameter. For example, the follwing dolphindb script loads the plugin described by ```PluginODBC.txt```.
```
loadPlugin("./plugins/odbc/PluginODBC.txt")
```

## Using the plugin
The plugin provides five functions.

### odbc::connect()
#### Syntax
* odbc::connect(connStr)
#### Parameters
* connStr: an ODBC connection string. For more information regarding the format of connection string, see [The Connection Strings Reference](https://www.connectionstrings.com) ;
You can access the database through odbc DSN or through DSN-Less.DSN ODBC requires the system administrator to create an ODBC DSN.Their connection strings can be referenced [DSN connection strings](https://www.connectionstrings.com/dsn/).Rather than relying on information stored in a file or in the system registry, DSN-less connections specify the driver name, and all driver-specific information in the connection string. For example, sql server's dsn-less connection string is shown [here](https://www.connectionstrings.com/sql-server/)，MySql's dsn-less connection string is shown [here](https://www.connectionstrings.com/mysql/).

note that the driver name could be different depending on the version of ODBC is installed. e.g. 

#### Details
* Create a connection to the database server. Return a handle of database connection, which will be used to access the database server later.
#### Example
```
conn1 = odbc::connect("Dsn=mysqlOdbcDsn")  //mysqlOdbcDsn is the name of data source name
conn2 = conn1=odbc::connect("Driver={MySQL ODBC 8.0 UNICODE Driver};Server=127.0.0.1;Database=ecimp_ver3;User=newuser;Password=dolphindb123;Option=3;") 
conn3 = conn1=odbc::connect("Driver=SQL Server;Server=localhost;Database=zyb_test;User =sa;Password=DolphinDB123;")  
```
### odbc::close()
#### Syntax
* odbc::close(conn)
#### Parameters
* conn:a connection handle created with `odbc::connect`.
#### Details
* closes an odbc connection associated with the handle.
#### Example
```
conn1 = odbc::connect("Dsn=mysqlOdbcDsn") 
odbc::close(conn1)
```
### odbc::query()
#### Syntax
* odbc::query(connHandle or connStr, querySql [,t])
#### Parameters
* The first argument could be connection handle or a connection string.   
* The second argument is the query string.  
* The last argument is a optional user-provided table. If provided, query results will be appended to the table. Note that, the table schema must be compatible with the results returned from ODBC or an exception will be thrown.  

### Details
```odbc::query``` queries the database via connHandle or connStr and return the results as a dolphindb table.  
#### Example
```
t=odbc::query(conn1,"SELECT max(time),min(time) FROM ecimp_ver3.tbl_monitor;")
```
### odbc::execute()
#### Syntax
* odbc::execute(connHandle or connStr, SQLstatements)
#### Parameters
* The first argument could be connection handle or a connection string.    
* The second argument is the SQL statements.   
#### Details
* ```odbc::execute``` execute the SQL statements with no return. 
#### Example
```
odbc::execute(conn1,"delete from ecimp_ver3.tbl_monitor where `timestamp` BETWEEN '2013-03-26 00:00:01' AND '2013-03-26 23:59:59'")
```  
 ### odbc::append(connHandle, tableData, tablename, [createTableIfNotExist], [insertIgnore])
 #### Syntax
 * odbc::append(connHandle, tableData, tablename, [createTableIfNotExist], [insertIgnore])
 #### Parameters
```odbc::append```  append the table from dolphindb to the database which you connect to.
* `connHandle` : connection handle    
* `tableData`  :  the table object in dolphindb.    
* `tablename`  : the tablename in database which you connect to.  
* `[createTableIfNotExist]` : optional, true means create a new table in database. The default value is `true`.
* `[insertIgnore]` : optional, true means use insert ignore. The default value is `false`.
#### Details
* ```odbc::append``` append the table from dolphindb to the database which you connect to.
#### Example
```
t=table(1..10 as id,take(now(),10) as time,rand(1..100,10) as value)
odbc::append(conn1, t,"ddbtale" ,true)
odbc::query(conn1,"SELECT * FROM ecimp_ver3.ddbtale")
```

You can ommit ```odbc::``` prefix by importing obdc module namespace with statement "use odbc". However, if the function name conflicts with a function name from a different module, you need to add prefix odbc:: to the function name.

```
use odbc;
```