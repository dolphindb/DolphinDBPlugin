# DolphinDB ORC Plugin

ORC 是一种自描述列式文件格式，专门为 Hadoop 生态设计，可用于高效存储与提取数据，因此适合大批量流数据读取场景。DolphinDB 提供的 ORC 插件支持将 ORC 文件导入和导出 DolphinDB，并且在导入导出过程中自动进行数据类型转换。

## 1 安装

> ORC 插件仅支持 Linux 系统

### 1.1 预编译安装

可以导入预编译好的 ORC 插件。

#### Linux

(1) 添加插件所在路径到 LIB 搜索路径 LD_LIBRARY_PATH

```bash
export LD_LIBRARY_PATH=path_to_orc_plugin/:$LD_LIBRARY_PATH
```

(2) 启动 DolphinDB server 并导入插件

```
loadPlugin("path_to_orc_plugin/PluginOrc.txt")
```

### 1.2 编译安装

使用以下方法编译 ORC 插件，编译成功后通过以上方法导入。

#### 1.2.1 在Linux-x86下安装

**安装 cmake：**

```bash
sudo apt-get install cmake
```

**安装 cyrus-sasl：**

下载源码：https://github.com/cyrusimap/cyrus-sasl/releases/download/cyrus-sasl-2.1.27/cyrus-sasl-2.1.27.tar.gz

```bash
tar -xf cyrus-sasl-2.1.27.tar.gz
cd cyrus-sasl-2.1.27
./configure CFLAGS=-fPIC CXXFLAGS=-fPIC LDFLAGS=-static
cp ./lib/.libs/libsasl2.a path_to_orc_plugin/lib/linux
```

**安装 openssl：**

```bash
wget https://www.openssl.org/source/old/1.0.2/openssl-1.0.2u.tar.gz
tar -xzf openssl-1.0.2u.tar.gz
cd openssl-1.0.2u
./config shared --prefix=/tmp/ssl -fPIC
make
make install

cp /tmp/ssl/lib/libcrypto.a path_to_orc_plugin/lib/linux
```


**安装 ORC 开发包：**

下载源码：https://www.apache.org/dyn/closer.cgi/orc/orc-1.6.7/orc-1.6.7.tar.gz

```bash
tar -xf orc-1.6.7.tar.gz
cd orc-1.6.7
mkdir build
cd build
cmake .. -DBUILD_JAVA=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON
make package
tar -xf ORC-1.6.7-Linux.tar.gz
cp ORC-1.6.7-Linux/lib/* path_to_orc_plugin/lib/linux
```

**重新编译 libprotobuf.a：**

```bash
cd orc-1.6.7/build/protobuf_ep-prefix/src/
tar -xf v3.5.1.tar.gz
cd protobuf-3.5.1
./autogen.sh
cd cmake
mkdir build
cd build
cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON
make
cp libprotobuf.a path_to_orc_plugin/lib/linux
```

> 请注意，执行 ./autogen.sh 后出现的 ./autogen.sh: line 50: autoreconf: command not found，并不影响接下来的步骤

**重新编译 libsnappy.a：**

```bash
cd orc-1.6.7/build/snappy_ep-prefix/src/
tar -xf 1.1.7.tar.gz
cd snappy-1.17
mkdir build
cd build
cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON
make
cp libsnappy.a path_to_orc_plugin/lib/linux
```

**重新编译 libz.a：**

```bash
cd orc-1.6.7/build/zlib_ep-prefix/src
tar -xf zlib-1.2.11.tar.gz
cd zlib-1.2.11
mkdir build
cd build
cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON
make
cp libz.a path_to_orc_plugin/lib/linux
```

**设置 libDolphinDB.so 库位置：**

```bash
export LIBDOLPHINDB=path_to_libdolphindb/
```

**编译整个项目：**

```bash
mkdir build
cd build
cmake path_to_orc_plugin/
make
```

#### 1.2.2 在Linux-arm下安装
先在arm机器上编译ssl, orc, protobuffer, snappy, zlib依赖库，并把编译出来的静态库拷贝到lib_arm目录。

