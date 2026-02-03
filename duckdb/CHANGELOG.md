# DuckDB Plugin Changelog

## v1.0.0 - 2026-02-03

### Features

- Initial release of DuckDB plugin for DolphinDB
- Support connecting to DuckDB databases
- Support loading data from DuckDB tables or query results into DolphinDB
- Support extracting schema from DuckDB
- Support listing tables in DuckDB databases
- Automatic data type conversion between DuckDB and DolphinDB
- Support batch loading with pagination

### API Functions

- `duckdb::connect(dbpath)` - Create a connection to DuckDB database
- `duckdb::close(connection)` - Close the DuckDB connection
- `duckdb::load(connection, query_or_table, [schema], [startRow], [rowNum], [allowEmptyTable])` - Load data from DuckDB
- `duckdb::extractSchema(connection, table_or_query)` - Extract schema from DuckDB
- `duckdb::tables(connection)` - List all tables in DuckDB database

### Supported Data Types

| DuckDB Type | DolphinDB Type |
|------------|----------------|
| BOOLEAN | BOOL |
| TINYINT | CHAR |
| SMALLINT | SHORT |
| INTEGER | INT |
| BIGINT | LONG |
| FLOAT | FLOAT |
| DOUBLE | DOUBLE |
| VARCHAR | STRING/SYMBOL |
| TIMESTAMP | DATETIME/TIMESTAMP |
| DATE | DATE |

### Build Requirements

- CMake 3.22+
- C++14 compiler
- DuckDB v1.1.3 (fetched via git submodule)
- DolphinDB v2.00.10+

### Platform Support

- Linux (x86_64, ARM64)
- macOS (Intel, Apple Silicon)
- Windows (MinGW, Visual Studio)

### Notes

- The plugin uses git submodules to fetch DuckDB source code
- Make sure libDolphinDB.so (or .dll/.dylib) is in the library path during compilation
