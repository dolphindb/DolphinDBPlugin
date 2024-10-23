# DolphinDB Parquet Plugin

Apache Parquet is a columnar storage format for efficient data storage and retrieval. This tutorial introduces how to use DolphinDB Parquet plugin to import and export Parquet files to and from DolphinDB.

The DolphinDB Parquet plugin has the branches [release 200](https://dolphindb.net/dzhou/DolphinDBPlugin/-/blob/release200/parquet/README.md) and [release130](https://dolphindb.net/dzhou/DolphinDBPlugin/-/blob/release130/parquet/README.md). Each plugin version corresponds to a DolphinDB server version. You're looking at the plugin documentation for release200. If you use a different DolphinDB server version, please refer to the corresponding branch of the plugin documentation.

* [1 Install the Plugin](#1-install-the-plugin)
    * [1.1 Download Precompiled Binaries](#11-download-precompiled-binaries)
    * [1.2 (Optional) Build a Plugin](#12-optional-build-a-plugin)
    * [1.3 Load the Plugin](#13-load-the-plugin)
* [2 Methods](#2-methods)  
    * [2.1 parquet::extractParquetSchema](#21-parquetextractparquetschema)
    * [2.2 parquet::loadParquet](#22-parquetloadparquet)
    * [2.3 parquet::loadParquetEx](#23-parquetloadparquetex)
    * [2.4 parquet::parquetDS](#24-parquetparquetds)
    * [2.5 parquet::saveParquet](#25-parquetsaveparquet)
* [3 Data Type Mappings](#3-data-type-mappings) 
    * [3.1 Import](#31-import)
    * [3.2 Export](#32-export)

## 1 Install the Plugin

### 1.1 Download Precompiled Binaries

You can download precompiled binaries for DolphinDB Parquet Plugin [here](https://github.com/dolphindb/DolphinDBPlugin/tree/master/parquet/bin/linux64). It can be installed directly on Linux (See Chap 02).

Please note that the plugin version should be consistent with your DolphinDB server version. You can switch branches to obtain the expected version.

### 1.2 (Optional) Build a Plugin

You can also manually compile a Parquet plugin with [CMake](https://cmake.org/) on Linux following the instructions:

(1) Install CMake

```
sudo apt-get install cmake
```

(2) Install zlib

```
sudo apt-get install zlib1g
```

(3) Compile Parquet Development Kit

```
git clone https://github.com/apache/arrow.git
cd arrow/cpp
mkdir build
cd build
cmake .. -DARROW_PARQUET=ON 
        -DARROW_IPC=ON 
        -DARROW_BUILD_INTEGRATION=ON 
        -DARROW_BUILD_STATIC=ON 
        -DPARQUET_BUILD_SHARED=OFF 
        -DARROW_BUILD_SHARED=OFF 
        -DARROW_DEPENDENCY_USE_SHARED=OFF 
        -DARROW_WITH_ZLIB=ON 
        -DARROW_WITH_SNAPPY=ON 
        -DARROW_WITH_ZSTD=ON 
        -DARROW_WITH_LZ4=ON 
        -DARROW_WITH_BZ2=ON
```

> **Note:** 
The dependency library libstdc++ in the compilation environment must be the same as that of the DolphinDB server. The precompiled Parquet plugin supports 5 compression algorithms: zlib, snappy, zstd, lz4 and bz2. You can change the compression options as needed. For more details on the optional building components, see Building Arrow C++.

(4) After compiling, copy the following files to the target directories.

| **Files **                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                | **Target Directory**         |
|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|------------------------------|
| arrow/cpp/build/src/parquet/parquet_version.h                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             | ./parquetApi/include/parquet |
| arrow/cpp/src/arrow<br>arrow/cpp/src/parquet<br>arrow/cpp/src/generated                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   | ./parquetApi/include         |
| arrow/cpp/build/release/libarrow.a<br>arrow/cpp/build/release/libparquet.a<br>arrow/cpp/build/thrift_ep-install/lib/libthrift.a<br>arrow/cpp/build/utf8proc_ep-install/lib/libutf8proc.a<br>arrow/cpp/build/jemalloc_ep-prefix/src/jemalloc_ep/lib/libjemalloc_pic.a<br>arrow/cpp/build/zstd_ep-install/lib64/libzstd.a<br>arrow/cpp/build/zlib_ep/src/zlib_ep-install/lib/libz.a<br>arrow/cpp/build/snappy_ep/src/snappy_ep-install/lib/libsnappy.a<br>arrow/cpp/build/lz4_ep-prefix/src/lz4_ep/lib/liblz4.a<br>arrow/cpp/build/bzip2_ep-install/lib/libbz2.a<br>arrow/cpp/build/boost_ep-prefix/src/boost_ep/stage/lib/libboost_regex.a | ./parquetApi/lib/linux       |


(5) Build the entire project

```
mkdir build
cd build
cmake ../path_to_parquet_plugin/
make
```

### 1.3 Load the Plugin

Load the precompiled plugin provided by DolphinDB or the manually compiled plugin on Linux.

(1) Add the plugin path to the library search path LD_LIBRARY_PATH

```
export LD_LIBRARY_PATH=/path_to_parquet_plugin/:$LD_LIBRARY_PATH
```

(2) Start DolphinDB server and load the plugin
```
loadPlugin("/path_to_parquet_plugin/PluginParquet.txt")
```

## 2 Methods

### 2.1 parquet::extractParquetSchema

#### **Syntax**

parquet::extractParquetSchema(fileName)

#### **Parameters** 

* fileName: a STRING scalar indicating the Parquet file name.

#### **Details**

Return the schema of the input Parquet file. It includes 2 columns: column names and their data types.

#### **Examples**

```
parquet::extractParquetSchema("userdata1.parquet")
```

### 2.2 parquet::loadParquet

#### **Syntax**

parquet::loadParquet(fileName,[schema],[column],[rowGroupStart],[rowGroupNum])

#### **Parameters** 

* fileName: a STRING scalar indicating the Parquet file name.

* schema: a table with the column names and their data types. Specify the parameter to modify the data types of the columns generated by the system. It is an optional parameter.

* column: a vector of integers indicating the column index to be imported. It is an optional parameter and all columns will be read if it is not specified.

* rowGroupStart: a non-negative integer indicating the index of the row group from which the data import starts. It is an optional parameter, and the file will be read from the beginning if it is not specified.

* rowGroupNum: an integer indicating the number of row groups to be read. It is an optional parameter, and the method will read to the end of the file if it is not specified.

#### **Details**

Import a Parquet file to a DolphinDB in-memory table. 

Regarding data type conversion, please see [Data Type Mappings](#3-Data-Type-Mappings).

#### **Examples**

```
parquet::loadParquet("userdata1.parquet")
```

### 2.3 parquet::loadParquetEx

#### **Syntax**

parquet::loadParquetEx(dbHandle,tableName,[partitionColumns],fileName,[schema],[column],[rowGroupStart],[rowGroupNum],[tranform])

#### **Parameters** 

* dbHandle: a database handle

* tableName: a STRING indicating the table name

* partitionColumns: a STRING scalar or vector indicating the partitioning column(s). For a composite partition, it is a vector.

* fileName: a STRING scalar indicating the Parquet file name.

* schema: a table with the column names and their data types. Specify the parameter to modify the data types of the columns that are determined by DolphinDB automatically. It is an optional parameter.

* column: a vector of integers indicating the column index to be imported. It is an optional parameter and all columns will be read if it is not specified.

* rowGroupStart: a non-negative integer indicating the index of the row group from which the data import starts. It is an optional parameter, and the file will be read from the beginning if it is not specified.

* rowGroupNum: an integer indicating the number of row groups to be read. It is an optional parameter, and the method will read to the end of the file if it is not specified.

* transform: a unary function and the input argument must be a table. If it is specified, a partitioned table must be created before loading the file. The method will first apply the specified function to the data, and then save the result to the partitioned table.

#### **Details**

Load a Parquet file to a DolphinDB partitioned table and return a table object with metadata of the table.

* If *dbHandle* is specified and is not an empty string "": load the file to a DFS database.

* If *dbHandle* is an empty string "" or unspecified: load the file to a partitioned in-memory table.

Regarding data type conversion, please see [Data Type Mappings](#3-Data-Type-Mappings).

#### **Examples**

* Import to a partitioned DFS table

```
db = database("dfs://rangedb", RANGE, 0 500 1000)
parquet::loadParquetEx(db,`tb,`id,"userdata1.parquet")
```

* Import to a partitioned in-memory table

```
db = database("", RANGE, 0 500 1000)
parquet::loadParquetEx(db,`tb,`id,"userdata1.parquet")
```

* Specify the parameter *transform* to transform the default data type (e.g. 20200101) to a specific type (e.g. DATE)

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

#### **Syntax**

parquet::parquetDS(fileName,[schema])

#### **Parameters**

* fileName: a STRING scalar indicating the Parquet file name.

* schema: a table with the column names and their data types. Specify the parameter to modify the data types of the columns that are determined by DolphinDB automatically. It is an optional parameter.

#### **Details**

Create data sources based on the input file name. The number of tables is the same as the number of row groups.

#### **Examples**

```
>ds = parquet::parquetDS("userdata1.parquet")
>size ds;
1
>ds[0];
DataSource< loadParquet("userdata1.parquet",,,0,1) >
```

### 2.5 parquet::saveParquet

#### **Syntax**

parquet::saveParquet(table, fileName)

#### **Parameters**

* table: the table to be exported

* fileName: a STRING scalar indicating the Parquet file name to save the table

#### **Details**

Export a DolphinDB table to a Parquet file.

#### **Examples**

```
parquet::saveParquet(tb, "userdata1.parquet")
```

## 3 Data Type Mappings

### 3.1 Import

When a Parquet file is imported to DolphinDB, the data types are converted based on the `LogicalType` as annotated in the file. If the `LogicalType` or `ConvertedType` is not defined, the conversion will be performed based on the physical types.

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


> **Note:** 
    - Conversion of the Parquet repeated fields is not supported.
    - DECIMAL can be used to convert data of the following physical types: INT32, INT64 and FIXED_LEN_BYTE_ARRAY.
    - DolphinDB does not support unsigned data types. Therefore, in case of UINT_64 overflow when loading a Parquet file, the data will be converted to NULL values in DolphinDB.


### 3.2 Export

When exporting data from DolphinDB to a Parquet file, the system will convert the data types to Parquet types based on the given table schema.

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