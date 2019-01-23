# DolphinDB MySQL Plugin

DolphinDB's MySQL plugin imports MySQL datasets or query results into DolphinDB. It supports data type conversion. Part of the plugin follows mysqlxx by Yandex.Clickhouse.


* [Build](#Build)
    * [Build with cmake](#Build with cmake)
* [User-API](#User API)
    * [mysql::connect](#mysqlConnect)
    * [mysql::showTables](#mysqlShowTables)
    * [mysql::extractSchema](#mysqlExtractSchema)
    * [mysql::load](#mysqlLoad)
    * [mysql::loadEx](#mysqlLoadEx)
* [Data Types](#Data Types)
    * [integer](#integer)
    * [float](#float)
    * [time](#time)
    * [string](#string)
    * [enum](#enum)
<!-- * [Performance](#Performance) -->


# Build

## Prerequisite

Install [git](https://git-scm.com/) and [CMake](https://cmake.org/).

For Ubuntu users, just type

```bash
$ sudo apt-get install git cmake
```

Then update submodule with the following script. This automatically downloads [mariadb-connector-c](https://github.com/MariaDB/mariadb-connector-c) source files.

```
$ git submodule update --init --recursive
```

## For Windows users (tested on Win10 64bit)

### Build with CMake and MinGW

**Note:** [cmake](https://cmake.org/) is a popular project build tool that helps solve third-party dependencies.

**Note:** [MinGW](http://www.mingw.org/), for "Minimalist GNU for Windows", is a minimalist development environment for native Microsoft Windows applications.

Build the project

```
mkdir build
cp libDolphinDB.dll build                 # copy libDolphinDB.dll to build directory
cp -r curl build                          # copy curl headers to build directory
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../path_to_mysql_plugin/ -G "MinGW Makefiles"
mingw32-make -j4
```

**Note:** Before compiling, copy `libDolphinDB.dll` and `curl` header folder to build directory.

## For Linux users

### Build with cmake
**Note:** [cmake](https://cmake.org/) is a popular project build tool that helps solve third-party dependencies.

Install cmake

```
sudo apt-get install cmake
```

Build the project

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../path_to_mysql_plugin/
make -j`nproc`
```

**Note:** Before compiling, please make sure that `libDolphinDB.so` is in the gcc searchable path. You can use `LD_LIBRARY_PATH` to specify its path.

The file libPluginMySQL.so will be generated after the compilation.

# User-API

**Remember:** Use `loadPlugin("/path_to_PluginMySQL.txt/PluginMySQL.txt")` to import MySQL plugin before using the API.

## mysql::connect

### Syntax

<!-- (host, user, password, port, [socket], [ssl_ca], [ssl_cert], [ssl_key]) -->
* `mysql::connect(host, user, password, port, db)`

### Parameters

* `host`: address of MySQL server. Data type `string`.
* `user`: user name of MySQL server. Data type `string`.
* `password`: password of MySQL server. Data type `string`.
* `port`: port of MySQL server. Data type `int`.
* `db`: database name. Data type `string`.

### Details

* Create a connection to MySQL server. Return a handle of MySQL connection, which will be used to access MySQL server later.

### Example

```
conn = mysql::connect(`localhost, `root, `root, 3306, `DolphinDB)
```

## mysql::showTables

### Syntax

* `mysql::showTables(connection)`

### Parameters

* `connection`: a MySQL connection handle created with `mysql::connect`.

### Details

* List all table names in a MySql database specified in `mysql::connect`.

### Example

```
conn = mysql::connect(`localhost, `root, `root, 3306, `DolphinDB)
mysql::showTables(conn)

output:
  Tables_in_DolphinDB
  -------------------
  US
```

## mysql::extractSchema

### Syntax

* `mysql::extractSchema(connection, tableName)`

### Parameters
* `connection`: a MySQL connection handle created with `mysql::connect`.
* `tableName`: the name of a table in MySQL server. Data type `string`.

### Details
* Generate the schema of a table.

### Example

```
conn = mysql::connect(`localhost, `root, `root, 3306, `DolphinDB)
mysql::extractSchema(conn, `US)

output:
        Name    MySQLType   DolphinDBType
        PERMNO  int(11)     INT
        date    date        DATE
        SHRCD   int(11)     INT
        TICKER  varchar(10) STRING
        ...
        PRC     double      DOUBLE
```

## mysql::load

### Syntax

* `mysql::load(connection, table_or_query, [schema], [startRow], [rowNum])`

### Parameters
* `connection`: a MySQL connection handle created with `mysql::connect`.
* `table_or_query`: the name of a MySQL server table or a MySQL query. Data type `string`.
* `schema`: a table with names and data types of columns. If we need to change the data type of a column that is automatically determined by the system, the schema table needs to be modified and used as an argument.
* `startRow`: an integer indicating the index of the starting row to read. If unspecified, read from the first row.
* `rowNum`: an integer indicating the number of rows to read. If unspecified, read to the last row. If `table_or_query` is a SQL query, then `rowNum` is unspecified.

### Details
* Load a MySQL table or SQL query result into a DolphinDB in-memory table.
* For details about supported data types as well as data conversion rules, please refer to the [Data Types] (#Data Types) section.

### Example
```
conn = mysql::connect(`localhost, `root, `root, 3306, `DolphinDB)
tb = mysql::load(conn, `US,,0,123456)
select count(*) from tb
```

```
conn = mysql::connect(`localhost, `root, `root, 3306, `DolphinDB)
tb = mysql::load(conn, "SELECT PERMNO FROM US LIMIT 123456")
select count(*) from tb
```

```
mysql::load(conn, "SELECT now()");
```

## mysql::loadEx

### Syntax

* `mysql::loadEx(connection, dbHandle,tableName,[partitionColumns],table_or_query,[schema],[startRow],[rowNum])`

### Parameters
* `connection`: a MySQL connection handle created with `mysql::connect`.
* `dbHandle` and `tableName`: If the input data is to be saved into a distributed database, the database handle and table name should be specified.
* `partitionColumns`: a string scalar/vector indicating partitioning column(s).
* `table_or_query`: the name of a MySQL server table or a MySQL query. Data type `string`.
* `schema`: a table with names and data types of columns. If we need to change the data type of a column that is automatically determined by the system, the schema table needs to be modified and used as an argument.
* `startRow`: an integer indicating the index of the starting row to read. If unspecified, read from the first row.
* `rowNum`: an integer indicating the number of rows to read. If unspecified, read to the last row. If `table_or_query` is a SQL query, then `rowNum` is unspecified.

### Details
* Load a MySQL table as a distributed table. The result is a table object with loaded metadata.
* For details about supported data types as well as data conversion rules, please refer to the [Data Types] (#Data Types) section.

### Example

* Load data as a partitioned table on disk. 
```
dbPath = "C:/..."
db = database(dbPath, RANGE, 0 500 1000)
mysql::loadEx(conn, db,`tb, `PERMNO, `US)
tb = loadTable(dbPath, `tb)
```

```
dbPath = "C:/..."
db = database(dbPath, RANGE, 0 500 1000)
mysql::loadEx(conn, db,`tb, `PERMNO, "SELECT * FROM US LIMIT 1000");
tb = loadTable(dbPath, `tb)
```

* Load data as an in-memory partitioned table
```
db = database("", RANGE, 0 50000 10000)
tb = mysql::loadEx(conn, db,`tb, `PERMNO, `US)
```

```
db = database("", RANGE, 0 50000 10000)
tb = mysql::loadEx(conn, db,`tb, `PERMNO, "SELECT * FROM US LIMIT 100");
```

* Load data as a DFS partitioned table
```
db = database("dfs://US", RANGE, 0 50000 10000)
mysql::loadEx(conn, db,`tb, `PERMNO, `US)
tb = loadTable("dfs://US", `tb)
```

```
db = database("dfs://US", RANGE, 0 50000 10000)
mysql::loadEx(conn, db,`tb, `PERMNO, "SELECT * FROM US LIMIT 1000");
tb = loadTable("dfs://US", `tb)
```

# Data Types

## integer
| type in MySQL       | corresponding DolphinDB type |
| ------------------- | :--------------------------- |
| MYSQL_TYPE_TINY     | INT                          |
| MYSQL_TYPE_SHORT    | INT                          |
| MYSQL_TYPE_INT24    | INT                          |
| MYSQL_TYPE_LONG     | LONG                         |
| MYSQL_TYPE_LONGLONG | LONG                         |


* The numeric types in DolphinDB are all signed types. To prevent overflow, all unsigned types are converted to ```high-order signed types, 64-bit unsigned types are not supported ```
* `unsigned long long` are not supported in DolphinDB, you can specify schema and use `DOUBLE` or `FLOAT` if needed.

## float
| type in MySQL         | corresponding DolphinDB type |
| --------------------- | :--------------------------- |
| MYSQL_TYPE_DOUBLE     | DOUBLE                       |
| MYSQL_TYPE_DECIMAL    | DOUBLE                       |
| MYSQL_TYPE_NEWDECIMAL | DOUBLE                       |
| MYSQL_TYPE_FLOAT      | FLOAT                        |

Note: IEEE754 floating types are all signed numbers.

* All floating types can be converted to numeric types ```(bool, char, short, int, long, float, double)``` in DolphinDB. 

## time

| type in MySQL        | corresponding DolphinDB type |
| -------------------- | :--------------------------- |
| MYSQL_TYPE_DATE      | DATE                         |
| MYSQL_TYPE_TIME      | TIME                         |
| MYSQL_TYPE_DATETIME  | DATETIME                     |
| MYSQL_TYPE_TIMESTAMP | TIMESTAMP                    |
| MYSQL_TYPE_YEAR      | INT                          |


* All data types above can be converted to a temperal data type in DolphinDB `(date, month, time, minute, second, datetime, timestamp, nanotime, nanotimestamp)`.

## string
| type in MySQL         | corresponding DolphinDB type |
| --------------------- | :--------------------------- |
| MYSQL_TYPE_STRING     | STRING                       |
| MYSQL_TYPE_VAR_STRING | STRING                       |
| MYSQL_TYPE_VAR_CHAR   | STRING                       |
| MYSQL_TYPE_BLOB       | STRING                       |

* `string` type will be converted to STRING or SYMBOL type in DolphinDB.

## enum

| type in MySQL   | corresponding DolphinDB type |
| --------------- | :--------------------------- |
| MYSQL_TYPE_ENUM | SYMBOL                       |

* Enum type will be converted to STRING or SYMBOL type in DolphinDB.

# Data Import Performance

## Environment

* CPU: i7-7700 3.60GHZ
* Harddisk: SSD, read speed 30~40MB/s

## Data 

* Load US stock daily data from 1990 to 2016 with 22 fields and 50591907 rows.

## Time Consumed 

191.5 seconds