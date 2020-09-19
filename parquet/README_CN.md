# DolphinDB Parquet Plugin

DolphinDB Parquet插件可將Parquet文件导入DolphinDB，并支持进行数据类型转换。

* [1 安装](#1-安装)
    * [1.1 预编译安装](#11-预编译安装)
    * [1.2 编译安装](#12-编译安装)
* [2 用户接口](#2-用户接口)  
    * [2.1 parquet::extractParquetSchema](#21-parquetextractParquetSchema)
    * [2.2 parquet::loadParquet](#22-parquetloadparquet)
    * [2.3 parquet::loadParquetEx](#23-parquetloadParquetEx)
    * [2.4 parquet::parquetDS](#26-parquetparquetDS)
* [3 支持的数据类型](#3-支持的数据类型) 

## 1 安装

### 1.1 预编译安装

可以导入DolphinDB安装包中或者bin目录下预编译好的parquet插件。

#### Linux

(1) 添加插件所在路径到LIB搜索路径 LD_LIBRARY_PATH
```
export LD_LIBRARY_PATH=/path_to_parquet_plugin/:$LD_LIBRARY_PATH
```

(2) 启动DolphinDB server并导入插件
```
loadPlugin("/path_to_parquet_plugin/PluginParquet.txt")
```

### 1.2 编译安装

可使用以下方法编译parquet插件，编译成功后通过以上方法导入。

#### 在Linux下安装

##### 使用cmake构建

安装cmake：
```
sudo apt-get install cmake
```
安装zlib：
```
sudo apt-get install zlib1g
```
编译Parquet开发包：
```
git clone https://github.com/apache/arrow.git
cd arrow/cpp
mkdir build
cd build
cmake .. -DARROW_PARQUET=ON -DARROW_IPC=ON -DARROW_BUILD_INTEGRATION=ON -DARROW_BUILD_STATIC=ON -DPARQUET_BUILD_SHARED=OFF -DARROW_BUILD_SHARED=OFF -DARROW_DEPENDENCY_USE_SHARED=OFF -DARROW_WITH_ZLIB=ON -DARROW_WITH_SNAPPY=ON -DARROW_WITH_ZSTD=ON -DARROW_WITH_LZ4=ON -DARROW_WITH_BZ2=ON
```
> **请注意：编译环境中的libstdc++需要和dolphindb server下的一致, 可以根据需要支持的压缩类型增减编译选项，预编译版本支持zlib、snappy、zstd、lz4，bz2压缩格式，其余见[arrow](https://github.com/apache/arrow/blob/master/docs/source/developers/cpp/building.rst#optional-components)**

将arrow/cpp/src下的arrow、parquet、generated复制到./parquetApi/include下，将arrow/cpp/build/src/parquet/parquet_version.h复制到./parquetApi/include/parquet下,将arrow/cpp/build/release下libarrow.a、libparquet.a，arrow/cpp/build/thrift_ep-install/lib/libthrift.a, arrow/cpp/build/utf8proc_ep-install/lib/libutf8proc.a, cpp/build/jemalloc_ep-prefix/src/jemalloc_ep/lib/libjemalloc_pic.a, arrow/cpp/build/zstd_ep-install/lib64/libzstd.a,arrow/cpp/build/zlib_ep/src/zlib_ep-install/lib/libz.a,arrow/cpp/build/snappy_ep/src/snappy_ep-install/lib/libsnappy.a,/arrow/cpp/build/lz4_ep-prefix/src/lz4_ep/lib/liblz4.a,arrow/cpp/build/bzip2_ep-install/lib/libbz2.a,arrow/cpp/build/boost_ep-prefix/src/boost_ep/stage/lib/libboost_regex.a复制到./parquetApi/lib/linux。

编译整个项目：
```
mkdir build
cd build
cmake ../path_to_parquet_plugin/
make
```

## 2 用户接口

### 2.1 parquet::extractParquetSchema

#### 语法

parquet::extractParquetSchema(fileName)

#### 参数  

* fileName: parquet文件名，类型为string。

#### 详情

生成parquet文件中指定数据集的结构，包括两列：列名和数据类型。

#### 例子
```
parquet::extractParquetSchema("userdata1.parquet")
```

### 2.2 parquet::loadParquet

#### 语法

parquet::loadParquet(fileName,[schema],[column],[rowGroupStart],[rowGroupNum])

#### 参数

* fileName: parquet文件名，类型为字符串标量。
* schema: 包含列名和列的数据类型的表。若要改变由系统自动决定的列的数据类型，需要在schema表中修改数据类型，并且把它作为`loadparquet`函数的一个参数。
* column: 读取的列索引。若不指定，读取所有列。
* rowGroupStart: 从哪一个rowGroup开始读取parquet文件。若不指定，默认从文件起始位置读取。
* rowGroupNum: 读取rowGroup的数量。若不指定，默认读到文件的结尾。

#### 详情

将parquet文件数据加载为DolphinDB数据库的内存表。支持的数据类型，以及数据转化规则可见[数据类型](#3-支持的数据类型)章节。

#### 例子
```
parquet::loadParquet("userdata1.parquet")
```

### 2.3 parquet::loadParquetEx

#### 语法

parquet::loadParquetEx(dbHandle,tableName,[partitionColumns],fileName,[schema],[column],[rowGroupStart],[rowGroupNum],[tranform])

#### 参数

* dbHandle与tableName: 若要将输入数据文件保存在分布式数据库中，需要指定数据库句柄和表名。
* partitionColumns: 字符串标量或向量，表示分区列。当分区数据库不是SEQ分区时，我们需要指定分区列。在组合分区中，partitionColumns是字符串向量。
* fileName: parquet文件名，类型为字符串标量。
* schema: 包含列名和列的数据类型的表。若要改变由系统自动决定的列的数据类型，需要在schema表中修改数据类型，并且把它作为`loadparquet`函数的一个参数。
* column: 读取的列索引。若不指定，读取所有列。
* rowGroupStart: 从哪一个rowGroup开始读取parquet文件。若不指定，默认从文件起始位置读取。
* rowGroupNum: 读取rowGroup的数量。若不指定，默认读到文件的结尾。
* tranform: 一元函数，并且该函数接受的参数必须是一个表。如果指定了transform参数，需要先创建分区表，再加载数据，程序会对数据文件中的数据执行transform参数指定的函数，再将得到的结果保存到数据库中。
#### 详情

将parquet文件数据转换为DolphinDB数据库的分布式表，然后将表的元数据加载到内存中。支持的数据类型,以及数据转化规则可见[数据类型](#3-支持的数据类型)章节。

#### 例子

* 磁盘上的SEQ分区表
```
db = database("seq_on_disk", SEQ, 16)
parquet::loadParquetEx(db,`tb,,"userdata1.parquet")
```

* 内存中的SEQ分区表
```
db = database("", SEQ, 16)
parquet::loadParquetEx(db,`tb,,"userdata1.parquet")
```

* 磁盘上的非SEQ分区表
```
db = database("non_seq_on_disk", RANGE, 0 500 1000)
parquet::loadParquetEx(db,`tb,`id,"userdata1.parquet")
```

* 内存中的非SEQ分区表
```
db = database("", RANGE, 0 500 1000)
parquet::loadParquetEx(db,`tb,`id,"userdata1.parquet")
```

* 指定transform将将数值类型表示的日期和时间(比如:20200101)转化为指定类型(比如:日期类型)
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
### 2.6 parquet::parquetDS

#### 语法

parquet::parquetDS(fileName,[schema])

#### 参数

* fileName: parquet文件名，类型为字符串标量。
* schema: 包含列名和列的数据类型的表。若要改变由系统自动决定的列的数据类型，需要在schema表中修改数据类型，并且把它作为`loadparquet`函数的一个参数。

#### 详情

根据输入的文件名创建数据源列表,数量为rowGroup的数量。

#### 例子
```
>ds = parquet::parquetDS("userdata1.parquet")
>size ds;
1
>ds[0];
DataSource< loadParquet("userdata1.parquet",,,0,1) >
```

## 3 支持的数据类型
系统根据Parquet的原始数据类型结合LogicalType和ConvertedType做数据的类型转换，优先按照LogicalType。如果没有定义LogicalType或者ConvertedType，则只根据原始数据类型转换。

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

| Converted Type in Parquet    | Type in DolphinDB |
| --------------------------- | :-------------------------- |
| INT_8\UINT_8\UINT_16\INT_16\INT_32  | INT                 |
| TIMESTAMP_MICROS                    | NANOTIMESTAMP       |
| TIMESTAMP_MILLIS                    | TIMESTAMP           | 
| DECIMAL                             | DOUBLE              |
| UINT_32\INT_64                      | LONG                | 
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

| Logical Type in Parquet    | TimeUnit in Parquet     | Type in DolphinDB|
| -------------------------- | :--------------------------- |:-----------------|
| INT                        |          \                   | INT\LONG         |
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

> **请注意：parquet repeated field 还未支持, DECIMAL仅支持Physical Type为INT32或INT64**
