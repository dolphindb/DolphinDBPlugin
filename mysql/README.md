# DolphinDB MySQL Plugin

DolphinDB's MySQL plugin imports MySQL datasets or query results into DolphinDB. It supports data type conversion. Part of the plugin follows mysqlxx by Yandex.Clickhouse.

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

**Note:** Before compiling, please make sure that `libDolphinDB.so` is in a path that can be found by gcc. The path can be specified with `LD_LIBRARY_PATH`.

The file libPluginMySQL.so will be generated after the compilation.

# User-API

**Remember:** Use `loadPlugin("/path_to_PluginMySQL.txt/PluginMySQL.txt")` to import MySQL plugin before using the API.

## mysql::connect

### Syntax

<!-- (host, user, password, port, [socket], [ssl_ca], [ssl_cert], [ssl_key]) -->
* `mysql::connect(host, port, user, password, db)`

### Parameters

* `host`: address of MySQL server. Data type `string`.
* `port`: port of MySQL server. Data type `int`.
* `user`: user name of MySQL server. Data type `string`.
* `password`: password of MySQL server. Data type `string`.
* `db`: database name. Data type `string`.

### Details

* Create a connection to MySQL server. Return a handle of MySQL connection, which will be used to access MySQL server later.

### Examples

```
conn = mysql::connect(`localhost, 3306, `root, `root, `DolphinDB)
```

## mysql::showTables

### Syntax

* `mysql::showTables(connection)`

### Parameters

* `connection`: a MySQL connection handle created with `mysql::connect`.

### Details

* List all table names in a MySql database specified in `mysql::connect`.

### Examples

