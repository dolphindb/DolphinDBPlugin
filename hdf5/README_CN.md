# DolphinDB HDF5 Plugin

DolphinDB HDF5 插件可將 HDF5 文件导入 DolphinDB，并支持进行数据类型转换。

HDF5 插件目前支持版本：[relsease200](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/hdf5/README_CN.md), [release130](https://github.com/dolphindb/DolphinDBPlugin/blob/release130/hdf5/README_CN.md), [release120](https://github.com/dolphindb/DolphinDBPlugin/blob/release120/hdf5/README_CN.md), [release110](https://github.com/dolphindb/DolphinDBPlugin/blob/release110/hdf5/README_CN.md)。您当前查看的插件版本为 release200，请使用 DolphinDB 2.00.X 版本 server。若使用其它版本 server，请切换至相应插件分支。

- [DolphinDB HDF5 Plugin](#dolphindb-hdf5-plugin)
  - [1 安装](#1-安装)
    - [1.1 预编译安装](#11-预编译安装)
    - [1.2 编译安装](#12-编译安装)
  - [2 用户接口](#2-用户接口)
    - [2.1 hdf5::ls](#21-hdf5ls)
    - [2.2 hdf5::lsTable](#22-hdf5lstable)
    - [2.3 hdf5::extractHDF5Schema](#23-hdf5extracthdf5schema)
    - [2.4 hdf5::loadHDF5](#24-hdf5loadhdf5)
    - [2.5 hdf5::loadPandasHDF5](#25-hdf5loadpandashdf5)
    - [2.6 hdf5::loadHDF5Ex](#26-hdf5loadhdf5ex)
    - [2.7 hdf5::HDF5DS](#27-hdf5hdf5ds)
    - [2.8 hdf5::saveHDF5](#28-hdf5savehdf5)
  - [3 支持的数据类型](#3-支持的数据类型)
    - [3.1 integer](#31-integer)
    - [3.2 float](#32-float)
    - [3.3 time](#33-time)
    - [3.4 string](#34-string)
    - [3.5 enum](#35-enum)
    - [3.6 compound and array](#36-compound-and-array)
  - [4 表结构](#4-表结构)
    - [4.1 简单类型](#41-简单类型)
    - [4.2 复杂类型](#42-复杂类型)
  - [5 性能](#5-性能)
    - [5.1 环境](#51-环境)
    - [5.2 数据集导入性能](#52-数据集导入性能)
- [ReleaseNotes:](#releasenotes)
  - [v2.00.10](#v20010)
    - [bug修复](#bug修复)
## 1 安装

### 1.1 预编译安装

可以导入 DolphinDB 安装包中或者 bin 目录下预编译好的 HDF5 插件。

#### Linux <!-- omit in toc -->

(1) 添加插件所在路径到 LIB 搜索路径 LD_LIBRARY_PATH
```
export LD_LIBRARY_PATH=/path_to_hdf5_plugin/:$LD_LIBRARY_PATH
```

(2) 启动 DolphinDB server 并导入插件
```
loadPlugin("/path_to_hdf5_plugin/PluginHdf5.txt")
```

#### Windows <!-- omit in toc -->

必须通过绝对路径加载，且路径中使用"\\\\"或"/"代替"\\"。
```
loadPlugin("path_to_hdf5_plugin/PluginHdf5.txt")
```

### 1.2 编译安装

可使用以下方法编译 HDF5 插件，编译成功后通过以上方法导入。

#### 在 Linux 下安装 <!-- omit in toc -->

安装 cmake：
```bash
sudo apt install cmake
```
编译 c-blosc
```bash
#在 https://github.com/Blosc/c-blosc/releases/tag/v1.21.1 下载源码
cd c-blosc-1.21.1
mkdir build
cd build
cmake -DCMAKE_C_FLAGS="-fPIC -std=c11" ..
make -j
cp blosc/src/blosc.h /path_to_hdf5_plugin/include/c-blosc
cp blosc/src/blosc-export.h /path_to_hdf5_plugin/include/c-blosc
cp blosc/libblosc.a /path_to_hdf5_plugin/lib
```

编译 HDF5 1.13.1：
```bash
# git clone https://github.com/HDFGroup/hdf5/ -b hdf5-1_13_1
# 若您不熟悉插件源代码，请不要下载其他版本，hdf5 版本兼容性差，可能导致安装失败
cd hdf5-1.13.1
export CC=gcc-4.8
export CXX=g++-4.8
export CFLAGS="-fPIC -std=c11"
export CXXFLAGS="-fPIC -std=c++11"
./configure --enable-cxx
make
make check
make install
make check-install
cp hdf5/include/* /path_to_hdf5_plugin/include/
cp hdf5/lib/libhdf5.a /path_to_hdf5_plugin/lib
cp hdf5/lib/libhdf5_cpp.a /path_to_hdf5_plugin/lib
cp hdf5/lib/libhdf5_hl.a /path_to_hdf5_plugin/lib
```
编译整个项目：
```bash
mkdir build
cd build
cp /path_to_dolphindb/libDolphinDB.so ./
cmake ..
make
```

#### 在 Windows 下安装 <!-- omit in toc -->
在 msys2 环境下，使用 mingw 编译 hdf5 1.13.1.zip
+ 使用 mingw-w64-x86_64 编译器
```
export PATH=/PATH_to mingw-w64/x86_64-5.3.0-win32-seh-rt_v4-rev0/mingw64/bin/:$PATH
```
+ 安装 make
```
pacman -S make
```

+ 打开 msys2 终端，进入已经解压好的 hdf5-1.13.1 所在目录
```
 CFLAGS="-std=c11" CXXFLAGS="-std=c++11" ./configure --host=x86_64-w64-mingw32 --build=x86_64-w64-mingw32 --prefix=/d/hdf5_1.13.1 --enable-cxx --enable-tests=no --enable-tools=no with_pthread=no
```
+ 打开 src/H5pubconf.h,在末尾添加以下宏定义
    ```
    #ifndef H5_HAVE_WIN32_API
    #define H5_HAVE_WIN32_API 1
    #endif

    #ifndef H5_HAVE_MINGW
    #define H5_HAVE_MINGW 1
    #endif
    ```
+ 开始编译
```
make -j8
+ make install -j8
```

+ 拷贝编译文件到 hdf5 插件文件目录
```
cp $HOME/hdf5/include/* /path_to_hdf5_plugin/include_win/hdf5
cp $HOME/hdf5/lib/libhdf5.a /path_to_hdf5_plugin/build
cp $HOME/hdf5/lib/libhdf5_cpp.a /path_to_hdf5_plugin/build
cp $HOME/hdf5/lib/libhdf5_hl.a /path_to_hdf5_plugin/build
cp $HOME/hdf5/lib/libhdf5_hl_cpp.a /path_to_hdf5_plugin/build
```

+ 打开 Windows 的 CMD 命令行终端

+ 编译 c_blosc
```
cd c_blosc-1.21.1
mkdir build
cd build
cmake  ../ -G "MinGW Makefiles"
mingw32-make -j8
copy .\blosc\libblosc.a /path_to_hdf5_plugin\build\
```
+ 复制 libDolphinDB.dll 到 hdf5 编译目录
```
copy /path_to_dolphindb/libDolphinDB.dll /path_to_hdf5_plugin/build
```

+ hdf5 插件编译
```
cmake  ../ -G "MinGW Makefiles"
mingw32-make -j
```

## 2 用户接口

### 2.1 hdf5::ls

#### 语法 <!-- omit in toc -->

hdf5::ls(fileName)

#### 参数  <!-- omit in toc -->

* fileName: HDF5 文件名，类型为 string。

#### 详情 <!-- omit in toc -->

列出一个 HDF5 文件中的所有对象(数据集(dataset)和组(group))以及对象类型(objType)。在对象类型中，数据集会包括其列数及行数。例如 DataSet{(7,3)}代表 7 列 3 行。

#### 例子   <!-- omit in toc -->
```
hdf5::ls("/smpl_numeric.h5")

output:
        objName	     objType
        --------------------
        /            Group
        /double	     DataSet{(7,3)}
        /float	     DataSet{(7,3)}
        /schar	     DataSet{(7,3)}
        /sint	     DataSet{(7,3)}
        /slong	     DataSet{(7,3)}
        /sshort	     DataSet{(7,3)}
        /uchar	     DataSet{(7,3)}
        /uint	     DataSet{(7,3)}
        /ulong	     DataSet{(1,1)}
        /ushort	     DataSet{(7,3)}

hdf5::ls("/named_type.h5")

output:
        objName      objType
        ----------------------
        /            Group
        /type_name   NamedDataType
```
### 2.2 hdf5::lsTable

#### 语法 <!-- omit in toc -->

hdf5::lsTable(fileName)

#### 参数  <!-- omit in toc -->

* fileName: HDF5 文件名，类型为 string。

#### 详情 <!-- omit in toc -->

列出一个 HDF5 文件中的所有 table 信息，即 HDF5 数据集(dataset)对象信息，包括表名、列数及行数、表的类型。

#### 例子   <!-- omit in toc -->
```
hdf5::lsTable("/smpl_numeric.h5")

output:
       tableName    tableDims	 tableType
       /double        7,3       H5T_NATIVE_DOUBLE
       /float	      7,3       H5T_NATIVE_FLOAT
       /schar	      7,3       H5T_NATIVE_SCHAR
       /sint	      7,3       H5T_NATIVE_INT
       /slong	      7,3       H5T_NATIVE_LLONG
       /sshort	      7,3       H5T_NATIVE_SHORT
       /uchar	      7,3       H5T_NATIVE_UCHAR
       /uint	      7,3       H5T_NATIVE_UINT
       /ulong	      1,1       H5T_NATIVE_ULLONG
       /ushort	      7,3       H5T_NATIVE_USHORT
```

### 2.3 hdf5::extractHDF5Schema

#### 语法 <!-- omit in toc -->

hdf5::extractHDF5Schema(fileName, datasetName)

#### 参数 <!-- omit in toc -->

* fileName: HDF5 文件名，类型为字符串标量。
* datasetName: dataset 名称，即表名。可通过 ls 或 lsTable 获得，类型为字符串标量。

#### 详情 <!-- omit in toc -->

生成 HDF5 文件中指定数据集的结构，包括两列：列名和数据类型。

#### 例子 <!-- omit in toc -->
```
hdf5::extractHDF5Schema("/smpl_numeric.h5","sint")

output:
        name	type
        col_0	INT
        col_1	INT
        col_2	INT
        col_3	INT
        col_4	INT
        col_5	INT
        col_6	INT


hdf5::extractHDF5Schema("/compound.h5","com")

output:
        name	type
        fs	STRING
        vs	STRING
        d	DOUBLE
        t	TIMESTAMP
        l	LONG
        f	FLOAT
        i	INT
        s	SHORT
        c	CHAR
```

### 2.4 hdf5::loadHDF5

#### 语法 <!-- omit in toc -->

hdf5::loadHDF5(fileName,datasetName,[schema],[startRow],[rowNum])

#### 参数 <!-- omit in toc -->

* fileName: HDF5 文件名，类型为字符串标量。
* datasetName: dataset 名称，即表名。可通过 ls 或 lsTable 获得，类型为字符串标量。
* schema: 包含列名和列的数据类型的表。若要改变由系统自动决定的列的数据类型，需要在 schema 表中修改数据类型，并且把它作为`loadHDF5`函数的一个参数。
* startRow: 从哪一行开始读取 HDF5 数据集。若不指定，默认从数据集起始位置读取。
* rowNum: 读取 HDF5 数据集的行数。若不指定，默认读到数据集的结尾。

#### 详情 <!-- omit in toc -->

将 HDF5 文件中的指定数据集加载为 DolphinDB 数据库的内存表。读取的行数为 HDF5 文件中定义的行数，而不是读取结果中的 DolphinDB 表的行数。支持的数据类型，以及数据转化规则可见[数据类型](#3-支持的数据类型)章节。

#### 例子 <!-- omit in toc -->
```
hdf5::loadHDF5("/smpl_numeric.h5","sint")

output:
        col_0	col_1	col_2	col_3	col_4	col_5	col_6
        (758)	8	(325,847)	87	687	45	90
        61	0	28	77	546	789	45
        799	5,444	325,847	678	90	54	0


scm = table(`a`b`c`d`e`f`g as name, `CHAR`BOOL`SHORT`INT`LONG`DOUBLE`FLOAT as type)
hdf5::loadHDF5("../hdf5/h5file/smpl_numeric.h5","sint",scm,1,1)

output:
        a	b	c	d	e	f	g
        '='	false	28	77	546	789	45
```

> **请注意：数据集的 dataspace 维度必须小于等于 2。只有一维或二维表可以被解析。**

### 2.5 hdf5::loadPandasHDF5

#### 语法 <!-- omit in toc -->

hdf5::loadPandasHDF5(fileName,groupName,[schema],[startRow],[rowNum])

#### 参数 <!-- omit in toc -->

* fileName: 由 Pandas 保存的 HDF5 文件名，类型为字符串标量。
* groupName: group 的标识符，即 key 名。
* schema: 包含列名和列的数据类型的表。若要改变由系统自动决定的列的数据类型，需要在 schema 表中修改数据类型，并且把它作为`loadPandasHDF5`函数的一个参数。
* startRow: 从哪一行开始读取 HDF5 数据集。若不指定，默认从数据集起始位置读取。
* rowNum: 读取 HDF5 数据集的行数。若不指定，默认读到数据集的结尾。

#### 详情 <!-- omit in toc -->

将由 Pandas 保存的 HDF5 文件中的指定数据表加载为 DolphinDB 数据库的内存表。读取的行数为 HDF5 文件中定义的行数，而不是读取结果中的 DolphinDB 表的行数。支持的数据类型，以及数据转化规则可见[数据类型](#3-支持的数据类型)章节。

#### 例子 <!-- omit in toc -->
```
hdf5::loadPandasHDF5("../hdf5/h5file/data.h5","/s",,1,1)

output:
        A	 B	C  D  E
        28 77	54 78 9
```

### 2.6 hdf5::loadHDF5Ex

#### 语法 <!-- omit in toc -->

hdf5::loadHDF5Ex(dbHandle, tableName, [partitionColumns], fileName, datasetName, [schema], [startRow], [rowNum], [transform])

#### 参数 <!-- omit in toc -->

* dbHandle 与 tableName: 若要将输入数据文件保存在分布式数据库中，需要指定数据库句柄和表名。
* partitionColumns: 字符串标量或向量，表示分区列。当分区数据库不是 SEQ 分区时，我们需要指定分区列。在组合分区中，partitionColumns 是字符串向量。
* fileName: HDF5 文件名，类型为字符串标量。
* datasetName: dataset 名称，即表名，可通过 ls 或 lsTable 获得，类型为字符串标量。
* schema: 包含列名和列的数据类型的表。如果我们想要改变由系统自动决定的列的数据类型，需要在 schema 表中修改数据类型，并且把它作为`loadHDF5Ex`函数的一个参数。
* startRow: 读取 HDF5 数据集的起始行位置。若不指定，默认从数据集起始位置读取。
* rowNum: 读取 HDF5 数据集的行数。若不指定，默认读到数据集的结尾。
* transform: 一元函数，并且该函数接受的参数必须是一个表。如果指定了 transform 参数，需要先创建分区表，再加载数据，程序会对数据文件中的数据执行 transform 参数指定的函数，再将得到的结果保存到数据库中。

#### 详情 <!-- omit in toc -->

将 HDF5 文件中的数据集转换为 DolphinDB 数据库的分布式表，然后将表的元数据加载到内存中。读取的行数为 HDF5 文件中定义的行数，而不是读取结果中的 DolphinDB 表的行数。支持的数据类型,以及数据转化规则可见[数据类型](#3-支持的数据类型)章节。

#### 例子 <!-- omit in toc -->

* 磁盘上的 SEQ 分区表
```
db = database("seq_on_disk", SEQ, 16)
hdf5::loadHDF5Ex(db,`tb,,"/large_file.h5", "large_table")
```

* 内存中的 SEQ 分区表
```
db = database("", SEQ, 16)
hdf5::loadHDF5Ex(db,`tb,,"/large_file.h5", "large_table")
```

* 磁盘上的非 SEQ 分区表
```
db = database("non_seq_on_disk", RANGE, 0 500 1000)
hdf5::loadHDF5Ex(db,`tb,`col_4,"/smpl_numeric.h5","sint")
```

* 内存中的非 SEQ 分区表
```
db = database("", RANGE, 0 500 1000)
t0 = hdf5::loadHDF5Ex(db,`tb,`col_4,"/smpl_numeric.h5","sint")
```

* 内存中的非 SEQ 分区表
```
db = database("", RANGE, 0 500 1000)
t0 = hdf5::loadHDF5Ex(db,`tb,`col_4,"/smpl_numeric.h5","sint")
```

* 指定 transform 将数值类型表示的日期和时间(比如:20200101)转化为指定类型(比如:日期类型)
```
dbPath="dfs://DolphinDBdatabase"
db=database(dbPath,VALUE,2020.01.01..2020.01.30)
dataFilePath="/transform.h5"
datasetName="/SZ000001/data"
schemaTB=hdf5::extractHDF5Schema(dataFilePath,datasetName)
update schemaTB set type="DATE" where name="trans_time"
tb=table(1:0,schemaTB.name,schemaTB.type)
tb1=db.createPartitionedTable(tb,`tb1,`trans_time);
def i2d(mutable t){
    return t.replaceColumn!(`trans_time,datetimeParse(string(t.trans_time),"yyyyMMdd"))
}
t = hdf5::loadHDF5Ex(db,`tb1,`trans_time,dataFilePath,datasetName,,,,i2d)
```
### 2.7 hdf5::HDF5DS

#### 语法 <!-- omit in toc -->

hdf5::HDF5DS(fileName,datasetName,[schema],[dsNum])

#### 参数 <!-- omit in toc -->

* fileName: HDF5 文件名，类型为字符串标量。
* datasetName: 数据集名，即表名。可通过`ls`或`lsTable`获得，类型为字符串标量。
* schema: 包含列名和列的数据类型的表。若要改变由系统自动决定的列的数据类型，需要在 schema 表中修改数据类型，并且把它作为`HDF5DS`函数的一个参数。
* dsNum: 需要生成的数据源数量。整个表会被均分为 dsNum 份。如果不指定，默认生成 1 个数据源。

#### 详情 <!-- omit in toc -->

根据输入的文件名和数据集名创建数据源列表。

#### 例子 <!-- omit in toc -->
```
>ds = hdf5::HDF5DS(smpl_numeric.h5","sint")
>size ds;
1
>ds[0];
DataSource< loadHDF5("/smpl_numeric.h5", "sint", , 0, 3) >

>ds = hdf5::HDF5DS(smpl_numeric.h5","sint",,3)
>size ds;
3
>ds[0];
DataSource< loadHDF5("/smpl_numeric.h5", "sint", , 0, 1) >
>ds[1];
DataSource< loadHDF5("/smpl_numeric.h5", "sint", , 1, 1) >
>ds[2];
DataSource< loadHDF5("/smpl_numeric.h5", "sint", , 2, 1) >
```
> **请注意：HDF5 不支持并行读入。以下例子是错误示范和正确示范**
错误示范
```
ds = hdf5::HDF5DS("/smpl_numeric.h5", "sint", ,3)
res = mr(ds, def(x) : x)
```
正确示范，将 mr parallel 参数设为 false
```
ds = hdf5::HDF5DS("/smpl_numeric.h5", "sint", ,3)
res = mr(ds, def(x) : x,,,false)
```
### 2.8 hdf5::saveHDF5

#### 语法 <!-- omit in toc -->

hdf5::saveHDF5(table, fileName, datasetName, [append], [stringMaxLength])

#### 参数 <!-- omit in toc -->

table: 要保存的内存表。

fileName: HDF5 文件名，类型为字符串标量。

datasetName: dataset 名称，即表名。可通过 ls 或 lsTable 获得，类型为字符串标量。

append: 是否追加数据到已存在 dataset。类型为布尔型，默认为 false。

stringMaxLength: 字符串最大长度，类型为数值类型，默认为 16。仅对 table 中的 string 和 symbol 类型起作用。

#### 详情 <!-- omit in toc -->

将 DolphinDB 数据库的内存表保存到 HDF5 文件中的指定数据集。支持的数据类型，以及数据转化规则可见[数据类型](#3-支持的数据类型)章节。

#### 例子 <!-- omit in toc -->

```
hdf5::saveHDF5(tb, "example.h5", "dataset name in hdf5")
```

> 注意：
>
> 1. HDF5 文件中无法存入空值。若 DolphinDB 表中存在空值，会按照[数据类型](#3-支持的数据类型)中的默认值存入。
> 2. 若需通过 python 读取由 HDF5 插件生成的 h5 文件，需要使用通过 h5py 库进行读取。例如：
> ```python
> import h5py
> f = h5py.File("/home/workDir/dolphindb_src/build/test.h5", 'r')
> print(f['aaa']['TimeStamp'])
> print(f['aaa']['StockID'])
> ```

## 3 支持的数据类型

浮点和整数类型会被先转换为 H5T_NATIVE_*类型。

### 3.1 integer
| Type in HDF5      | Default Value in HDF5 | Type in C        | Type in DolphinDB |
| ----------------- | :-------------------------- | :--------------------------- | :--------------------------- |
| H5T_NATIVE_CHAR   | ‘\0’ | signed char / unsigned char | char/short                   |
| H5T_NATIVE_SCHAR  | ‘\0’ | signed char                 | char                         |
| H5T_NATIVE_UCHAR  | ‘\0’ | unsigned char               | short                        |
| H5T_NATIVE_SHORT  | 0 | short                       | short                        |
| H5T_NATIVE_USHORT | 0 | unsigned short              | int                          |
| H5T_NATIVE_INT    | 0   | int                         | int                          |
| H5T_NATIVE_UINT   | 0  | unsigned int                | long                         |
| H5T_NATIVE_LONG   | 0  | long                        | int/long                     |
| H5T_NATIVE_ULONG  | 0 | unsigned long               | unsupported/long             |
| H5T_NATIVE_LLONG  | 0 | long long                   | long                         |
| H5T_NATIVE_ULLONG | 0 | unsigned long long          | unsupported/long                  |


* DolphinDB 中数值类型都为有符号类型。为了防止溢出，除 64 位无符号类型外，所有无符号类型会被转化为高一阶的有符号类型。特别的，64 位无符号类型转化为 64 位有符号类型，若发生溢出则返回 64 位有符号类型的最大值。

* H5T_NATIVE_CHAR 对应 C 中的 char 类型，而 char 是否有符号依赖与编译器及平台。若有符号依赖，则在 DolphinDB 中转化为 char，否则为 short。

* H5T_NATIVE_LONG 以及 H5T_NATIVE_ULONG 对应 C 中的 long 类型。

* 所有整数类型皆可以转化为 DolphinDB 中的数值类型(bool, char, short, int, long, float, double)，若进行转化会发生溢出。例如 LONG->INT 会返回一个 int 的最值。

### 3.2 float
| Type in HDF5      | Default Value in HDF5 | Type in C | Type in DolphinDB |
| ----------------- | :------------------- | :--------------------------- | :--------------------------- |
| H5T_NATIVE_FLOAT  | +0.0f | float                | float                        |
| H5T_NATIVE_DOUBLE | +0.0 | double               | double                       |

注意：IEEE754 浮点数类型皆为有符号数。

* 所有浮点数类型皆可以转化为 DolphinDB 中的数值类型(bool, char, short, int, long, float, double)，若进行转化会发生溢出。例如 DOUBLE->FLOAT 会返回一个 float 的最值。

### 3.3 time
| type in hdf5   | Default Value in HDF5   | corresponding c type | corresponding dolphindb type |
| -------------- | ----------------------- | :------------------- | :--------------------------- |
| H5T_UNIX_D32BE | 1970.01.01T00:00:00     | 4 bytes integer      | DT_TIMESTAMP                 |
| H5T_UNIX_D32LE | 1970.01.01T00:00:00     | 4 bytes integer      | DT_TIMESTAMP                 |
| H5T_UNIX_D64BE | 1970.01.01T00:00:00.000 | 8 bytes integer      | DT_TIMESTAMP                 |
| H5T_UNIX_D64LE | 1970.01.01T00:00:00.000 | 8 bytes integer      | DT_TIMESTAMP                 |

* HDF5 预定义的时间类型为 32 位或者 64 位的 posix 时间。HDF5 的时间类型缺乏官方的定义，在此插件中，32 位时间类型代表距离 1970 年的秒数，而 64 位则精确到毫秒。所有时间类型会被插件统一转化为一个 64 位整数，然后转化为 DolphinDB 中的 timestamp 类型。

* 以上类型皆可以转化为 DolphinDB 中的时间相关类型(date, month, time, minute, second, datetime, timestamp, nanotime, nanotimestamp)。

### 3.4 string
| type in hdf5 | Default Value in HDF5 | corresponding c type | corresponding dolphindb type |
| ------------ | --------------------- | :------------------- | :--------------------------- |
| H5T_C_S1     | “”                    | char*                | DT_STRING                    |

* H5T_C_S1 包括固定长度(fixed-length)字符串和可变长度(variable-length)字符串。

* string 类型可以转化为 DolphinDB 中的字符串相关类型(string, symbol)。

### 3.5 enum
| type in hdf5 | corresponding c type | corresponding dolphindb type |
| ------------ | :------------------- | :--------------------------- |
| ENUM         | enum                 | DT_SYMBOL                    |

* 枚举类型会被转化为 DolphinDB 中的一个 symbol 变量。请注意，每个字符串所对应的枚举值以及大小关系并不会被保存。例如，枚举类型 HDF5_ENUM{"a"=100，"b"=2000,"c"=30000}可能会被转化为 SYMBOL{"a"=3,"b"=1"c"=2}。

* enum 类型可以转化为 DolphinDB 中的字符串相关类型(string, symbol)。

### 3.6 compound and array
| type in hdf5 | corresponding c type | corresponding dolphindb type |
| ------------ | :------------------- | :--------------------------- |
| H5T_COMPOUND | struct               | \                            |
| H5T_ARRAY    | array                | \                            |

* 复合(compound)类型以及数组(array)类型只要不包含不支持的类型，就可以被解析，而且支持嵌套。

* 复杂类型的转化取决于其内部子类型。


## 4 表结构

### 4.1 简单类型

简单类型导入 DolphinDB 后的 table 与 HDF5 文件中的 table 完全一致。

#### HDF5 中的简单类型表 <!-- omit in toc -->

|     | 1       | 2       |
| --- | :------ | :------ |
| 1   | int(10) | int(67) |
| 2   | int(20) | int(76) |

#### 导入 DolphinDB 后的简单类型表 <!-- omit in toc -->

|     | col_1 | col_2 |
| --- | :---- | :---- |
| 1   | 10    | 67    |
| 2   | 20    | 76    |

### 4.2 复杂类型

复杂类型导入 DolphinDB 后的类型取决于复杂类型的结构。

####  HDF5 中的复合类型表 <!-- omit in toc -->

|     | 1                        | 2                        |
| --- | :----------------------- | :----------------------- |
| 1   | struct{a:1 b:2 c:3.7}    | struct{a:12 b:22 c:32.7} |
| 2   | struct{a:11 b:21 c:31.7} | struct{a:13 b:23 c:33.7} |

导入 DolphinDB 后：

|     | a    | b    | c    |
| --- | :--- | :--- | :--- |
| 1   | 1    | 2    | 3.7  |
| 2   | 11   | 21   | 31.7 |
| 3   | 12   | 22   | 32.7 |
| 4   | 13   | 23   | 33.7 |

#### HDF5 中的数组类型表 <!-- omit in toc -->

|     | 1             | 2               |
| --- | :------------ | :-------------- |
| 1   | array(1,2,3)  | array(4,5,6)    |
| 2   | array(8,9,10) | array(15,16,17) |

导入 DolphinDB 后：

|     | array_1 | array_2 | array_3 |
| --- | :------ | :------ | :------ |
| 1   | 1       | 2       | 3       |
| 2   | 4       | 5       | 6       |
| 3   | 8       | 9       | 10      |
| 4   | 15      | 16      | 17      |


#### HDF5 中的嵌套类型表 <!-- omit in toc -->

对嵌套的复杂类型，结果中会添加'A'前缀代表数组，'C'前缀代表复合类型。

|     | 1                                                         | 2                                                          |
| --- | :-------------------------------------------------------- | :--------------------------------------------------------- |
| 1   | struct{a:array(1,2,3)<br>  b:2<br>  c:struct{d:"abc"}}    | struct{a:array(7,8,9)<br>  b:5<br>  c:struct{d:"def"}}     |
| 2   | struct{a:array(11,21,31)<br>  b:0<br>  c:struct{d:"opq"}} | struct{a:array(51,52,53)<br>  b:24<br>  c:struct{d:"hjk"}} |

导入 DolphinDB 后：

|     | Aa_1 | Aa_2 | Aa_3 | b    | Cc_d |
| --- | :--- | :--- | :--- | :--- | :--- |
| 1   | 1    | 2    | 3    | 2    | abc  |
| 2   | 7    | 8    | 9    | 5    | def  |
| 3   | 11   | 21   | 31   | 0    | opq  |
| 4   | 51   | 52   | 53   | 24   | hjk  |


## 5 性能

### 5.1 环境

* CPU: i7-7700 3.60GHZ
* SSD: 连续读取 最多每秒 460~500MB

### 5.2 数据集导入性能

* 单一类型(int)
    * 行数 1024 * 1024 * 16
    * 列数 64
    * 文件大小 4G
    * 耗时 8 秒
* 单一类型(unsigned int)
    * 行数 1024 * 1024 * 16
    * 列数 64
    * 文件大小 4G
    * 耗时 9 秒
* 单一类型(variable-length string)
    * 行数 1024 * 1024
    * 列数 64
    * 文件大小 3.6G
    * 耗时 17 秒
* 复合类型
    * 子类型共 9 列：(str,str,double,int,long,float,int,short,char)
    * 行数 1024 * 1024 * 62
    * 文件大小 3.9G
    * 耗时 10 秒
* 数组复合类型
    * 子类型共 72 列：(str,str,double,int,long,float,int,short,char) * 8
    * 行数 1024 * 128 * 62
    * 文件大小 3.9G
    * 耗时 15 秒

# Release Notes

## 2.00.11

### 新功能

- 接口 `hdf5::loadPandasHDF5` 新增对表类型 series_table, frame_table 支持索引方式 multiIndex 。

## 2.00.10

### bug修复

- 修复使用方法 hdf5::ls 执行特定类型的 hdf5 文件后 server 宕机的问题。
- 修复并行导入多个文件时 server 宕机的问题。
