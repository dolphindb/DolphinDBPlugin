# DolphinDB Parquet Plugin

Apache Parquet 文件采用列式存储格式，可用于高效存储与提取数据。DolphinDB 提供的 Parquet 插件支持将 Parquet 文件导入和导出 DolphinDB，并进行数据类型转换。

Parquet 插件目前支持版本：[relsease200](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/parquet/README_CN.md), [release130](https://github.com/dolphindb/DolphinDBPlugin/blob/release130/parquet/README_CN.md), [relsease120](https://github.com/dolphindb/DolphinDBPlugin/blob/release120/parquet/README_CN.md), [release110](https://github.com/dolphindb/DolphinDBPlugin/blob/release110/parquet/README_CN.md)。您当前查看的插件版本为 release200，请使用 DolphinDB 2.00.X 版本 server。若使用其它版本 server，请切换至相应插件分支。

- [DolphinDB Parquet Plugin](#dolphindb-parquet-plugin)
  - [1 安装插件](#1-安装插件)
    - [1.1 下载预编译插件](#11-下载预编译插件)
    - [1.2 （可选）手动编译插件](#12-可选手动编译插件)
      - [（Linux）使用 CMake构建](#linux使用-cmake构建)
    - [1.3 安装插件](#13-安装插件)
  - [2 用户接口](#2-用户接口)
    - [2.1 parquet::extractParquetSchema](#21-parquetextractparquetschema)
    - [2.2 parquet::loadParquet](#22-parquetloadparquet)
    - [2.3 parquet::loadParquetEx](#23-parquetloadparquetex)
    - [2.4 parquet::parquetDS](#24-parquetparquetds)
    - [2.5 parquet::saveParquet](#25-parquetsaveparquet)
    - [2.6 parquet::setReadThreadNum](#26-parquetsetreadthreadnum)
    - [2.7 parquet::getReadThreadNum](#27-parquetgetreadthreadnum)
  - [3 支持的数据类型](#3-支持的数据类型)
    - [3.1 导入](#31-导入)
    - [3.2 导出](#32-导出)
- [Release Notes](#release-notes)
  - [2.00.11](#20011)
    - [新增功能](#新增功能)
  - [2.00.10](#20010)
    - [优化](#优化)

## 1 安装插件

### 1.1 下载预编译插件

DolphinDB 提供了预编译的 Parquet 插件，可在 Linux 系统上直接进行安装。[点击此处下载插件](https://gitee.com/dolphindb/DolphinDBPlugin/tree/master/parquet/bin/linux64)

请注意插件的版本应与 DolphinDB 客户端版本相同，可以通过切换分支获取相应版本。

### 1.2 （可选）手动编译插件

用户也可根据业务需求，自行编译 Parquet 插件进行安装。方法如下：

#### （Linux）使用 CMake构建

(1) 安装 CMake：

```
sudo apt-get install cmake
```

(2) 安装 zlib：

```
sudo apt-get install zlib1g
```

(3) 编译 Parquet 开发包：

```
git clone https://github.com/apache/arrow.git
cd arrow/cpp
mkdir build
cd build
cmake .. -DARROW_PARQUET=ON -DARROW_IPC=ON -DARROW_BUILD_INTEGRATION=ON -DARROW_BUILD_STATIC=ON -DPARQUET_BUILD_SHARED=OFF -DARROW_BUILD_SHARED=OFF -DARROW_DEPENDENCY_USE_SHARED=OFF -DARROW_WITH_ZLIB=ON -DARROW_WITH_SNAPPY=ON -DARROW_WITH_ZSTD=ON -DARROW_WITH_LZ4=ON -DARROW_WITH_BZ2=ON
```

> **请注意：编译环境中的依赖库 libstdc++ 需要和 dolphindb server 下版本的一致。DolphinDB 提供的预编译版本插件支持 zlib, snappy, zstd, lz4 和 bz2 压缩格式，您可在此基础上根据需要支持的压缩类型增减编译选项。详情请参考 [Apache Arrow 相关文档](https://github.com/apache/arrow/blob/master/docs/source/developers/cpp/building.rst#optional-components)。**

（4）编译完成后，拷贝以下文件到目标目录：

| **Files **                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                | **Target Directory**         |
|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|------------------------------|
| arrow/cpp/build/src/parquet/parquet_version.h                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             | ./parquetApi/include/parquet |
| arrow/cpp/src/arrow<br>arrow/cpp/src/parquet<br>arrow/cpp/src/generated                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   | ./parquetApi/include         |
| arrow/cpp/build/release/libarrow.a<br>arrow/cpp/build/release/libparquet.a<br>arrow/cpp/build/thrift_ep-install/lib/libthrift.a<br>arrow/cpp/build/utf8proc_ep-install/lib/libutf8proc.a<br>arrow/cpp/build/jemalloc_ep-prefix/src/jemalloc_ep/lib/libjemalloc_pic.a<br>arrow/cpp/build/zstd_ep-install/lib64/libzstd.a<br>arrow/cpp/build/zlib_ep/src/zlib_ep-install/lib/libz.a<br>arrow/cpp/build/snappy_ep/src/snappy_ep-install/lib/libsnappy.a<br>arrow/cpp/build/lz4_ep-prefix/src/lz4_ep/lib/liblz4.a<br>arrow/cpp/build/bzip2_ep-install/lib/libbz2.a<br>arrow/cpp/build/boost_ep-prefix/src/boost_ep/stage/lib/libboost_regex.a | ./parquetApi/lib/linux       |

编译整个项目：

```
mkdir build
cd build
cmake ../path_to_parquet_plugin/
make
```

### 1.3 安装插件

在 Linux 导入 DolphinDB 提供的[预编译 Parquet 插件](https://gitee.com/dolphindb/DolphinDBPlugin/tree/master/parquet/bin/linux64)，或用户自行编译的插件。

(1) 添加插件所在路径到 LIB 搜索路径 LD_LIBRARY_PATH

```
export LD_LIBRARY_PATH=/path_to_parquet_plugin/:$LD_LIBRARY_PATH
```

(2) 启动 DolphinDB server 并导入插件

```
loadPlugin("/path_to_parquet_plugin/PluginParquet.txt")
```

## 2 用户接口

### 2.1 parquet::extractParquetSchema

**语法**

parquet::extractParquetSchema(fileName)

**参数**

* fileName: Parquet 文件名，类型为字符串标量。

**详情**

获取 Parquet 文件的结构，返回两列：列名和数据类型。

**例子**
```
parquet::extractParquetSchema("userdata1.parquet")
```

### 2.2 parquet::loadParquet

**语法**

parquet::loadParquet(fileName,[schema],[column],[rowGroupStart],[rowGroupNum])

**参数**

* fileName: Parquet 文件名，类型为字符串标量。
* schema: 可选参数，必须是包含列名和列数据类型的表。通过设置该参数，可改变系统自动生成的列数据类型。
* column: 可选参数，整数向量，表示要读取的列索引。若不指定，读取所有列。
* rowGroupStart: 可选参数，是一个非负整数。从哪一个 row group 开始读取 Parquet 文件。若不指定，默认从文件起始位置读取。
* rowGroupNum: 可选参数，要读取 row group 的数量。若不指定，默认读到文件的结尾。

**详情**

将 Parquet 文件数据加载为 DolphinDB 数据库的内存表。关于 Parquet 数据类型及在 DolphinDB 中的转化规则，参见下文[数据类型](#3-支持的数据类型)章节。

**例子**
```
parquet::loadParquet("userdata1.parquet")
```

### 2.3 parquet::loadParquetEx

**语法**

parquet::loadParquetEx(dbHandle,tableName,partitionColumns,fileName,[schema],[column],[rowGroupStart],[rowGroupNum],[tranform])

**参数**

* dbHandle：数据库句柄
* tableName：一个字符串，表示表的名称。
* partitionColumns: 字符串标量或向量，表示分区列。在组合分区中，该参数是字符串向量。
* fileName：Parquet 文件名，类型为字符串标量。
* schema: 可选参数，必须是包含列名和列数据类型的表。通过设置该参数，可改变系统自动生成的列数据类型。
* column: 可选参数，整数向量，表示读取的列索引。若不指定，读取所有列。
* rowGroupStart: 可选参数，从哪一个 row group 开始读取 Parquet 文件。若不指定，默认从文件起始位置读取。
* rowGroupNum: 可选参数，要读取 row group 的数量。若不指定，默认读到文件的结尾。
* tranform: 可选参数，为一元函数，且该函数接受的参数必须是一个表。如果指定了 *transform* 参数，需要先创建分区表，再加载数据，程序会对数据文件中的数据执行 *transform* 参数指定的函数，再将得到的结果保存到分区表中。

**详情**

将 Parquet 文件数据加载到 DolphinDB 数据库的分区表，返回该表的元数据。 

* 如果要将数据文件加载到分布式数据库或本地磁盘数据库中，必须指定 *dbHandle*，并且不能为空字符串。

* 如果要将数据文件加载到内存数据库中，那么 *dbHandle* 为空字符串或者不指定 *dbHandle*。

关于 Parquet 数据类型及在 DolphinDB 中的转化规则，参见下文[数据类型](#3-支持的数据类型)章节。

**例子**

* dfs 分区表

```
db = database("dfs://rangedb", RANGE, 0 500 1000)
parquet::loadParquetEx(db,`tb,`id,"userdata1.parquet")
```

* 分区内存表
```
db = database("", RANGE, 0 500 1000)
parquet::loadParquetEx(db,`tb,`id,"userdata1.parquet")
```

* 指定参数 *transform*，将数值类型表示的日期和时间（如：20200101）转化为指定类型（比如：日期类型）
```
dbPath="dfs://DolphinDBdatabase"
db=database(dbPath,VALUE,2020.01.01..2020.01.30)
dataFilePath="level.parquet"
schemaTB=parquet::extractParquetSchema(dataFilePath)
update schemaTB set type="DATE" where name="date"
tb=table(1:0,schemaTB.name,schemaTB.type)
tb1=db.createPartitionedTable(tb,`tb1,`date);
def i2d(mutable t){
    return t.replaceColumn!(`date,datetimeParse(t.date),"yyyy.MM.dd"))
}
t = parquet::loadParquetEx(db,`tb1,`date,dataFilePath,datasetName,,,,i2d)
```

### 2.4 parquet::parquetDS

**语法**

parquet::parquetDS(fileName,[schema])

**参数**

* fileName: Parquet 文件名，类型为字符串标量。
* schema: 可选参数，必须是包含列名和列数据类型的表。通过设置该参数，可改变系统自动生成的列数据类型。

**详情**

根据输入的 Parquet 文件名创建数据源列表，生成的数据源数量等价于 row group 的数量。

**例子**
```
>ds = parquet::parquetDS("userdata1.parquet")
>size ds;
1
>ds[0];
DataSource< loadParquet("userdata1.parquet",,,0,1) >
```

### 2.5 parquet::saveParquet

**语法**

parquet::saveParquet(table, fileName)

**参数**

table: 要保存的表

fileName: 保存的文件名，类型为字符串标量

**详情**

将 DolphinDB 中的表以 Parquet 格式保存到文件中。

**例子**

```
parquet::saveParquet(tb, "userdata1.parquet")
```

### 2.6 parquet::setReadThreadNum

**语法**

parquet::setReadThreadNum(num)

**参数**

num：最大的读取线程数。

- 默认为1，表示不额外创建线程，在当前线程读取 parquet 文件。
- 如果大于1，则会将读取 parquet 文件的任务分成 num 份，即最大的读取线程数为 num。
- 如果等于0，则每一列的读取都会作为 ploop 的任务。

**详情**

用于设置是否需要并发读取 parquet 文件和读取 parquet 的最大线程数。

注意：因为 parquet 插件内部会调用 ploop 函数按列分组并行读取 parquet 文件，所以实际读取 parquet 文件的并发度也受 DolphinDB 的 worker 参数限制。

**例子**

```
parquet::setReadThreadNum(0)
```

### 2.7 parquet::getReadThreadNum

**语法**

parquet::getReadThreadNum()

**详情**

获取 parquet 插件的最大读线程数。

**例子**

```
parquet::getReadThreadNum()
```

## 3 支持的数据类型

### 3.1 导入

DolphinDB 在导入 Parquet 数据时，优先按照源文件中定义的 LogicalType 转换相应的数据类型。如果没有定义 LogicalType 或 ConvertedType，则只根据原始数据类型（physical type）转换。

| Logical Type in Parquet    | TimeUnit in Parquet     | Type in DolphinDB|
| -------------------------- | :--------------------------- |:-----------------|
| INT(bit_width=8,is_signed=true)                       |          \                   | CHAR         |
| INT(bit_width=8,is_signed=false or bit_width=16,is_signed=true)                       |          \                   | SHORT         |
| INT(bit_width=16,is_signed=false or bit_width=32,is_signed=true)                       |          \                   | INT         |
| INT(bit_width=32,is_signed=false or bit_width=64,is_signed=true)                       |          \                   | LONG         |
| INT(bit_width=64,is_signed=false)      |          \       | LONG             |
| ENUM                       |          \                   | SYMBOL           |
| DECIMAL                    |          \                   | DOUBLE           |
| DATE                       |          \                   | DATE             |
| TIME                       |     MILLIS\MICROS\NANOS      | TIME\NANOTIME\NANOTIME    |
| TIMESTAMP                  |     MILLIS\MICROS\NANOS      | TIMESTAMP\NANOTIMESTAMP\NANOTIMESTAMP   |
| INTEGER                    |          \                   | INT\LONG         |
| STRING                     |          \                   | STRING           |
| JSON                       |          \                   | not support      |
| BSON                       |          \                   | not support      |
| UUID                       |          \                   | not support      |
| MAP                        |          \                   | not support      |
| LIST                       |          \                   | not support      |
| NIL                        |          \                   | not support      |

| Converted Type in Parquet    | Type in DolphinDB |
| --------------------------- | :-------------------------- |
| INT_8                           | CHAR                 |
| UINT_8\INT_16                 | SHORT                 |
| UINT_16\INT_32                       | INT                 |
| TIMESTAMP_MICROS                    | NANOTIMESTAMP       |
| TIMESTAMP_MILLIS                    | TIMESTAMP           |
| DECIMAL                             | DOUBLE              |
| UINT_32\INT_64\UINT_64              | LONG                |
| TIME_MICROS                         | NANOTIME            |
| TIME_MILLIS                         | TIME                |
| DATE                                | DATE                |
| ENUM                                | SYMBOL              |
| UTF8                                | STRING              |
| MAP                                 | not support         |
| LIST                                | not support         |
| JSON                                | not support         |
| BSON                                | not support         |
| MAP_KEY_VALUE                       | not support         |

| Physical Type in Parquet    | Type in DolphinDB |
| ----------------- | :-------------------------- |
| BOOLEAN           | BOOL                        |
| INT32             | INT                         |
| INT64             | LONG                        |
| INT96             | NANOTIMESTAMP               |
| FLOAT             | FLOAT                       |
| DOUBLE            | DOUBLE                      |
| BYTE_ARRAY        | STRING                      |
| FIXED_LEN_BYTE_ARRAY | STRING                   |

> **请注意：**
>
>- 暂不支持转化 Parquet 中的 repeated 字段。
>- 在 Parquet 中标注了 DECIMAL 类型的字段中，仅支持转化原始数据类型（physical type）为 INT32, INT64 和 FIXED_LEN_BYTE_ARRAY 的数据。
>- 由于 DolphinDB 不支持无符号类型，所以读取 Parquet 中的 UINT_64 时若发生溢出，则会取 DolphinDB 中的 NULL 值。

### 3.2 导出

将 DolphinDB 数据导出为 Parquet 文件时，系统根据给出表的结构自动转换到 Parquet 文件支持的类型。

| Type in DolphinDB | Physical Type in Parquet | Logical Type in Parquet |
| ----------------- | ------------------------ | ----------------------- |
| BOOL              | BOOLEAN                  | \                       |
| CHAR              | FIXED_LEN_BYTE_ARRAY     | \                       |
| SHORT             | INT32                    | INT(16)                 |
| INT               | INT32                    | INT(32)                 |
| LONG              | INT64                    | INT(64)                 |
| DATE              | INT32                    | DATE                    |
| MONTH             | INT32                    | DATE                    |
| TIME              | INT32                    | TIME_MILLIS             |
| MINUTE            | INT32                    | TIME_MILLIS             |
| SECOND            | INT32                    | TIME_MILLIS             |
| DATETIME          | INT64                    | TIMESTAMP_MILLIS        |
| TIMESTAMP         | INT64                    | TIMESTAMP_MILLIS        |
| NANOTIME          | INT64                    | TIME_NANOS              |
| NANOTIMESTAMP     | INT64                    | TIMESTAMP_NANOS         |
| FLOAT             | FLOAT                    | \                       |
| DOUBLE            | DOUBLE                   | \                       |
| STRING            | BYTE_ARRAY               | STRING                  |
| SYMBOL            | BYTE_ARRAY               | STRING                  |

# Release Notes

## 2.00.11

### 新增功能

- 新增接口 `parquet::setReadThreadNum(num)`，用于设置插件的最大读线程数。
- 新增接口 `parquet::getReadThreadNum()`，用于获取插件的最大读线程数。

## 2.00.10

### 优化

- 优化了部分报错信息。
