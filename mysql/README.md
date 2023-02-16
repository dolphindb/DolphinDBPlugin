# DolphinDB MySQL Plugin

DolphinDB's MySQL plugin offers high speed import of MySQL datasets or query results into DolphinDB. It supports data type conversion. Part of the plugin follows mysqlxx by Yandex.Clickhouse.

The DolphinDB MySQL plugin has the branches [release 200](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/mysql/README.md) and [release130](https://github.com/dolphindb/DolphinDBPlugin/blob/release130/mysql/README.md). Each plugin version corresponds to a DolphinDB server version. You're looking at the plugin documentation for release200. If you use a different DolphinDB server version, please refer to the corresponding branch of the plugin documentation.

## 1. Build

### 1.1 Install a precompiled distribution

Users can import pre-compiled MySQL plug-ins (in the DolphinDB installation package or under the bin directory) with the following command in DolphinDB:

In Linux:
```
loadPlugin("/path/to/plugins/mysql/PluginMySQL.txt")
```

In Windows:
```
loadPlugin("C:/path/to/mysql/PluginMySQL.txt")
```

Note that you must load the plugin with an absolute path and replace "\\" with "\\\\" or "/".

### 1.2 Compile and install

#### 1.2.1 Install in Linux

Install [git](https://git-scm.com/) and [CMake](https://cmake.org/).

For Ubuntu users, just type
```bash
$ sudo apt-get install git cmake
```

Then update the git submodule with the following script. This automatically downloads [mariadb-connector-c](https://github.com/MariaDB/mariadb-connector-c) source files.
```
$ git submodule update --init --recursive
```

Install cmake:
```
sudo apt-get install cmake
```

Build the project:
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../path_to_mysql_plugin/
make -j`nproc`
```

**Note:** Before compiling, please make sure that libDolphinDB.so is on a path that can be found by gcc. The path can be specified with "LD_LIBRARY_PATH".

The file libPluginMySQL.so will be generated after compilation.

#### 1.2.2 Install in Windows

To install in Windows, we need to compile with [cmake](https://cmake.org/) and [MinGW](http://www.mingw.org/). 

Please download [cmake](https://cmake.org/) and [MinGW](http://www.mingw.org/). Make sure to add the bin directory to the system environment variable "Path" in MinGW.

Build the project:
```
mkdir build
cp libDolphinDB.dll build                 # copy libDolphinDB.dll to build directory
cp -r curl build                          # copy curl headers to build directory
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../path_to_mysql_plugin/ -G "MinGW Makefiles"
mingw32-make -j4
```

**Note:** Before compiling, copy libDolphinDB.dll and the curl header folder to "build" directory.


## 2. Users API

**Note:** Use loadPlugin("/path_to_PluginMySQL.txt/PluginMySQL.txt") to import MySQL plugin.

### 2.1 mysql::connect

#### Syntax

mysql::connect(host, port, user, password, db)

#### Parameters

* host: a string indicating the address of the MySQL server. 
* port: an int indicating the port of the MySQL server. 
* user: a string indicating the user name of the MySQL server. 
* password: a string indicating the password of the MySQL server. 
* db: a string indicating the database name. 

#### Details

Create a connection to the MySQL server. Return a handle of MySQL connection, which will be used to access the MySQL server later.

#### Example

```
conn = mysql::connect(`localhost, 3306, `root, `root, `DolphinDB)
```

### 2.2 mysql::showTables

#### Syntax

mysql::showTables(connection)

#### Parameters

* connection: a MySQL connection handle created with `mysql::connect`.

#### Details

List all table names in a MySql database specified in `mysql::connect`.

#### Examples

```
conn = mysql::connect(`localhost, 3306, `root, `root, `DolphinDB)
mysql::showTables(conn)

output:
  Tables_in_DolphinDB
  -------------------
  US
```

### 2.3 mysql::extractSchema

#### Syntax

mysql::extractSchema(connection, tableName)

#### Parameters
* connection: a MySQL connection handle created with `mysql::connect`.
* tableName: a string indicating the name of a table in MySQL server.

#### Details
Generate the schema of a table.

#### Examples
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

### 2.4 mysql::load

#### Syntax

mysql::load(connection, table_or_query, [schema], [startRow], [rowNum])

#### Parameters
* connection: a MySQL connection handle created with `mysql::connect`.
* table_or_query: a string indicating the name of a MySQL server table or a MySQL query.
* schema: a table with names and data types of columns. If we need to change the data type of a column that is automatically determined by the system, the schema table needs to be modified and used as an argument.
* startRow: an integer indicating the index of the starting row to read. If unspecified, read from the first row. If 'table_or_query' is a SQL query, then 'startRow' should be unspecified.
* rowNum: an integer indicating the number of rows to read. If unspecified, read to the last row. If 'table_or_query' is a SQL query, then 'rowNum' should be unspecified.

**Note:** If 'table_or_query' is a SQL query, use 'LIMIT' in SQL query to specify 'startRow' and 'rowNum'.

* allowEmptyTable: a Boolean indicating whether to allow importing an empty table from MySQL. The default value is false.

#### Details

Load a MySQL table or SQL query result into a DolphinDB in-memory table.

For details about supported data types as well as data conversion rules, please refer to the section of [Data Types](#Data Types) below.

#### Examples

```
conn = mysql::connect(`192.168.1.18, 3306, `root, `root, `DolphinDB)
tb = mysql::load(conn, `US,,0,123456)
select count(*) from tb
```

```
conn = mysql::connect(`127.0.0.1, 3306, `root, `root, `DolphinDB)
tb = mysql::load(conn, "SELECT PERMNO FROM US LIMIT 123456")
select count(*) from tb
```

```
mysql::load(conn, "SELECT now(6)");
```

### 2.5 mysql::loadEx

#### Syntax

mysql::loadEx(connection, dbHandle,tableName,partitionColumns,table_or_query,[schema],[startRow],[rowNum],[transform])

#### Parameters
* connection: a MySQL connection handle created with `mysql::connect`.
* dbHandle and tableName: If the input data is to be saved into a distributed database, the database handle and table name should be specified.
* partitionColumns: a string scalar/vector indicating partitioning column(s).
* table_or_query: a string indicating the name of a MySQL server table or a MySQL query.
* schema: a table with names and data types of columns. If we need to change the data type of a column that is automatically determined by the system, the schema table needs to be modified and used as an argument.
* startRow: an integer indicating the index of the starting row to read. If unspecified, read from the first row. If 'table_or_query' is a SQL query, then 'startRow' should unspecified.
* rowNum: an integer indicating the number of rows to read. If unspecified, read to the last row. If 'table_or_query' is a SQL query, then 'rowNum' should unspecified.

**Note:** If 'table_or_query' is a SQL query, use 'LIMIT' in SQL query to specify 'startRow' and 'rowNum'.

* transform: apply certain transformation on a MySQL table or query before importing into DolphinDB database.

#### Details

Load a MySQL table as a distributed table. The result is a table object with loaded metadata.

For details about supported data types as well as data conversion rules, please refer to the [Data Types](#Data Types) section.

#### Examples

* **Load data as a partitioned table on disk.**

Load the entire table

```
dbPath = "C:/..."
db = database(dbPath, RANGE, 0 500 1000)
mysql::loadEx(conn, db,`tb, `PERMNO, `US)
tb = loadTable(dbPath, `tb)
```

Load via SQL statement

```
dbPath = "C:/..."
db = database(dbPath, RANGE, 0 500 1000)
mysql::loadEx(conn, db,`tb, `PERMNO, "SELECT * FROM US LIMIT 1000");
tb = loadTable(dbPath, `tb)
```

* **Load data as an in-memory partitioned table**

Load the entire table

```
db = database("", RANGE, 0 50000 10000)
tb = mysql::loadEx(conn, db,`tb, `PERMNO, `US)
```

Load via SQL statement

```
db = database("", RANGE, 0 50000 10000)
tb = mysql::loadEx(conn, db,`tb, `PERMNO, "SELECT * FROM US LIMIT 100");
```

* **Load data as a DFS partitioned table**

Load the entire table

```
db = database("dfs://US", RANGE, 0 50000 10000)
mysql::loadEx(conn, db,`tb, `PERMNO, `US)
tb = loadTable("dfs://US", `tb)
```

Load via SQL statement

```
db = database("dfs://US", RANGE, 0 50000 10000)
mysql::loadEx(conn, db,`tb, `PERMNO, "SELECT * FROM US LIMIT 1000");
tb = loadTable("dfs://US", `tb)
```

Load and transform data into a DFS partitioned table

```
db = database("dfs://US", RANGE, 0 50000 10000)
def replaceTable(mutable t){
	return t.replaceColumn!(`svalue,t[`savlue]-1)
}
t=mysql::loadEx(conn, db, "",`stockid, 'select  * from US where stockid<=1000000',,,,replaceTable)

```

## 3. Data Types

<!-- INT TINYINT SMALLINT MEDIUMINT BIGINT CHAR VARCHAR ENUM -->

### 3.1 Integral

| MySQL type         | DolphinDB type |
| ------------------ | :--------------------------- |
| bit(1)-bit(8)      | CHAR                         |
| bit(9)-bit(16)     | SHORT                        |
| bit(17)-bit(32)    | INT                          |
| bit(33)-bit(64)    | LONG                         |
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

* The numeric types in DolphinDB are all signed types. To prevent overflow, all unsigned types are converted to high-order signed types. For example, unsigned CHAR is converted to signed SHORT, unsigned SHORT is converted to signed INT, etc. 64-bit unsigned types are not supported.
* 'unsigned long long' is not supported in DolphinDB, you can specify schema and use DOUBLE or FLOAT if needed.
* The smallest value of each integral type in DolphinDB is NULL value, e.g. -128 for CHAR, -32,768 for SHORT, -2,147,483,648 for INT and -9,223,372,036,854,775,808 for LONG all mean NULL values in each type respectively.

### 3.2 Floating-point

| MySQL type    | DolphinDB type |
| ------------- | :--------------------------- |
| double        | DOUBLE                       |
| decimal       | DOUBLE                       |
| newdecimal    | DOUBLE                       |
| float         | FLOAT                        |

**Note:** IEEE754 floating-point types are all signed numbers.

* All floating-point types can be converted to numeric types (BOOL, CHAR, SHORT, INT, LONG, FLOAT, DOUBLE) in DolphinDB.

### 3.3 Time

| MySQL type    | DolphinDB type |
| ------------- | :--------------------------- |
| date          | DATE                         |
| time          | TIME                         |
| datetime      | DATETIME                     |
| timestamp     | TIMESTAMP                    |
| year          | INT                          |

* All data types above can be converted to temperal data types in DolphinDB (DATE, MONTH, TIME, MINUTE, SECOND, DATETIME, TIMESTAMP, NANOTIME, NANOTIMESTAMP).

### 3.4 String

| MySQL type          | DolphinDB type |
| ------------------- | :--------------------------- |
| char  (len <= 10)   | SYMBOL                       |
| varchar (len <= 10) | SYMBOL                       |
| char  (len > 10)    | STRING                       |
| varchar (len > 10)  | STRING                       |
| other string types  | STRING                       |

* char and varchar types of length less or equal to 10 will be converted to SYMBOL type in DolphinDB. Other string types will be converted to STRING type in DolphinDB.
* string type will be converted to STRING or SYMBOL type in DolphinDB.

### 3.5 Enum

| MySQL type    | DolphinDB type |
| ------------- | :--------------------------- |
| enum          | SYMBOL                       |

* enum type will be converted to SYMBOL type in DolphinDB.

## 4. Data Import Performance

### 4.1 Hardware

* CPU: i7-7700 3.60GHZ
* Hard disk: SSD, read speed 460~500MB/s.

### 4.2 Data

* US stocks daily data from 1990 to 2016 with 22 fields and 50,591,907 rows. Total size is 6.5GB.

### 4.3 Time Consumed for data import

160.5 seconds