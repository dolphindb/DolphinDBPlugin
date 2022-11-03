# DolphinDB Feather Plugin

Feather uses the Apache Arrow columnar memory format for data, which is organized for efficient analytic operations. The DolphinDB Feather plugin supports efficient import and export of Feather files with automatic data type conversion.

The DolphinDB Feather plugin has the following versions: [release 200](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/parquet/README.md) and [release130](https://github.com/dolphindb/DolphinDBPlugin/blob/release130/parquet/README.md). Each plugin version corresponds to a DolphinDB server version. You're looking at the plugin documentation for release200. You're looking at the plugin documentation for release200. If you use a different DolphinDB server version, please refer to the corresponding version of the plugin documentation.

- [DolphinDB Feather Plugin](#dolphindb-feather-plugin)
  - [1 Install the Plugin](#1-install-the-plugin)
    - [1.1 Compile on Linux](#11-compile-on-linux)
    - [1.2 Load the Plugin](#12-load-the-plugin)
  - [2 Methods](#2-methods)
    - [2.1 feather::extractSchema](#21-featherextractschema)
    - [2.2 feather::load](#22-featherload)
    - [2.3 feather::save](#23-feathersave)
  - [3 Data Type Mappings](#3-data-type-mappings)
    - [3.1 Import](#31-import)
    - [3.2 Export](#32-export)

## 1 Install the Plugin

### 1.1 Compile on Linux

**Initialization**

(1) Compile the Feather Development Kit.

```shell
git clone https://github.com/apache/arrow.git
cd arrow/cpp
mkdir build
cd build
cmake .. -DARROW_BUILD_STATIC=ON -DARROW_BUILD_SHARED=OFF -DARROW_DEPENDENCY_USE_SHARED=OFF -DARROW_WITH_ZLIB=ON -DARROW_WITH_ZSTD=ON -DARROW_WITH_LZ4=ON
make -j
```

(2) After compiling, copy the following files to the target directories.

| **Files**                                                   | **Target Directory** |
| ------------------------------------------------------------ | -------------------- |
| arrow/cpp/src/arrow                                          | ./include            |
| arrow/cpp/build/release/libarrow.a<br/>arrow/cpp/build/jemalloc_ep-prefix/src/jemalloc_ep/lib/libjemalloc_pic.a<br/>arrow/cpp/build/zstd_ep-install/lib64/libzstd.a<br/>arrow/cpp/build/zlib_ep/src/zlib_ep-install/lib/libz.a<br/>arrow/cpp/build/lz4_ep-prefix/src/lz4_ep/lib/liblz4.a | ./lib/linux          |


**Note:**
If the files listed in the "Files" column do not exist during compilation, you can manually compile the following three libraries.

* If `libz.a` cannot be found, run the following command:

```shell
wget http://www.zlib.net/zlib-1.2.12.tar.gz
tar -zxf zlib-1.2.12.tar.gz
cd zlib-1.2.12
CFLAGS="-fPIC" ./configure
make
```
Find `libz.a` in the `zlib-1.2.12` directory and put it to the `./lib/linux` directory in the plugins folder.

* If `liblz4.a` cannot be found, run the following command:

```shell
wget https://github.com/lz4/lz4/archive/8f61d8eb7c6979769a484cde8df61ff7c4c77765.tar.gz
tar -xzvf 8f61d8eb7c6979769a484cde8df61ff7c4c77765.tar.gz
cd lz4-8f61d8eb7c6979769a484cde8df61ff7c4c77765/
make
```
Find `libz.a` in the `lz4-8f61d8eb7c6979769a484cde8df61ff7c4c77765/lib` directory and put it to the `./lib/linux` directory in the plugins folder.

* If  `libzstd.a`  cannot be found, run the following command:

```shell
wget https://github.com/facebook/zstd/releases/download/v1.5.2/zstd-1.5.2.tar.gz
tar -zxvf zstd-1.5.2.tar.gz
cd zstd-1.5.2/
cd build/cmake/
mkdir build
cd build/
cmake ..
make -j
```
Find `libz.a` in the `zstd-1.5.2/build/cmake/build/lib` directory and put it to the `./lib/linux` directory in the plugins folder.

(3) Build the Entire Project

```linux shell
cd /path/to/plugins/feather
mkdir build
cd build
cmake ..
make
```

**Note:** 
Make sure the file libDolphinDB.so is under the GCC search path before compilation. You can add the plugin path to the library search path `LD_LIBRARY_PATH` or copy it to the build directory.

### 1.2 Load the Plugin

```DolphinDB shell
loadPlugin("/path/to/plugin/PluginFeather.txt")
```

## 2 Methods

### 2.1 feather::extractSchema

**Syntax**

``` shell
feather::extractSchema(filePath)
```

**Parameters**

- filePath: a STRING scalar indicating the Feather file path.

**Details**

Get the schema of the Feature file and return a table containing the following three columns:

1. column names
2. data type of Arrow
3. data type of DolphinDB

**Note:**
If the value of a cell in column DolphinDB Type is VOID, it indicates that the corresponding data type in Arrow is not supported to be converted.

**Examples**

```dolphindb
feather::extractSchema("path/to/data.feather");
feather::extractSchema("path/to/data.compressed.feather");
```

### 2.2 feather::load

**Syntax**

``` shell
feather::load(filePath, [columns])
```

**Parameters**

- filePath: a STRING scalar indicating the Feather file path.
- columns: a STRING scalar indicating column names to be loaded. It is an optional parameter.

**Details**

Load a Feather file to a DolphinDB in-memory table. Regarding data type conversion, see [Data Type Mappings](##3 Data Type Mappings) 。

**Note:**
* Since the minimum of DolphinDB integral type is a NULL character, the minimum of Arrow int8, int16, int32, int64 cannot be imported into DolphinDB.
* The infinities and NaNs (not a number) of floating-point numbers are converted to NULL values in DolphinDB.

**Examples**

```dolphindb
table = feather::load("path/to/data.feather");
table_part = feather::load("path/to/data.feather", [ "col1_name","col2_name"]);
```

### 2.3 feather::save

**Syntax**

``` shell
feather::save(table, filePath, [compressMethod], [compressionLevel])
```

**Parameters**

- table: the table to be exported.
- filePath: a STRING scalar indicating the Feather file path.
- compression: a STRING scalar indicating the following three compression methods: "uncompressed", "lz4", "zstd" (case insensitive). The default is lz4. It is an optional parameter.
- compressionLevel: an integral scalar. It is an optional parameter only used for compression method “zstd”.

**Details**

Export a DolphinDB table to a Feather file. Regarding data type conversion, see [Data Type Mappings](##3 Data Type Mappings) 。

**Examples**

``` dolphindb
feather::save(table, "path/to/save/data.feather");
feather::save(table, "path/to/save/data.feather", "lz4");
feather::save(table, "path/to/save/data.feather", "zstd", 2);
```

## 3 Data Type Mappings

### 3.1 Import

The following is the data type mappings when a Feather file is imported to DolphinDB:

| Arrow           | DolphinDB |
| ----------------| :---------------- |
| bool            | BOOL              |
| int8            | CHAR              |
| uint8           | SHORT             |
| int16           | SHORT             |
| uint16          | INT               |
| int32           | INT               |
| uint32          | LONG              |
| int64           | LONG              |
| uint64          | LONG              |
| float           | FLOAT             |
| double          | DOUBLE            |
| string          | STRING            |
| date32          | DATE              |
| date64          | TIMESTAMP         |
| timestamp(ms)    | TIMESTAMP         |
| timestamp(ns)   | NANOTIMESTAMP     |
| time32(s)        | SECOND            |
| time32(ms)       | TIME              |
| time64(ns)       | NANOTIME          |

The following Arrow types are not supported for conversion: binary, fixed_size_binary, half_float, timestamp(us), time64(us), interval_months, interval_day_time, decimal128, decimal, decimal256, list, struct, sparse_union, dense_union, dictionary, map, extension, fixed_size_list, large_string, large_binary, large_list, interval_month_day_nano, max_id

### 3.2 Export

The following is the data type mappings when exporting data from DolphinDB to a Parquet file:

| DolphinDB         | Arrow           |
| ----------------- | :-------------- |
| BOOL              | bool            |
| CHAR              | int8            |
| SHORT             | int16           |
| INT               | int32           |
| LONG              | int64           |
| DATE              | date32          |
| TIME              | time32(ms)      |
| SECOND            | time32(s)       |
| TIMESTAMP         | timestamp(ms)   |
| NANOTIME          | time64(ns)      |
| NANOTIMESTAMP     | timestamp(ns)   |
| FLOAT             | float           |
| DOUBLE            | double          |
| STRING            | string          |
| SYMBOL            | string          |


The following DolphinDB data types are not supported for conversion: MINUTE, MONTH, DATETIME, UUID, FUNCTIONDEF, HANDLE, CODE, DATASOURCE, RESOURCE, ANY, COMPRESS, ANY DICTIONARY, DATEHOUR, IPADDR, INT128, BLOB, COMPLEX, POINT, DURATION

**Note:**

You may encounter some problems when reading Feather files using Python.

**Scenario 1:**
The error `Value XXXXXXXXXXXXX has non-zero nanoseconds` is raised when reading the Feather file contains data of type time64(ns) using pyarrow.feather.read_feather(). When a table is converted to a DataFrame, the time64(ns) type is converted to the datetime.time type, which does not support temporal data in nanosecond.

**Solution:** 
It is recommended to read with function pyarrow.feather.read_table().

**Scenario 2:**
Use `pyarrow.feather.read_feather()` to read Feather files that contain null integer columns will convert the integer columns to floating point types.

**Solution:**
It is recommended to read Feather files into the pyarrow table and convert the data type by specifying `types_mapper`.

    ```python
    pa_table = feather.read_table("path/to/feather_file")
    df = pa_table.to_pandas(types_mapper={pa.int64(): pd.Int64Dtype()}.get)
    ```