**编译openssl：**
```
wget https://www.openssl.org/source/old/1.0.1/openssl-1.0.1u.tar.gz
tar -xzf openssl-1.0.1u.tar.gz
cd openssl-1.0.1u
./config shared --prefix=/tmp/ssl -fPIC
make
make install

```

**编译orc开发包：**

下载源码：https://www.apache.org/dyn/closer.cgi/orc/orc-1.6.7/orc-1.6.7.tar.gz

```bash
tar -xf orc-1.6.7.tar.gz
cd orc-1.6.7
mkdir build
cd build
CMAKE_PREFIX_PATH=/tmp/ssl cmake .. -DBUILD_JAVA=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON//指定里前面安装openssl1.0.1u的路径。
make package
tar -xf ORC-1.6.7-Linux.tar.gz
cp ORC-1.6.7-Linux/lib/* path_to_orc_plugin/lib_arm/
```

**重新编译libprotobuf.a：**

```bash
cd orc-1.6.7/build/protobuf_ep-prefix/src/
tar -xf v3.5.1.tar.gz
cd protobuf-3.5.1
./autogen.sh
cd cmake
mkdir build
cd build
cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON
make
cp libprotobuf.a path_to_orc_plugin/lib_arm/
```

**重新编译libsnappy.a：**

```bash
cd orc-1.6.7/build/snappy_ep-prefix/src/
tar -xf 1.1.7.tar.gz
cd snappy-1.1.7
mkdir build
cd build
cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON
make
cp libsnappy.a path_to_orc_plugin/lib_arm/
```

**重新编译libz.a：**

```bash
cd orc-1.6.7/build/zlib_ep-prefix/srcap
tar -xf zlib-1.2.11.tar.gz
cd zlib-1.2.11
mkdir build
cd build
cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON
make
cp libz.a path_to_orc_plugin/lib_arm/
```

使用交叉编译器在x86-64的机器上编译，将lib_arm文件夹从arm机器上复制过来。
**编译整个项目：**

```bash
mkdir build
cd build
cp ../lib_arm/* .
CC=aarch64-linux-gnu-gcc CXX=aarch64-linux-gnu-g++ cmake .. -DBUILD_ARM=1
make
```

## 2 用户接口

### 2.1 orc::extractORCSchema

#### 语法

`orc::extractORCSchema(fileName)`

#### 参数

- `fileName`: ORC 文件名，类型为 string。

#### 详情

解析指定 ORC 文件中数据集的结构。

#### 返回值

返回一个表，包括 name 和 type 两列，分别表示列名和数据类型。

#### 例子

`orc::extractORCSchema("userdata1.orc")`

### 2.2 orc::loadORC

#### 语法

`orc::loadORC(fileName,[schema],[column],[rowStart],[rowNum])`

#### 参数

- `fileName`: ORC 文件名，类型为字符串标量。
- `schema`: 包含列名和列的数据类型的表。若要改变列的数据类型，需要在 schema 表中修改数据类型。
- `column`: 整型向量，表示读取的列的索引。若不指定，读取所有列。
- `rowStart`: 从哪一行开始读取 ORC 文件。若不指定，默认从文件起始位置读取。
- `rowNum`: 读取行的数量。若不指定，默认读到文件的结尾。

#### 详情

将 ORC 文件数据加载为 DolphinDB 数据表的内存表。支持的 ORC 数据类型，以及数据转化规则可见数据类型章节。

#### 返回值

返回一个将 ORC 文件导入后的内存表。

#### 例子

`orc::loadORC("userdata1.orc")`

>  请注意：DolphinDB 中不支持下划线开头的列名。若 ORC 文件中含有以下划线开头的列名，系统自动增加字母 "Z" 作为前缀。

### 2.3 orc::loadORCEx

#### 语法

`orc::loadORCEx(dbHandle,tableName,[partitionColumns],fileName,[schema],[column],[rowStart],[rowNum],[transform])`

#### 参数

