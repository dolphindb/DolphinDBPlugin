# DolphinDB DuckDB Plugin

DolphinDB的DuckDB导入插件可将DuckDB中的数据表或查询结果高速导入DolphinDB，
并且支持数据类型转换和高效的批量数据加载。

本文档仅介绍编译构建方法。通过[文档中心 - DuckDB](https://docs.dolphindb.cn/zh/plugins/duckdb/duckdb.html)查看使用介绍；通过 CHANGELOG.md 查看版本发布记录。

## 编译安装

### 在 Linux/macOS 下编译安装

#### 环境准备

安装 [git](https://git-scm.com/) 和 [CMake](https://cmake.org/)。

Ubuntu用户只需要在命令行输入以下命令即可：
```bash
$ sudo apt-get install git cmake
```

macOS 用户通过 Homebrew 安装：
```bash
$ brew install git cmake
```

然后通过更新git子模块来下载 [DuckDB](https://github.com/duckdb/duckdb) 的源文件。
```
$ git submodule update --init --recursive
```

#### cmake 编译

构建插件内容：
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../path/to/duckdb_plugin/
make -j$(nproc)
```

**注意:** 编译之前请确保libDolphinDB.so在gcc可搜索的路径中。可使用LD_LIBRARY_PATH指定其路径，或者直接将其拷贝到build目录下。

编译之后目录下会产生libPluginDuckDB.so文件。

### 在 Windows 下编译安装

#### 在 Windows 环境中需要使用CMake和MinGW编译

* 下载安装[MinGW](http://www.mingw.org/)。确保将bin目录添加到系统环境变量Path中。
* 下载安装[cmake](https://cmake.org/)。

#### cmake 编译

在编译开始之前，要将libDolphinDB.dll和包含DuckDB头文件的文件夹拷贝到build文件夹内。

构建插件内容：
```
mkdir build                                                       
cp path_to_libDolphinDB.dll/libDolphinDB.dll build               
cp -r contrib/duckdb build                                        
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../path_to_duckdb_plugin/ -G "MinGW Makefiles"
mingw32-make -j4
```

**注意：** 编译前请将 libDolphinDB.dll 复制到 "build" 目录。

## 2. 用户 API

**注意：** 使用 loadPlugin("/path_to_PluginDuckDB.txt/PluginDuckDB.txt") 导入 DuckDB 插件。

### 2.1 duckdb::connect

#### 语法

duckdb::connect(dbpath)

#### 参数

* dbpath: 字符串，表示 DuckDB 数据库文件的路径。使用 ":memory:" 表示内存数据库。

#### 详情

创建到 DuckDB 数据库的连接。返回 DuckDB 连接句柄，用于后续访问 DuckDB 数据库。

#### 示例

```
conn = duckdb::connect("/path/to/database.duckdb")
// 或使用内存数据库
conn = duckdb::connect(":memory:")
```

### 2.2 duckdb::close

#### 语法

duckdb::close(connection)

#### 参数

* connection: 由 duckdb::connect 创建的连接句柄。

#### 详情

关闭到 DuckDB 数据库的连接。

#### 示例

```
duckdb::close(conn)
```

### 2.3 duckdb::load

#### 语法

duckdb::load(connection, query_or_table, [schema], [startRow=0], [rowNum=ULONGLONG_MAX], [allowEmptyTable=FALSE])

#### 参数

* connection: 由 duckdb::connect 创建的连接句柄。
* query_or_table: 字符串，表示表名或 SQL 查询语句。
* schema: 表，表示结果的 schema。如果未指定，插件将自动从 DuckDB 提取 schema。
* startRow: 起始行号（包含）。默认为 0。
* rowNum: 要加载的行数。默认为 ULONGLONG_MAX（所有行）。
* allowEmptyTable: 布尔值。如果为 true，允许空表。默认为 false。

#### 详情

从 DuckDB 表或查询结果加载数据到 DolphinDB。返回 DolphinDB 表。

#### 示例

```
// 加载整个表
t = duckdb::load(conn, "SELECT * FROM my_table")

// 使用 schema 加载
schema = duckdb::extractSchema(conn, "my_table")
t = duckdb::load(conn, "SELECT * FROM my_table", schema)

// 分页加载
t = duckdb::load(conn, "SELECT * FROM my_table", , 1000, 1000)
```

### 2.4 duckdb::extractSchema

#### 语法

duckdb::extractSchema(connection, table_or_query)

#### 参数

* connection: 由 duckdb::connect 创建的连接句柄。
* table_or_query: 字符串，表示表名或 SQL 查询语句。

#### 详情

从 DuckDB 表或查询结果提取 schema（列名和类型）。返回包含两列的 DolphinDB 表：'name' 和 'type'。

#### 示例

```
schema = duckdb::extractSchema(conn, "my_table")
```

### 2.5 duckdb::tables

#### 语法

duckdb::tables(connection)

#### 参数

* connection: 由 duckdb::connect 创建的连接句柄。

#### 详情

列出 DuckDB 数据库中的所有表。返回包含表名的 DolphinDB 表。

#### 示例

```
tbls = duckdb::tables(conn)
```

## 3. 数据类型映射

| DuckDB 类型 | DolphinDB 类型 |
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
| INTERVAL | 不支持 |
| DECIMAL | DOUBLE（转换） |
| BLOB | 不支持 |

## 4. 性能优化

### 4.1 批量加载

对于大表，使用 `startRow` 和 `rowNum` 参数分批加载数据：

```
// 每次加载 100 万行
batchSize = 1000000
totalRows = 5000000
for(i in 0..4) {
    t = duckdb::load(conn, "SELECT * FROM large_table", , i * batchSize, batchSize)
    // 处理或保存数据
    tableInsert(targetTable, t)
}
```

### 4.2 并行加载

对于多个表，使用 DolphinDB 的 `submitJob` 进行并行加载：

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

### 4.3 查询优化

- 使用特定的列选择而不是 `SELECT *`
- 添加 WHERE 子句在 DuckDB 端过滤数据
- 尽可能使用 DuckDB 的聚合函数

## 5. 错误处理

插件在以下情况下会抛出异常：

- 无效的连接句柄
- 连接失败
- SQL 语法错误
- 数据类型转换错误
- 文件未找到（对于基于文件的数据库）

始终在 DolphinDB 中使用 try-catch 块包装插件调用：

```
try {
    t = duckdb::load(conn, "SELECT * FROM my_table")
} catch(ex) {
    print("错误: " + ex.message)
}
```

## 6. 许可证

本插件遵循与 DolphinDB 相同的许可证。

## 7. 支持

如有问题和疑问，请访问 [DolphinDB GitHub](https://github.com/dolphindb/DolphinDBPlugin)。
