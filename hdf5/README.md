# DolphinDB HTTP Client Plugin

使用该插件可以便捷地进行HTTP请求,邮件发送。

## 构建

需要首先构建`libcurl`, `libssl`(版本为1.0.2), `libcrypto`(版本为1.0.2)和libz的静态链接库。

<<<<<<< HEAD
在使用`make`构建时，需要指定`CURL_DIR`, `SSL_DIR`和`Z_DIR`（假定`libssl.a`和`libcrypto.a`在同一个目录下）。例如：

```
CURL_DIR=/home/zmx/curl-7.47.0 SSL_DIR=/home/zmx/openssl-1.0.2i Z_DIR=/home/zmx/zlib-1.2.11 make
=======
* [Build](#Build)  
    * [Build with cmake](#Build with cmake)
    * [Build with makefile](#Build with makefile)
* [User-API](#User API)  
    * [hdf5::ls](#hdf5ls)
    * [hdf5::lsTable](#hdf5lstable)
    * [hdf5::extractHDF5Schema](#hdf5extracthdf5schema)
    * [hdf5::loadHDF5](#hdf5loadhdf5)
    * [hdf5::loadHDF5Ex](#hdf5loadhdf5ex)
    * [hdf5::HDF5DS](#hdf5hdf5ds)
    * [hdf5::saveHDF5](#hdf5savehdf5)
* [Data Types](#Data Types) 
    * [integer](#integer)
    * [float](#float)
    * [time](#time)
    * [string](#string)
    * [enum](#enum)
    * [compound](#compound-and-array)
    * [array](#compound-and-array)
* [Table Struct](#Table Struct)  
    * [Simple datatype table struct](#Simple datatype table struct)
    * [complex datatype table struct](#complex datatype table struct)
* [Performance](#Performance)  

# Build

## For Linux users

### Build with cmake
**Note:** [cmake](https://cmake.org/) is a popular project build tool that can help you easily solve third-party dependencies. 

Install cmake
```bash
sudo apt install cmake
```

Install HDF5 development kit
```bash
# download source code at https://portal.hdfgroup.org/display/support/HDF5+1.10.6#files
# DO NOT download other versions unless you are familar with the source code of plugin
tar -xvf hdf5-1.10.6.tar.gz
cd hdf5-1.10.6
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
Build the entire project

```bash
mkdir build
cd build
cp /path_to_dolphindb/libDolphinDB.so ./
cmake ..
make
>>>>>>> c64b2c7... update hdf5
```

## 使用

<<<<<<< HEAD
编译生成`libPluginHttpClient.so`之后，通过以下脚本加载插件：

```
loadPlugin("/path/to/PluginHttpClient.txt");
```
=======
Install HDF5 development kit in the same way of Linux

Execute "make" command

```powershell
mkdir build
cd build
cmake ..
make
```

The libPluginHdf5.so file will be generated after the compilation.

## For Windows users

The `enable threadsafe` configuration must be used for Windows. It should be set when compiling HDF5.

You may use `hdf5.dll` file that have been prebuilt.

