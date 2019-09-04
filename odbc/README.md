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
  

You can ommit ```odbc::``` prefix by introducing obdc module namespace.
```
use odbc;
```