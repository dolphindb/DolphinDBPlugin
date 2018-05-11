## DolphinDB ODBC plugin

With this plugin, you can easily import data from databases that support ODBC interface into DolphinDB.

## Prerequisites

### Linux(Ubuntu 16.04)
```
# install unixODBC library
sudo apt-get install unixodbc unixodbc-dev

# MS SQL Server ODBC drivers
sudo apt-get install tdsodbc

# PostgreSQL ODBC drivers
sudo apt-get install odbc-postgresql

# MySQL ODBC drivers
sudo apt-get install libmyodbc

# SQLite ODBC drivers
sudo apt-get install libsqliteodbc
```

### Windows

Download and install MS SQL SERVER, PostgreSQL, MySQL, or SQLite ODBC drivers from their corresponding websites.

### Regarding the location of ODBC plugin and its configuration file

Please make sure:

libODBC.so (Linux) or libODBC.dll (Windows) and odbc.cfg are under the folder DOLPHINDB_DIR/server/plugins/odbc/. 

The DolphinDB server executable and its dynamic library libDolphinDB.so (Linux) or libDolphinDB.dll(Windows) are under the folder DOLPHINDB_DIR/server/. 

## Getting started --- MySQL server as an example

This short guide will walk you through loading ODBC plugin and connecting to a MySQL server.

### Loading plugin into DolphinDB
You can use DolphinDB function "loadPlugin" to load the ODBC plugin. This function takes a plugin configuration file as input. For example, the following DolphinDB script loads the plugin configured by odbc.cfg:
```
loadPlugin("/DOLPHINDB_DIR/server/plugins/odbc/odbc.cfg")
```

### Built-in functions

#### odbc::connect(connStr)
Take an ODBC connection string as the parameter, open up a connection via ODBC with this string and return the connection handle to the user. For more information regarding the format of connection string, see [httpswww.connectionstrings.com](httpswww.connectionstrings.com) ;

#### odbc::close(connHandle)
Close an ODBC connection associated with the handle.
#### odbc::query(connHandle or connStr, querySql, [t])
 Query the database via connHandle or connStr and return the results as a DolphinDB table.

connHandle: a connection handle object 

connStr: an ODBC connection string

querySql: a SQL query string

t: a DolphinDB table. If it is specified, query results will be appended to the table. Please note that the table schema must be compatible with the results returned from ODBC.

You can omit the prefix odbc:: by importing ODBC module namespace with statement "use odbc". However, if the function name conflicts with a function name from a different module, you need to add prefix odbc:: to the function name.

#### odbc::execute(connHandle or connStr, querySql)

This function is basically the same as function "query", except that the sql statement is executed but no result is returned.

### Examples
```
// load DolphinDB ODBC plugin specified by the configuration file odbc.cfg
loadPlugin("/<DOLPHINDB_DIR>/server/server/plugins/odbc/odbc.cfg")

// import ODBC module. The module must be loaded before parsing and running the import script. Otherwise, the parser can't recognize the module name 'odbc'
use odbc

// define a DB connection string
connStr="Driver=MySQL;Data Source = mysql-employees;server=127.0.0.1;uid=[username];pwd=[password]database=employees";

// establish a DB connection
conn=connect(connStr)

// query table "employees"
mysqlTable=query(conn,"select * from employees") 
select * from mysqlTable
close(conn)
```

Please check odbc.ini and confirm the correct configuration.
```
$ cat /etc/odbc.ini
[mysql-employees]
Description           = MySQL connection to  database
Driver                = MySQL
Database              = employees
Server                = localhost
User                  = root
Password              = ******
Port                  = 3306
Socket                = /var/run/mysqld/mysqld.sock
```

## Compliation

If you need to modify and complie source code, compile and deploy as follows:

### download DolphinDB core library
ODBC plugin compliation depends on DolphinDB core library libDolphinDB.so .
Please download libDolphinDB.so from [http://www.dolphindb.com/downloads.html](http://www.dolphindb.com/downloads.html) ;

### compile 
To complie the plugin, type following command in a shell.

```
make
```
This will complie the plugin into a new shared library named libODBC.so . Replace the origin library under /<DOLPHINDB_DIR>/server/plugins/odbc/ .

### nanodbc

Part of the code implementation uses another open source project nanodbc. For details, please refer to
https://github.com/lexicalunit/nanodbc

