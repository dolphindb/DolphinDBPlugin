# DolphinDB Feather Plugin

Apache Arrow Feather 文件采用列式存储格式，可用于高效存储与提取数据。DolphinDB 提供的 Feather 插件支持高效的将 Feather 文件导入和导出 DolphinDB，并且在导入导出过程中自动进行数据类型转换。

Feather 插件目前支持版本：[relsease200](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/feather/README.md), [release130](https://github.com/dolphindb/DolphinDBPlugin/blob/release130/feather/README.md)。您当前查看的插件版本为 release200，请使用 DolphinDB 2.00.X 版本 server。若使用其它版本 server，请切换至相应插件分支。

## 1 安装插件

### 1.1 Linux 编译

#### 初始化环境配置

(1) 编译 Feather 开发包：

```shell
git clone https://github.com/apache/arrow.git
cd arrow/cpp
mkdir build
cd build
cmake .. -DARROW_BUILD_STATIC=ON -DARROW_BUILD_SHARED=OFF -DARROW_DEPENDENCY_USE_SHARED=OFF -DARROW_WITH_ZLIB=ON -DARROW_WITH_ZSTD=ON -DARROW_WITH_LZ4=ON
make -j
```

(2) 编译完成后，拷贝以下文件到插件文件夹中的相应目录：

| **Files**                                                   | **Target Directory** |
| ------------------------------------------------------------ | -------------------- |
| arrow/cpp/src/arrow                                          | ./include            |
| arrow/cpp/build/release/libarrow.a<br/>arrow/cpp/build/jemalloc_ep-prefix/src/jemalloc_ep/lib/libjemalloc_pic.a<br/>arrow/cpp/build/zstd_ep-install/lib64/libzstd.a<br/>arrow/cpp/build/zlib_ep/src/zlib_ep-install/lib/libz.a<br/>arrow/cpp/build/lz4_ep-prefix/src/lz4_ep/lib/liblz4.a | ./lib/linux          |


**注意**
如果编译过程中出现上表 Files 列出的文件不存在，可以手动编译以下三个库。

1. 如果 libz.a 无法找到，执行以下命令：
```shell
wget http://www.zlib.net/zlib-1.2.12.tar.gz
tar -zxf zlib-1.2.12.tar.gz
cd zlib-1.2.12
CFLAGS="-fPIC" ./configure
make
```
然后在 zlib-1.2.12 目录下找到 libz.a，放到插件文件夹下的./lib/linux 目录中。

2. 如果 liblz4.a 无法找到，执行以下命令：
```shell
wget https://github.com/lz4/lz4/archive/8f61d8eb7c6979769a484cde8df61ff7c4c77765.tar.gz
tar -xzvf 8f61d8eb7c6979769a484cde8df61ff7c4c77765.tar.gz
cd lz4-8f61d8eb7c6979769a484cde8df61ff7c4c77765/
make
```
然后在 lz4-8f61d8eb7c6979769a484cde8df61ff7c4c77765/lib 目录下找到 liblz4.a，放到插件文件夹下的./lib/linux 目录中。

3. 如果 libzstd.a 无法找到，执行以下命令：
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
然后在 zstd-1.5.2/build/cmake/build/lib 目录下找到 libzstd.a，放到插件文件夹下的 ./lib/linux 目录中。

#### 编译插件
```linux shell
cd /path/to/plugins/feather
mkdir build
cd build
cmake ..
make
```
注意：编译之前请确保libDolphinDB.so在gcc可搜索的路径中。可使用LD_LIBRARY_PATH指定其路径，或者直接将其拷贝到build目录下。
### 1.2 DolphinDB 加载插件

```DolphinDB shell
loadPlugin("/path/to/plugin/PluginFeather.txt")
```

## 2 用户接口

### 2.1 feather::extractSchema

#### 语法

``` shell
feather::extractSchema(filePath)
```

#### 参数

- filePath：Feather 文件路径，类型为字符串标量

#### 详情

读取 Feather 文件数据的表结构，返回一张包含三列的表，第一列是列名，第二列是 Arrow 的数据类型，第三列是转换为 DolphinDB 的数据类型。
注意：如果 DolphinDBType 的某一行为 VOID，则说明 Feather 文件对应的列数据无法导入 DolphinDB。

#### 例子

```dolphindb
feather::extractSchema("path/to/data.feather");
feather::extractSchema("path/to/data.compressed.feather");
```

### 2.2 feather::load

#### 语法

``` shell
feather::load(filePath, [columns])
```

#### 参数

- filePath：Feather 文件路径，类型为字符串标量。
- columns：可选参数，字符串向量，表示要读取的列名集合。

#### 详情

将 Feather 文件数据加载到 DolphinDB 数据库的内存表。Feather 文件中的 Arrow 数据类型与 DolphinDB 数据类型的转化规则，参见 [数据类型](##3 支持的数据类型) 。

注意：

1. 由于 DolphinDB 整数类型的最小值表示空值，因此，Arrow int8, Arrow int16, Arrow int32, Arrow int64 类型对应的最小值无法导入 DolphinDB。

3. 浮点数的正负无穷、nan 值都会被转换为 DolphinDB 中的空值。

#### 例子

```dolphindb
table = feather::load("path/to/data.feather");
table_part = feather::load("path/to/data.feather", [ "col1_name","col2_name"]);
```

### 2.3 feather::save

#### 语法

``` shell
feather::save(table, filePath, [compressMethod], [compressionLevel])
```

#### 参数

- table：要保存的表。
- filePath：保存的文件路径，类型为字符串标量。
- compression：可选参数，类型为字符串标量，用于指定压缩类型。包含以下三种类型："uncompressed", "lz4", "zstd"，不区分大小写。本插件默认开启 lz4 压缩方式。
- compressionLevel：可选参数，类型为整型标量。只有 zstd 压缩类型能够指定压缩级别。

#### 详情

将 DolphinDB 中的表以 Feather 格式保存到文件中。关于 Feather 文件中的 Arrow 数据类型与 DolphinDB 数据类型的转化规则，参见 [数据类型](##3 支持的数据类型) 。


#### 例子

``` dolphindb
feather::save(table, "path/to/save/data.feather");
feather::save(table, "path/to/save/data.feather", "lz4");
feather::save(table, "path/to/save/data.feather", "zstd", 2);
```

### 2.4 完整示例

``` DolphinDB shell
loadPlugin("/path/to/plugins/feather/PluginFeather.txt")
feather::extractSchema("path/to/data.feather");
table = feather::load("path/to/data.feather");
table_part = feather::load("path/to/data.feather", [ "col1_name","col2_name"]);
feather::save(table, "path/to/save/data.feather");
feather::save(table, "path/to/save/data.feather", "lz4");
feather::save(table, "path/to/save/data.feather", "zstd", 2);
```

## 3 支持的数据类型

### 3.1 导入

DolphinDB 导入 Feather 文件时，Arrow 与 DolphinDB 数据类型转换关系如下：

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

不支持转换以下 Arrow 类型：binary, fixed_size_binary, half_float, timestamp(us), time64(us), interval_months, interval_day_time, decimal128, decimal, decimal256, list, struct, sparse_union, dense_union, dictionary, map, extension, fixed_size_list, large_string, large_binary, large_list, interval_month_day_nano, max_id

### 3.2 导出

DolphinDB 导出 feather 文件时，DolphinDB 与 Arrow 数据类型的对应关系如下：

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


不支持转换以下 DolphinDB 类型：MINUTE, MONTH, DATETIME, UUID, FUNCTIONDEF, HANDLE, CODE, DATASOURCE, RESOURCE, ANY, COMPRESS, ANY DICTIONARY, DATEHOUR, IPADDR, INT128, BLOB, COMPLEX, POINT, DURATION

### 3.3 Python 读取 Feather 文件

本节介绍通过 Python 读取 Feather 文件时，可能遇到的问题，给出了相应的解决方案：

1. Feather 文件中如果包含 time64(ns) 类型的数据，通过 `pyarrow.feather.read_feather()` 方法读取可能会报错 `Value XXXXXXXXXXXXX has non-zero nanoseconds`。这是因为 pyarrow.lib.Table 在转换为 DataFrame 时，time64(ns) 类型会被转换为 datetime.time 类型，而后者不支持纳秒精度的时间数据。建议使用 `pyarrow.feather.read_table()` 方法进行读取。

2. 通过 `pyarrow.feather.read_feather()` 读取的 Feather 文件若存在包含空值整型列，则会把该整型列转成浮点类型。建议先将 Feather 读到 pyarrow table 里，通过指定 types_mapper 转换类型。
    ```python
    pa_table = feather.read_table("path/to/feather_file")
    df = pa_table.to_pandas(types_mapper={pa.int64(): pd.Int64Dtype()}.get)
    ```