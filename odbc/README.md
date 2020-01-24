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


## Compilation
To compile the plugin, type following command in a shell.
```
make
```
This will compile the plugin into a shared library named ```  libPluginODBC.so```.

## Loading plugin into DolphinDB
You can use dolphindb's ``` loadPlugin ``` function to load the plugin. This function takes a plugin description file parameter. For example, the follwing dolphindb script loads the plugin described by ```DOlphinDBODBC.txt```.
```
loadPlugin("DolphinDBODBC.txt");
```

## Using the plugin
The plugin provides three functions.

### odbc::connect(connStr)
```odbc::connect``` takes a odbc connection string as the paramter and opens up a connection via ODBC with this string and return the connection handle to user. For more information regarding the format of connection string, see [https://www.connectionstrings.com/](https://www.connectionstrings.com/microsoft-sql-server-odbc-driver/) ;

### odbc::close(connHandle)
```odbc::close``` closes an odbc connection associated with the handle.

### odbc::query(connHandle or connStr, querySql [,t])
```odbc::query``` queries the database via connHandle or connStr and return the results as a dolphindb table.   
The first argument could be connection handle or a connection string.   
The second argument is the query string.  
The last argument is a optional user-provided table. If provided, query results will be appended to the table. Note that, the table schema must be compatible with the results returned from ODBC or an exception will be thrown.  

### odbc::execute(connHandle or connStr, SQLstatements)
```odbc::execute``` execute the SQL statements with no return.    
The first argument could be connection handle or a connection string.    
The second argument is the SQL statements.     

### odbc::append(connHandle, tableData, tablename, [createTableIfNotExist], [insertIgnore])
```odbc::append```  append the table from dolphindb to the database which you connect to.
* `connHandle` : connection handle    
* `tableData`  :  the table object in dolphindb.    
* `tablename`  : the tablename in database which you connect to.  
* `[createTableIfNotExist]` : optional, true means create a new table in database. The default value is `true`.
* `[insertIgnore]` : optional, true means use insert ignore. The default value is `false`.

You can ommit ```odbc::``` prefix by introducing obdc module namespace.
```
use odbc;
```