You can also build HDF5 yourself. Download the source code from the [official website of HDF5](https://www.hdfgroup.org/solutions/hdf5/), and follow the instructions to build. If you are to build HDF5 with configure, the `--enable-threadsafe` option must be used. If you are to build HDF5 with CMake, the `-DHDF5_ENABLE_THREADSAFE:BOOL=ON`, `-DHDF5_BUILD_CPP_LIB:BOOL=OFF`, `-DBUILD_SHARED_LIBS:BOOL=ON` must be added when compiling.
>>>>>>> c64b2c7... update hdf5

### httpClient::httpGet(url,[params],[timeout],[headers])

发送HTTP GET请求。
参数
* url：为请求的URL字符串。
* params：为一个字符串或一个键和值都是string的字典。http协议Get方法请求的会把参数放在url的后面。
假如url为 www.dolphindb.cn，
1. 如果params为一个字符串("example"),则发出的完整http报文的请求头中的url为 "www.dolphindb.cn?example"。
2. 如果params为一个字典(两个键值对"name"->"zmx"和"id"->"111"),则发出的完整http报文的请求头中的url为 "www.dolphindb.cn?id=111&name=zmx"。
* timeout：为超时时间，单位为毫秒。
* headers：为一个字符串或一个键和值都是string的字典,填写http请求头部。 如果headers为一个字典(两个键值对"groupName"->"dolphindb"和"groupId"->"11"),
则发出的完整http报文添加请求头"groupId:11"和"groupName:dolphindb"。如果只是一个字符串，则必须是"xx:xx"格式，会添加一个http请求头。

返回一个dictionary，键包括：
- `responseCode`: 请求返回的响应码。
- `headers`: 请求返回的头部。
- `text`: 请求返回的内容文本。
- `elapsed`: 请求经过的时间。

例子
```
loadPlugin('/home/zmx/worker/DolphinDBPlugin/httpClient/PluginHttpClient.txt');
param=dict(string,string);
header=dict(string,string);
param['name']='zmx';
param['id']='111';
header['groupName']='dolphindb';
header['groupId']='11';
//Please set up your own httpServer ex.(python -m SimpleHTTPServer 8900)
url = "localhost:8900";
res = httpClient::httpGet(url,param,1000,header);
```
### httpClient::httpPost(url,[params],[timeout],[headers])

发送HTTP POST请求。

参数
* url：为请求的URL字符串。
* params：为一个字符串或一个key是string的字典。http协议Post方法请求的会把参数放在http请求正文中。
1. 如果params为一个字符串("example"),则发出的完整http报文的请求正文l为"example"。
2. 如果params为一个字典(两个键值对"name"->"zmx"和"id"->"111"),则发出的完整http报文的请求正文为 "id=111&name=zmx"。
* timeout：为超时时间，单位为毫秒。
* headers：为一个字符串或一个键和值都是string的字典,填写http请求头部。 如果headers为一个字典(两个键值对"groupName"->"dolphindb"和"groupId"->"11"),
则发出的完整http报文添加请求头"groupId:11"和"groupName:dolphindb"。如果只是一个字符串，则必须是"xx:xx"格式，会添加一个http请求头。

返回一个dictionary，键包括：
- `responseCode`: 请求返回的响应码。
- `headers`: 请求返回的头部。
- `text`: 请求返回的内容文本。
- `elapsed`: 请求经过的时间。
例子
```
loadPlugin('/home/zmx/worker/DolphinDBPlugin/httpClient/PluginHttpClient.txt');
param=dict(string,string);
header=dict(string,string);
param['name']='zmx';
param['id']='111';
header['groupName']='dolphindb';
header['groupId']='11';
//Please set up your own httpServer ex.(python -m SimpleHTTPServer 8900)
url = "localhost:8900";
res = httpClient::httpPost(url,param,1000,header);
```
### httpClient::sendEmail(userId,pwd,recipient,subject,body)
发送邮件。
- 使用本插件的邮件发送函数通常需要在邮件服务商上面设置开启smtp协议，还有需要获取邮箱授权码，参数pwd为邮箱授权码的字符串。
- 如果成功发送邮件，返回的字典res，res['responseCode']==250。

参数
* userId：发送者邮箱账号。
* pwd：发送者邮箱密码(授权码)。
* recipient：目标邮箱账号的一个字符串或一个字符串集合。
* subject：邮件主题的字符串。
* body：为邮件正文的字符串。

返回一个dictionary，键包括：
- `userId`: 发送者邮箱字符串。
- `recipient`: 接受者邮箱的集合的字符串。
- `responseCode`: 请求返回的响应码。
- `headers`: 请求返回的头部。
- `text`: 请求返回的内容文本。
- `elapsed`: 请求经过的时间。

例子
```
res=httpClient::sendEmail('MailFrom@xxx.com','xxxxx','Maildestination@xxx.com','This is a subject','It is a text');
```
```
emailTo='Maildestination@xxx.com''Maildestination2@xxx.com''Maildestination3@xxx.com';
res=httpClient::sendEmail('MailFrom@xxx.com','xxxxx',emailTo,'This is a subject','It is a text');
```
<<<<<<< HEAD
=======
db = database("non_seq_on_disk", RANGE, 0 500 1000)
hdf5::loadHdf5Ex(db,`tb,`col_4,"/smpl_numeric.h5","sint")
```

* Non-SEQ in-memory partitioned table
```
db = database("", RANGE, 0 500 1000)
t0 = hdf5::loadHDF5Ex(db,`tb,`col_4,"/smpl_numeric.h5","sint")
```

* Specify parameter transform to transform dafault type(e.g. 20200101) to specific type(e.g DATE)
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

## hdf5::HDF5DS

### Syntax

* `hdf5::HDF5DS(fileName,datasetName,[schema],[dsNum])`

### Parameter
* `fileName`: a HDF5 file name of type `string`.
* `datasetName`: the dataset name, i.e., the table name of type `string`. It can be obtained by using `ls` or `lsTable`.
* `schema`: a table with column names and data types of columns. If there is a need to change the data type of a column that is automatically determined by the system,  the schema table needs to be modified and used as an argument in `hdf5DS`.
* `dsNum`: the number of data sources to be generated. `HDF5DS` will divide the whole table equally into `dsNum` tables. If not specified, it will generate one data source.

### Details
* Generate a tuple of data sources according to the input file name and dataset name.

### Example
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

## hdf5::saveHDF5

### Syntax

- hdf5::saveHDF5(table, fileName, datasetName, [append], [stringMaxLength])

### Parameter

* `table`: The table will be saved.
* `fileName`: a HDF5 file name of type `string`.
* `datasetName`: the dataset name, i.e., the table name of type `string`. It can be obtained by using `ls` or `lsTable`.
* `append`: If append data to an existed table or not. Logical type. Default value is false.
* `stringMaxLength`: Maximum length of string. Default is 16. Only effect string and symbol type in table.

### Details

Save table in DolphinDB to certain file.

### Example

```
hdf5::saveHDF5(tb, "example.h5", "dataset name in hdf5")
```

# Data Types

The floating point and integer types in the file are first converted to H5T_NATIVE_* type (via H5Tget_native_type)

## integer
| type in HDF5      | default value in HDF5 | corresponding c type        | corresponding dolphindb type |
| ----------------- | --------------------- | :-------------------------- | :--------------------------- |
| H5T_NATIVE_CHAR   | ‘\0’                  | signed char / unsigned char | char/short                   |
| H5T_NATIVE_SCHAR  | ‘\0’                  | signed char                 | char                         |
| H5T_NATIVE_UCHAR  | ‘\0’                  | unsigned char               | short                        |
| H5T_NATIVE_SHORT  | 0                     | short                       | short                        |
| H5T_NATIVE_USHORT | 0                     | unsigned short              | int                          |
| H5T_NATIVE_INT    | 0                     | int                         | int                          |
| H5T_NATIVE_UINT   | 0                     | unsigned int                | long                         |
| H5T_NATIVE_LONG   | 0                     | long                        | int/long                     |
| H5T_NATIVE_ULONG  | 0                     | unsigned long               | unsupported/long             |
| H5T_NATIVE_LLONG  | 0                     | long long                   | long                         |
| H5T_NATIVE_ULLONG | 0                     | unsigned long long          | unsupported                  |


* The numeric types in dolphindb are all signed types. To prevent overflow, all unsigned types are converted to ```high-order signed types, 64-bit unsigned types are not supported ```
* H5T_NATIVE_CHAR corresponds to the char type in c, and char has symbolic dependencies and compilers and platforms. If there is a symbol, it is converted to char in dolphindb, otherwise it is converted to a short in DolphinDB.
* H5T_NATIVE_LONG and H5T_NATIVE_ULONG correspond to the `long` type in c. The size of the `long` depends on the compiler and platform. If the size of the `long` is the same as the `int`, it is the same as the `int` in the conversion process. If the length is the same as that of the `long long`, it will be converted to `long long`.
* All integer types can be converted to the numeric type ```(bool,char,short,int,long,float,double)``` in dolphindb.  **overflow** may occur. e.g. the maximum value of an int will be returned when converting LONG to INT.

## float
| type in HDF5      | default value in HDF5 | corresponding c type | corresponding dolphindb type |
| ----------------- | --------------------- | :------------------- | :--------------------------- |
| H5T_NATIVE_FLOAT  | +0.0f                 | float                | float                        |
| H5T_NATIVE_DOUBLE | +0.0                  | double               | double                       |

Note: IEEE754 floating point types are all signed numbers.

* All floating points types can be converted to the numeric type ```(bool,char,short,int,long,float,double)``` in dolphindb. **overflow** may occur. e.g. the maximum value of an `float` will be returned when converting DOUBLE to FLOAT.

## time
| type in HDF5   | default type in HDF5    | corresponding c type | corresponding dolphindb type |
| -------------- | ----------------------- | :------------------- | :--------------------------- |
| H5T_UNIX_D32BE | 1970.01.01T00:00:00     | 4 bytes integer      | DT_TIMESTAMP                 |
| H5T_UNIX_D32LE | 1970.01.01T00:00:00     | 4 bytes integer      | DT_TIMESTAMP                 |
| H5T_UNIX_D64BE | 1970.01.01T00:00:00.000 | 8 bytes integer      | DT_TIMESTAMP                 |
| H5T_UNIX_D64LE | 1970.01.01T00:00:00.000 | 8 bytes integer      | DT_TIMESTAMP                 |

* The predefined time type of HDF5 is **posix time**, 32-bit or 64-bit. The time type of HDF5 lacks the official definition. In this plug-in, the 32-bit time type ** represents the number of seconds from 1970**, The 64-bit ** is accurate to the millisecond**. All time types are uniformly converted by the plugin into a 64-bit integer and then converted to the `timestamp` type in dolphindb

* All data types above can be converted to time-related types in dolphindb`(date,month,time,minute,second,datetime,timestamp,nanotime,nanotimestamp)`

## string
| type in HDF5 | default value in HDF5 | corresponding c type | corresponding dolphindb type |
| ------------ | --------------------- | :------------------- | :--------------------------- |
| H5T_C_S1     | “”                    | char*                | DT_STRING                    |

* H5T_C_S1,包括```fixed-length```string和```variable-length```string

* `string` type can be converted to a string-related type converted to dolphindb```(string,symbol)```

## enum
| type in HDF5 | corresponding c type | corresponding dolphindb type |
| ------------ | :------------------- | :--------------------------- |
| ENUM         | enum                 | DT_SYMBOL                    |

* Enum type will be converted to a symbol variable in dolphindb. It is worth noting that the enumeration value and size relationship ** of each string will not be saved**. For example, if an enum variable is defined as HDF5_ENUM{"a"=100,"b"=2000,"c"=30000}, it will be converted to SYMBOL{"a"=3,"b"=1"c"=2}.


## compound and array
| type in HDF5 | corresponding c type | corresponding dolphindb type |
| ------------ | :------------------- | :--------------------------- |
| H5T_COMPOUND | struct               | \                            |
| H5T_ARRAY    | array                | \                            |

* Compound types and array types, as long as these complex types do not contain unsupported types, nested parsing will be supported.

* Complex type conversion depends on its internal sub data type.



# Table Struct

## Simple datatype table struct
For simple data types, the table imported into dolphindb from  HDF5 file will keep ``` the same ``

### Simple data types in HDF5

|     | 1       | 2       |
| --- | :------ | :------ |
| 1   | int(10) | int(67) |
| 2   | int(20) | int(76) |

### Converted data types in DolphinDB

|     | col_1 | col_2 |
| --- | :---- | :---- |
| 1   | 10    | 67    |
| 2   | 20    | 76    |

## complex datatype table struct

For complex data types, the data types in DolphinDB table depends on the structure of complex types

###  Table of compound type in HDF5

|     | 1                        | 2                        |
| --- | :----------------------- | :----------------------- |
| 1   | struct{a:1 b:2 c:3.7}    | struct{a:12 b:22 c:32.7} |
| 2   | struct{a:11 b:21 c:31.7} | struct{a:13 b:23 c:33.7} |

###  The corresponding table in DolphinDB after converting
|     | a    | b    | c    |
| --- | :--- | :--- | :--- |
| 1   | 1    | 2    | 3.7  |
| 2   | 11   | 21   | 31.7 |
| 3   | 12   | 22   | 32.7 |
| 4   | 13   | 23   | 33.7 |

### Table of array in HDF5

|     | 1             | 2               |
| --- | :------------ | :-------------- |
| 1   | array(1,2,3)  | array(4,5,6)    |
| 2   | array(8,9,10) | array(15,16,17) |

### The correspoinding table in DolphinDB after converting

|     | array_1 | array_2 | array_3 |
| --- | :------ | :------ | :------ |
| 1   | 1       | 2       | 3       |
| 2   | 4       | 5       | 6       |
| 3   | 8       | 9       | 10      |
| 4   | 15      | 16      | 17      |

For table of nested complex data types in HDF5, we use prefix ```A```  to represent this as an array and  ```C``` as a compound type.

###  Table of nested compound types in HDF5
|     | 1                                                         | 2                                                          |
| --- | :-------------------------------------------------------- | :--------------------------------------------------------- |
| 1   | struct{a:array(1,2,3)<br>  b:2<br>  c:struct{d:"abc"}}    | struct{a:array(7,8,9)<br>  b:5<br>  c:struct{d:"def"}}     |
| 2   | struct{a:array(11,21,31)<br>  b:0<br>  c:struct{d:"opq"}} | struct{a:array(51,52,53)<br>  b:24<br>  c:struct{d:"hjk"}} |


### The corresponding table in DolphinDB after importing

|     | Aa_1 | Aa_2 | Aa_3 | b    | Cc_d |
| --- | :--- | :--- | :--- | :--- | :--- |
| 1   | 1    | 2    | 3    | 2    | abc  |
| 2   | 7    | 8    | 9    | 5    | def  |
| 3   | 11   | 21   | 31   | 0    | opq  |
| 4   | 51   | 52   | 53   | 24   | hjk  |


# Performance

## Environment

* cpu: i7-7700 3.60GHZ
* ssd: read 460~500MB/S
>>>>>>> c64b2c7... update hdf5


