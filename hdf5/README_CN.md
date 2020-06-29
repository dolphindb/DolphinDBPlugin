# DolphinDB HDF5 Plugin

DolphinDB HDF5插件可將HDF5数据集导入进DolphinDB中,并且支持对数据类型转换。


* [安装](#安装)
    * [预编译安装](#预编译安装)
    * [编译安装](#编译安装)

* [用户接口](#用户接口)  
    * [hdf5::ls](#hdf5ls)
    * [hdf5::lsTable](#hdf5lstable)
    * [hdf5::extractHDF5Schema](#hdf5extracthdf5schema)
    * [hdf5::loadHDF5](#hdf5loadhdf5)
    * [hdf5::loadHDF5Ex](#hdf5loadhdf5ex)
    * [hdf5::HDF5DS](#hdf5hdf5ds)
* [支持的数据类型](#支持的数据类型) 
    * [integer](#integer)
    * [float](#float)
    * [time](#time)
    * [string](#string)
    * [enum](#enum)
    * [compound](#compound-and-array)
    * [array](#compound-and-array)
* [表结构](#表结构)  
    * [简单类型](#简单类型)
    * [复杂类型](#复杂类型)
* [性能](#性能数据)  

## 安装
### 预编译安装

用户可以导入预编译好的HDF5插件（DolphinDB安装包中或者bin目录下。

#### Linux
1） 将插件所在路径倒入到`LD_LIBRARY_PATH`
```
export LD_LIBRARY_PATH=/path_to_hdf5_plugin/:$LD_LIBRARY_PATH
```

2）启动DolphinDB server并导入插件
```
loadPlugin("/path_to_hdf5_plugin/PluginHdf5.txt")
```
#### Windows

```
loadPlugin("/path_to_hdf5_plugin/PluginHdf5.txt")
```


### 编译安装

用户可以通过以下方法自己编译HDF5插件。 编译成功后通过上面方法导入。

### 在Linux下安装

#### 使用cmake构建
**Note:** [cmake](https://cmake.org/) 是一个流行的项目构建工具,可以帮你轻松的解决第三方依赖的问题  

安装cmake
```
sudo apt-get install cmake
```
安装HDF5开发包
```
sudo apt-get install libhdf5-dev
```
构建整个工程
```
mkdir build
cd build
cmake ../path_to_hdf5_plugin/
make
```

#### 使用makefile构建
安装HDF5开发包

```
sudo apt-get install libhdf5-dev
```
执行make构建
```
make
```

编译之前请确保`libDolphinDB.so`在gcc可搜索的路径中,可使用`LD_LIBRARY_PATH`指定其路径

编译之后目录下会产生libPluginHdf5.so文件 

### 在Windows下安装

在Windows下安装，需要启用HDF5的`enable threadsafe`选项。这需要在编译HDF5时配置。

你可以使用我们预先编译的1.10.2版本的`hdf5.dll`文件。

你也可以自己选择HDF5的版本编译，在HDF5的[官方网站](https://www.hdfgroup.org/solutions/hdf5/)下载源代码，按照说明，如果采用configure方式编译，在配置时启用`--enable-threadsafe`, `--disable-cxx`, `--enable-shared`选项，如果采用CMake方式编译，在编译时启用`-DHDF5_ENABLE_THREADSAFE:BOOL=ON`, `-DHDF5_BUILD_CPP_LIB:BOOL=OFF`, `-DBUILD_SHARED_LIBS:BOOL=ON`参数。



## 用户接口


### hdf5::ls

### 语法

* `hdf5::ls(fileName)`

### 参数  

* `fileName`: HDF5文件名，类型为string。

### 详情
* 列出一张表中的所有`HDF5对象`包括`数据集(dataset)`和`组(group)`以及`(命名类型)namedType`,对于数据集,我们会返回他的尺寸,尺寸是列优先的,对于比如DataSet{(7,3)}代表7列3行。

### 例子   
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
## hdf5::lsTable

### 语法

* `hdf5::lsTable(fileName)`

### 参数  

* `fileName`: HDF5文件名，类型为string。

### 详情
* 列出一张表中的所有table信息，即HDF5`数据集(dataset)`对象信息，包括表名，表大小，表类型。

### 例子   
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

## hdf5::extractHDF5Schema

### 语法

* `hdf5::extractHDF5Schema(fileName,datasetName)`

### 参数
* `fileName`: HDF5文件名，类型为字符串标量。
* `datasetName`: dataset名称，即表名，可通过ls或lsTable获得，类型为字符串标量。

### 详情
* 生成输入数据集的表的结构。表的结构有两列：列名和数据类型。

### 例子
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

## hdf5::loadHDF5

### 语法

* `hdf5::loadHDF5(fileName,datasetName,[schema],[startRow],[rowNum])`

### 参数
* `fileName`: HDF5文件名，类型为字符串标量。
* `datasetName`: dataset名称，即表名，可通过ls或lsTable获得，类型为字符串标量。
* `schema`: 包含列名和列的数据类型的表。如果我们想要改变由系统自动决定的列的数据类型，需要在`schema`表中修改数据类型，并且把它作为`loadHDF5`函数的一个参数。
* `startRow`: 读取HDF5数据集的起始行数，若不指定，默认从数据集起始位置读取。
* `rowNum`: 读取HDF5数据集的行数，若不指定，默认读到数据集的结尾。

### 详情
* 将HDF5文件中的数据集转换为DolphinDB数据库的内存表。
* 读取的行数为HDF5文件中定义的行数，而不是读取结果中的DolphinDB表的行数。
* 支持的数据类型,以及数据转化规则可见[数据类型](#支持的数据类型)章节。

### 例子
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

> **Note:** **数据集的dataspace维度必须小于等于2,只有二维或一维表可以被解析**

## hdf5::loadHDF5Ex

### 语法

* `hdf5::loadHDF5Ex(dbHandle,tableName,[partitionColumns],fileName,datasetName,[schema],[startRow],[rowNum])`

### 参数
* `dbHandle`和`tableName`: 如果我们要将输入数据文件保存在分布式数据库中，需要指定数据库句柄和表名。
* `partitionColumns`: 字符串标量或向量，表示分区列。当分区数据库不是SEQ分区时，我们需要指定分区列。在组合分区中，partitionColumns是字符串向量。
* `fileName`: HDF5文件名，类型为字符串标量。
* `datasetName`: dataset名称，即表名，可通过ls或lsTable获得，类型为字符串标量。
* `schema`: 包含列名和列的数据类型的表。如果我们想要改变由系统自动决定的列的数据类型，需要在`schema`表中修改数据类型，并且把它作为`loadHDF5Ex`函数的一个参数。
* `startRow`: 读取HDF5数据集的起始行数，若不指定，默认从数据集起始位置读取。
* `rowNum`: 读取HDF5数据集的行数，若不指定，默认读到数据集的结尾。

### 详情
* 将HDF5文件中的数据集转换为DolphinDB数据库的分布式表，然后将表的元数据加载到内存中。
* 读取的行数为HDF5文件中定义的行数，而不是读取结果中的DolphinDB表的行数。
* 支持的数据类型,以及数据转化规则可见[数据类型](#支持的数据类型)章节。

### 例子
* 磁盘上的SEQ分区表
```
db = database("seq_on_disk", SEQ, 16)
hdf5::loadHDF5Ex(db,`tb,,"/large_file.h5", "large_table")
```

* 内存中的SEQ分区表
```
db = database("", SEQ, 16)
hdf5::loadHDF5Ex(db,`tb,,"/large_file.h5", "large_table")
```

* 磁盘上的非SEQ分区表
```
db = database("non_seq_on_disk", RANGE, 0 500 1000)
hdf5::loadHDF5Ex(db,`tb,`col_4,"/smpl_numeric.h5","sint")
```

* 内存中的非SEQ分区表
```
db = database("", RANGE, 0 500 1000)
t0 = hdf5::loadHDF5Ex(db,`tb,`col_4,"/smpl_numeric.h5","sint")
```

## hdf5::HDF5DS

### 语法

* `hdf5::HDF5DS(fileName,datasetName,[schema],[dsNum])`

### 参数
* `fileName`: HDF5文件名，类型为字符串标量。
* `datasetName`: dataset名称，即表名，可通过ls或lsTable获得，类型为字符串标量。
* `schema`: 包含列名和列的数据类型的表。如果我们想要改变由系统自动决定的列的数据类型，需要在`schema`表中修改数据类型，并且把它作为`HDF5DS`函数的一个参数。
* `dsNum`: 需要生成的数据源数量。`HDF5DS`会将整个表均分为`dsNum`份作为结果。如果不指定，默认生成1个数据源。

### 详情
* 根据输入的文件名和数据集名创建数据源列表。

### 例子
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

# 支持的数据类型

文件中的浮点和整数类型会被先转换为H5T_NATIVE_*类型(通过H5Tget_native_type)

## integer
| type in hdf5      | corresponding c type        | corresponding dolphindb type |
| ----------------- | :-------------------------- | :--------------------------- |
| H5T_NATIVE_CHAR   | signed char / unsigned char | char/short                   |
| H5T_NATIVE_SCHAR  | signed char                 | char                         |
| H5T_NATIVE_UCHAR  | unsigned char               | short                        |
| H5T_NATIVE_SHORT  | short                       | short                        |
| H5T_NATIVE_USHORT | unsigned short              | int                          |
| H5T_NATIVE_INT    | int                         | int                          |
| H5T_NATIVE_UINT   | unsigned int                | long                         |
| H5T_NATIVE_LONG   | long                        | int/long                     |
| H5T_NATIVE_ULONG  | unsigned long               | unsupported/long             |
| H5T_NATIVE_LLONG  | long long                   | long                         |
| H5T_NATIVE_ULLONG | unsigned long long          | unsupported                  |


* dolphindb中数值类型都为有符号类型,为了防止溢出,所有无符号类型会被转化为```高一阶的有符号类型,64位无符号类型不予支持```  

* H5T_NATIVE_CHAR 对应c中的char类型,而char是否有符号依赖与编译器及平台,若有符号,着在dolphindb中转化为char,否则为short

* H5T_NATIVE_LONG以及H5T_NATIVE_ULONG 对应c中的long类型,long的大小依赖与编译器及平台,若long大小与int相同,着在转化过程中与int类型相同,若long大小与long long相同,着转化过程与long long相同

* 所有整数类型皆可以转化为dolphindb中的数值类型```(bool,char,short,int,long,float,double)```，若进行转化时会发生**溢出**，比如LONG->INT,则会返回一个int的最值

## float
| type in hdf5      | corresponding c type | corresponding dolphindb type |
| ----------------- | :------------------- | :--------------------------- |
| H5T_NATIVE_FLOAT  | float                | float                        |
| H5T_NATIVE_DOUBLE | double               | double                       |

注:IEEE754浮点数类型皆为有符号数

* 所有浮点数类型皆可以转化为dolphindb中的数值类型`(bool,char,short,int,long,float,double)`，若进行转化时会发生**溢出**，比如DOUBLE->FLOAT,则会返回一个float的最值

## time
| type in hdf5   | corresponding c type | corresponding dolphindb type |
| -------------- | :------------------- | :--------------------------- |
| H5T_UNIX_D32BE | 4 bytes integer      | DT_TIMESTAMP                 |
| H5T_UNIX_D32LE | 4 bytes integer      | DT_TIMESTAMP                 |
| H5T_UNIX_D64BE | 8 bytes integer      | DT_TIMESTAMP                 |
| H5T_UNIX_D64LE | 8 bytes integer      | DT_TIMESTAMP                 |

* HDF5预定义的时间类型为**posix时间**,32位或者64位,HDF5的时间类型缺乏官方的定义，在此插件中，32位时间类型**代表距离1970年的秒数**，而64位则**精确到毫秒**。所有时间类型会被插件统一转化为一个64位整数,然后转化成dolphindb中的`timestamp`类型

* 以上类型皆可以转化为dolphindb中的时间相关类型`(date,month,time,minute,second,datetime,timestamp,nanotime,nanotimestamp)`

## string
| type in hdf5 | corresponding c type | corresponding dolphindb type |
| ------------ | :------------------- | :--------------------------- |
| H5T_C_S1     | char*                | DT_STRING                    |

* H5T_C_S1,包括```固定长度(fixed-length)```字符串和```可变长度(variable-length)```字符串

* string类型可以转化为转化为dolphindb中的字符串相关类型```(string,symbol)```

## enum
| type in hdf5 | corresponding c type | corresponding dolphindb type |
| ------------ | :------------------- | :--------------------------- |
| ENUM         | enum                 | DT_SYMBOL                    |

* 枚举类型会被转化为dolphindb中的一个symbol变量，值得注意的是，每个字符串所对应的枚举值以及大小关系**并不会被保存**.即，定义了一个枚举类型 HDF5_ENUM{"a"=100，"b"=2000,"c"=30000},那么可能会被转化为 SYMBOL{"a"=3,"b"=1"c"=2}

* enum类型可以转化为dolphindb中的字符串相关类型```(string,symbol)```

## compound and array
| type in hdf5 | corresponding c type | corresponding dolphindb type |
| ------------ | :------------------- | :--------------------------- |
| H5T_COMPOUND | struct               | \                            |
| H5T_ARRAY    | array                | \                            |

* 复合(compound)类型以及数组(array)类型,只要这些复杂类型中不包含不支持的类型,就可以被解析,而且支持嵌套

* 复杂类型的转化依赖与其内部子类型



# 表结构

## 简单类型
对于简单类型,导入到dolphindb后的table与h5文件中的table会保持```相同```

### HDF5中的简单类型表

|     | 1       | 2       |
| --- | :------ | :------ |
| 1   | int(10) | int(67) |
| 2   | int(20) | int(76) |

### 导入进dolphindb后的简单类型表

|     | col_1 | col_2 |
| --- | :---- | :---- |
| 1   | 10    | 67    |
| 2   | 20    | 76    |

## 复杂类型
对于复杂类型,dolphindb中的表的类型依赖与复杂类型的结构

###  HDF5中的复合类型表

|     | 1                        | 2                        |
| --- | :----------------------- | :----------------------- |
| 1   | struct{a:1 b:2 c:3.7}    | struct{a:12 b:22 c:32.7} |
| 2   | struct{a:11 b:21 c:31.7} | struct{a:13 b:23 c:33.7} |

###  导入进dolphindb后的复合类型表
|     | a    | b    | c    |
| --- | :--- | :--- | :--- |
| 1   | 1    | 2    | 3.7  |
| 2   | 11   | 21   | 31.7 |
| 3   | 12   | 22   | 32.7 |
| 4   | 13   | 23   | 33.7 |

### HDF5中的数组类型表

|     | 1             | 2               |
| --- | :------------ | :-------------- |
| 1   | array(1,2,3)  | array(4,5,6)    |
| 2   | array(8,9,10) | array(15,16,17) |

### 导入进dolphindb后的数组类型表
|     | array_1 | array_2 | array_3 |
| --- | :------ | :------ | :------ |
| 1   | 1       | 2       | 3       |
| 2   | 4       | 5       | 6       |
| 3   | 8       | 9       | 10      |
| 4   | 15      | 16      | 17      |

对于嵌套的复杂类型,我们用一个```A```前缀代表这是个数组,```C```前缀代表这是个复合类型

### HDF5中的嵌套类型表
|     | 1                                                         | 2                                                          |
| --- | :-------------------------------------------------------- | :--------------------------------------------------------- |
| 1   | struct{a:array(1,2,3)<br>  b:2<br>  c:struct{d:"abc"}}    | struct{a:array(7,8,9)<br>  b:5<br>  c:struct{d:"def"}}     |
| 2   | struct{a:array(11,21,31)<br>  b:0<br>  c:struct{d:"opq"}} | struct{a:array(51,52,53)<br>  b:24<br>  c:struct{d:"hjk"}} |


### 导入进dolphindb后的嵌套类型表

|     | Aa_1 | Aa_2 | Aa_3 | b    | Cc_d |
| --- | :--- | :--- | :--- | :--- | :--- |
| 1   | 1    | 2    | 3    | 2    | abc  |
| 2   | 7    | 8    | 9    | 5    | def  |
| 3   | 11   | 21   | 31   | 0    | opq  |
| 4   | 51   | 52   | 53   | 24   | hjk  |


# 性能数据

## 环境
* cpu: i7-7700 3.60GHZ
* ssd: read 460~500MB/S

## 数据集导入性能

* 单一类型(int)  
    * 行数 1024 * 1024 * 16 
    * 列数 64
    * 文件大小 4G
    * 时间 8s
* 单一类型(usigned int)
    * 行数 1024 * 1024 * 16 
    * 列数 64
    * 文件大小 4G
    * 时间 9s
* 单一类型(variable-length string)
    * 行数 1024 * 1024 
    * 列数 64
    * 文件大小 3.6G
    * 时间 17s
* 复合类型
    * 子类型共9列,分别为 (str,str,double,int,long,float,int,short,char)
    * 数据总量 1024 * 1024 * 62
    * 文件大小 3.9G
    * 时间 10 s
* 数组复合类型
    * 子类型共72列,为(str,str,double,int,long,float,int,short,char) * 8
    * 数据总量 1024 * 128 * 62
    * 文件大小 3.9G
    * 时间 15 s