- `dbHandle` 与 ``tableName`: 数据库句柄和表名。
- `partitionColumns`: 字符串标量或向量，表示分区列。当分区数据库不是 SEQ 分区时，我们需要指定分区列。在组合分区中，`partitionColumns` 是字符串向量。
- `fileName`: ORC 文件名，类型为字符串标量。
- `schema`: 包含列名和列的数据类型的表。若要改变列的数据类型，需要在 schema 表中修改数据类型。
- `column`: 整型向量，表示读取的列的索引。若不指定，读取所有列。
- `rowStart`: 从哪一行开始读取 ORC 文件。若不指定，默认从文件起始位置读取。
- `rowNum`: 读取行的数量。若不指定，默认读到文件的结尾。
- `tranform`: 一元函数，并且该函数接受的参数必须是一个表。如果指定了 `transform` 参数，需要先创建分区表，再加载数据，程序会对数据文件中的数据执行 `transform` 参数指定的函数，再将得到的结果保存到数据库中。

#### 详情

将 ORC 文件数据转换为 DolphinDB 数据库的分布式表，然后将表的元数据加载到内存中。支持的 ORC 数据类型, 以及数据转化规则可见数据类型章节。

#### 返回值

返回一个包含分布式表元数据的表对象。

#### 例子

```
db = database("dfs://db1", RANGE, 0 500 1000)
orc::loadORCEx(db,`tb,`id,"userdata1.orc")
```

- 指定 `transform` 将将数值类型表示的日期和时间 (比如: 20200101) 转化为指定类型(比如: 日期类型)

```
dbPath="dfs://db2"
db=database(dbPath,VALUE,2020.01.01..2020.01.30)
dataFilePath="userdata1.orc"
schemaTB=orc::extractORCSchema(dataFilePath)
update schemaTB set type="DATE" where name="date"
tb=table(1:0,schemaTB.name,schemaTB.type)
tb1=db.createPartitionedTable(tb,`tb1,`date);
def i2d(mutable t){
    return t.replaceColumn!(`date,datetimeParse(t.date),"yyyy.MM.dd"))
}
t = orc::loadORCEx(db,`tb1,`date,dataFilePath,datasetName,,,,i2d)
```

### 2.4 orc::orcDS

#### 语法

`orc::orcDS(fileName,chunkSize,[schema],[skipRows])`

#### 参数

- `fileName`: ORC 文件名，类型为字符串标量。
- `chunkSize`: 每个数据源包含的行数。
- `schema`: 包含列名和列的数据类型的表。若要改变由系统自动决定的列的数据类型，需要在 schema 表中修改数据类型，并且把它作为 `loadORC` 函数的一个参数。
- `skipRows`: 从文件头开始忽略的行数，默认值为 0。

#### 详情

根据输入的文件名创建数据源列表, 数量由文件中的行数与 chunkSize 决定。

#### 返回值

返回由数据源组成的向量。

#### 例子

```
>ds = orc::orcDS("userdata1.orc", 1000)
>size ds;
1
>ds[0];
DataSource<loadORC("userdata1.orc", , 0, 1000) >
```

### 2.5 orc::saveORC

#### 语法

`orc::saveORC(table, fileName)`

#### 参数

- `table`: 要保存的内存表对象。
- `fileName`: 保存的 ORC 文件名，类型为字符串标量。

#### 详情

将 DolphinDB 内存表保存为 ORC 格式文件。
请注意，若向一个已存在的 ORC 文件中保存表数据，这会覆盖 ORC 文件原有内容。

#### 返回值

无。

#### 例子

`orc::saveORC(tb, "example.orc")`

## 3 支持的数据类型

ORC 的数据类型转换为 DolphinDB 数据类型对照表：

| Type in ORC | Type in DolphinDB |
| ----------- | ----------------- |
| boolean     | BOOL              |
| tinyint     | CHAR              |
| smallint    | SHORT             |
| int         | INT               |
| bigint      | LONG              |
| float       | FLOAT             |
| double      | DOUBLE            |
| string      | STRING            |
| char        | STRING            |
| varchar     | STRING            |
| binary      | not support       |
| timestamp   | NANOTIMESTAMP     |
| date        | DATE              |
| struct      | not support       |
| list        | not support       |
| map         | not support       |
| union       | not support       |

# Release Notes

## 1.30.23

### 故障修复

- 修复了数据量小的情况下，ORC 数据写入数据库失败的问题。

## 1.30.22

### 功能优化

- 优化了时间类型和字符串类型数据导入为数值类型的处理方法。

### 故障修复

- 修复了读取时间类型空值数据时输出结果不正确的问题。