```
conn = mysql::connect(`localhost, 3306, `root, `root, `DolphinDB)
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

### Examples

```
conn = mysql::connect(`localhost, 3306, `root, `root, `DolphinDB)
mysql::extractSchema(conn, `US)

output:
        name    type        DolphinDBType
        PERMNO  INT         int(11)
        date    DATE        date
        SHRCD   INT         int(11)
        TICKER  SYMBOL      varchar(10)
        ...
        PRC     DOUBLE      double
```

## mysql::load

### Syntax

* `mysql::load(connection, table_or_query, [schema], [startRow], [rowNum])`

### Parameters
* `connection`: a MySQL connection handle created with `mysql::connect`.
* `table_or_query`: the name of a MySQL server table or a MySQL query. Data type `string`.
* `schema`: a table with names and data types of columns. If we need to change the data type of a column that is automatically determined by the system, the schema table needs to be modified and used as an argument.
* `startRow`: an integer indicating the index of the starting row to read. If unspecified, read from the first row. If `table_or_query` is a SQL query, then `startRow` is unspecified.
* `rowNum`: an integer indicating the number of rows to read. If unspecified, read to the last row. If `table_or_query` is a SQL query, then `rowNum` is unspecified.

**Note:** If `table_or_query` is a SQL query, use `LIMIT` in SQL query to specify `startRow` and `rowNum`.

### Details
* Load a MySQL table or SQL query result into a DolphinDB in-memory table.
* For details about supported data types as well as data conversion rules, please refer to the [Data Types] (#Data Types) section.

### Examples
```
conn = mysql::connect(`localhost, 3306, `root, `root, `DolphinDB)
tb = mysql::load(conn, `US,,123,123456)
select count(*) from tb
```

```
conn = mysql::connect(`localhost, 3306, `root, `root, `DolphinDB)
tb = mysql::load(conn, "SELECT PERMNO FROM US LIMIT 123,123456")
select count(*) from tb
```

```
mysql::load(conn, "SELECT now(6)", table(`val as name, `NANOTIMESTAMP as type));
```

## mysql::loadEx

### Syntax

* `mysql::loadEx(connection, dbHandle,tableName,partitionColumns,table_or_query,[schema],[startRow],[rowNum])`

### Parameters
* `connection`: a MySQL connection handle created with `mysql::connect`.
* `dbHandle` and `tableName`: If the input data is to be saved into a distributed database, the database handle and table name should be specified.
* `partitionColumns`: a string scalar/vector indicating partitioning column(s).
* `table_or_query`: the name of a MySQL server table or a MySQL query. Data type `string`.
* `schema`: a table with names and data types of columns. If we need to change the data type of a column that is automatically determined by the system, the schema table needs to be modified and used as an argument.
* `startRow`: an integer indicating the index of the starting row to read. If unspecified, read from the first row. If `table_or_query` is a SQL query, then `startRow` is unspecified.
* `rowNum`: an integer indicating the number of rows to read. If unspecified, read to the last row. If `table_or_query` is a SQL query, then `rowNum` is unspecified.

**Note:** If `table_or_query` is a SQL query, use `LIMIT` in SQL query to specify `startRow` and `rowNum`.

### Details
* Load a MySQL table as a distributed table. The result is a table object with loaded metadata.
* For details about supported data types as well as data conversion rules, please refer to the [Data Types] (#Data Types) section.

### Examples

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

<!-- INT TINYINT SMALLINT MEDIUMINT BIGINT CHAR VARCHAR ENUM -->

## Integral

| MySQL type         | corresponding DolphinDB type |
| ------------------ | :--------------------------- |
| tinyint            | CHAR                         |
| tinyint unsigned   | SHORT                        |
| smallint           | SHORT                        |
| smallint unsigned  | INT                          |
| mediumint          | INT                          |
| mediumint unsigned | INT                          |
| int                | INT                          |
| int unsigned       | LONG                         |
| bigint             | LONG                         |
| bigint unsigned    | (unsupported) LONG           |

* The numeric types in DolphinDB are all signed types. To prevent overflow, all unsigned types are converted to high-order signed types. For example, unsigned`CHAR` is converted to signed `SHORT`, unsigned `SHORT` is converted to signed `INT`, etc. 64-bit unsigned types are not supported.
* `unsigned long long` are not supported in DolphinDB, you can specify schema and use `DOUBLE` or `FLOAT` if needed.
* The smallest value of each integral type in DolphinDB means NULL value, e.g. `-128` for `CHAR`, `-32,768` for `SHORT`, `-2,147,483,648` for `INT` and `-9,223,372,036,854,775,808` for `LONG` mean `NULL` values in each type respectively.

## Floating-point

| MySQL type    | corresponding DolphinDB type |
| ------------- | :--------------------------- |
| double        | DOUBLE                       |
| decimal       | DOUBLE                       |
| newdecimal    | DOUBLE                       |
| float         | FLOAT                        |

**Note:** IEEE754 floating-point types are all signed numbers.

* All floating-point types can be converted to numeric types ```(bool, char, short, int, long, float, double)``` in DolphinDB.

## Time

| MySQL type    | corresponding DolphinDB type |
| ------------- | :--------------------------- |
| date          | DATE                         |
| time          | TIME                         |
| datetime      | DATETIME                     |
| timestamp     | TIMESTAMP                    |
| year          | INT                          |

* All data types above can be converted to temperal data types in DolphinDB `(date, month, time, minute, second, datetime, timestamp, nanotime, nanotimestamp)`.

## String

| MySQL type          | corresponding DolphinDB type |
| ------------------- | :--------------------------- |
| char  (len <= 10)   | SYMBOL                       |
| varchar (len <= 10) | SYMBOL                       |
| char  (len > 10)    | STRING                       |
| varchar (len > 10)  | STRING                       |
| other string types  | STRING                       |

* `char` and `varchar` types of length less or equal to 10 will be converted to `SYMBOL` type in DolphinDB. Other string types will be converted to `STRING` type in DolphinDB.
* `string` type will be converted to `STRING` or `SYMBOL` type in DolphinDB.

## Enum

| MySQL type    | corresponding DolphinDB type |
| ------------- | :--------------------------- |
| enum          | SYMBOL                       |

* `enum` type will be converted to `SYMBOL` type in DolphinDB.

# Data Import Performance

## Environment

* CPU: i7-7700 3.60GHZ
* Hard disk: SSD, read speed 460~500MB/s.

## Data

* US stock daily data from 1990 to 2016 with 22 fields and 50591907 rows. Total size is 6.5GB.

## Time Consumed

160.5 seconds