# DolphinDB DuckDB Plugin

DolphinDB's DuckDB plugin offers high speed import of DuckDB datasets or query results into DolphinDB. It supports data type conversion and efficient batch data loading.

## 1. Build

### 1.1 Install a precompiled distribution

Users can import pre-compiled DuckDB plug-ins (in the DolphinDB installation package or under the bin directory) with the following command in DolphinDB:

In Linux/macOS:
```
loadPlugin("/path/to/plugins/duckdb/PluginDuckDB.txt")
```

In Windows:
```
loadPlugin("C:/path/to/duckdb/PluginDuckDB.txt")
```

Note that you must load the plugin with an absolute path and replace "\\" with "\\\\" or "/".

### 1.2 Compile and install

#### 1.2.1 Install in Linux/macOS

Install [git](https://git-scm.com/) and [CMake](https://cmake.org/).

For Ubuntu users, just type
```bash
$ sudo apt-get install git cmake
```

For macOS users, install via Homebrew:
```bash
$ brew install git cmake
```

Then update the git submodule with the following script. This automatically downloads [DuckDB](https://github.com/duckdb/duckdb) source files.
```
$ git submodule update --init --recursive
```

Build the project:
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../path_to_duckdb_plugin/
make -j$(nproc)
```

**Note:** Before compiling, please make sure that libDolphinDB.so is on a path that can be found by gcc. The path can be specified with "LD_LIBRARY_PATH".

The file libPluginDuckDB.so will be generated after compilation.

#### 1.2.2 Install in Windows

To install in Windows, we need to compile with [cmake](https://cmake.org/) and [MinGW](http://www.mingw.org/) or Visual Studio.

Please download [cmake](https://cmake.org/) and [MinGW](http://www.mingw.org/). Make sure to add the bin directory to the system environment variable "Path" in MinGW.

Build the project:
```
mkdir build
cp libDolphinDB.dll build                 # copy libDolphinDB.dll to build directory
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../path_to_duckdb_plugin/ -G "MinGW Makefiles"
mingw32-make -j4
```

**Note:** Before compiling, copy libDolphinDB.dll to the "build" directory.

## 2. Users API

**Note:** Use loadPlugin("/path_to_PluginDuckDB.txt/PluginDuckDB.txt") to import DuckDB plugin.

### 2.1 duckdb::connect

#### Syntax

duckdb::connect(dbpath)

#### Parameters

* dbpath: a string indicating the path to the DuckDB database file. Use ":memory:" for in-memory database.

#### Details

Create a connection to the DuckDB database. Return a handle of DuckDB connection, which will be used to access the DuckDB database later.

#### Example

```
conn = duckdb::connect("/path/to/database.duckdb")
// or for in-memory database
conn = duckdb::connect(":memory:")
```

### 2.2 duckdb::close

#### Syntax

duckdb::close(connection)

#### Parameters

* connection: a connection handle created by duckdb::connect.

#### Details

Close the connection to the DuckDB database.

#### Example

```
duckdb::close(conn)
```

### 2.3 duckdb::load

#### Syntax

duckdb::load(connection, query_or_table, [schema], [startRow=0], [rowNum=ULONGLONG_MAX], [allowEmptyTable=FALSE])

#### Parameters

* connection: a connection handle created by duckdb::connect.
* query_or_table: a string indicating the table name or SQL query.
* schema: a table indicating the schema of the result. If not specified, the plugin will automatically extract schema from DuckDB.
* startRow: the starting row number (inclusive). Default is 0.
* rowNum: the number of rows to load. Default is ULONGLONG_MAX (all rows).
* allowEmptyTable: a boolean value. If true, empty tables are allowed. Default is false.

#### Details

Load data from DuckDB table or query result into DolphinDB. Returns a DolphinDB table.

#### Example

```
// Load entire table
t = duckdb::load(conn, "SELECT * FROM my_table")

// Load with schema
schema = duckdb::extractSchema(conn, "my_table")
t = duckdb::load(conn, "SELECT * FROM my_table", schema)

// Load with pagination
t = duckdb::load(conn, "SELECT * FROM my_table", , 1000, 1000)
```

### 2.4 duckdb::extractSchema

#### Syntax

duckdb::extractSchema(connection, table_or_query)

#### Parameters

* connection: a connection handle created by duckdb::connect.
* table_or_query: a string indicating the table name or SQL query.

#### Details

Extract the schema (column names and types) from a DuckDB table or query result. Returns a DolphinDB table with two columns: 'name' and 'type'.

#### Example

```
schema = duckdb::extractSchema(conn, "my_table")
```

### 2.5 duckdb::tables

#### Syntax

duckdb::tables(connection)

#### Parameters

* connection: a connection handle created by duckdb::connect.

#### Details

List all tables in the DuckDB database. Returns a DolphinDB table containing table names.

#### Example

```
tbls = duckdb::tables(conn)
```

## 3. Data Type Mapping

| DuckDB Type | DolphinDB Type |
|------------|----------------|
| BOOLEAN | BOOL |
| TINYINT | CHAR |
| SMALLINT | SHORT |
| INTEGER | INT |
| BIGINT | LONG |
| UTINYINT | UCHAR |
| USMALLINT | USHORT |
| UINTEGER | UINT |
| UBIGINT | ULONG |
| FLOAT | FLOAT |
| DOUBLE | DOUBLE |
| VARCHAR | STRING |
| DATE | DATE |
| TIME | TIME |
| TIMESTAMP | TIMESTAMP |
| TIMESTAMP WITH TIME ZONE | NANOTIMESTAMP |
| INTERVAL | Not supported |
| DECIMAL | DOUBLE (converted) |
| BLOB | Not supported |

## 4. Performance Optimization

### 4.1 Batch Loading

For large tables, use the `startRow` and `rowNum` parameters to load data in batches:

```
// Load 1 million rows at a time
batchSize = 1000000
totalRows = 5000000
for(i in 0..4) {
    t = duckdb::load(conn, "SELECT * FROM large_table", , i * batchSize, batchSize)
    // Process or save the data
    tableInsert(targetTable, t)
}
```

### 4.2 Parallel Loading

For multiple tables, use DolphinDB's `submitJob` for parallel loading:

```
def loadTable(tableName) {
    t = duckdb::load(conn, "SELECT * FROM " + tableName)
    return t
}

tables = ["table1", "table2", "table3"]
jobIds = []
for(tbl in tables) {
    jobIds.append!(submitJob("load_" + tbl, , loadTable, tbl))
}
```

### 4.3 Query Optimization

- Use specific column selection instead of `SELECT *`
- Add WHERE clauses to filter data at DuckDB side
- Use DuckDB's aggregate functions when possible

## 5. Error Handling

The plugin will throw exceptions in the following cases:

- Invalid connection handle
- Connection failure
- SQL syntax errors
- Data type conversion errors
- File not found (for file-based databases)

Always wrap plugin calls in try-catch blocks in DolphinDB:

```
try {
    t = duckdb::load(conn, "SELECT * FROM my_table")
} catch(ex) {
    print("Error: " + ex.message)
}
```

## 6. License

This plugin follows the same license as DolphinDB.

## 7. Support

For issues and questions, please visit [DolphinDB GitHub](https://github.com/dolphindb/DolphinDBPlugin).
