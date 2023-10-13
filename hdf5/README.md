# DolphinDB HDF5 Plugin

DolphinDB's HDF5 plugin imports HDF5 datasets into DolphinDB and supports data type conversions.

- [DolphinDB HDF5 Plugin](#dolphindb-hdf5-plugin)
- [1. Install the Plugin](#1-install-the-plugin)
  - [1.1 Import Precompiled Binaries](#11-import-precompiled-binaries)
  - [1.2 (Optional) Compiling](#12-optional-compiling)
- [2. Methods](#2-methods)
  - [2.1 hdf5::ls](#21-hdf5ls)
  - [2.2 hdf5::lsTable](#22-hdf5lstable)
  - [2.3 hdf5::extractHDF5Schema](#23-hdf5extracthdf5schema)
  - [2.4 hdf5::loadHDF5](#24-hdf5loadhdf5)
  - [2.5 hdf5::loadPandasHDF5](#25-hdf5loadpandashdf5)
  - [2.6 hdf5::loadHDF5Ex](#26-hdf5loadhdf5ex)
  - [2.7 hdf5::HDF5DS](#27-hdf5hdf5ds)
  - [2.8 hdf5::saveHDF5](#28-hdf5savehdf5)
- [3. Data Types](#3-data-types)
  - [3.1 integer](#31-integer)
  - [3.2 float](#32-float)
  - [3.3 time](#33-time)
  - [3.4 string](#34-string)
  - [3.5 enum](#35-enum)
  - [3.6 compound and array](#36-compound-and-array)
- [4. Table Struct](#4-table-struct)
  - [4.1 Simple datatype table struct](#41-simple-datatype-table-struct)
  - [4.2 complex datatype table struct](#42-complex-datatype-table-struct)
- [5. Performance](#5-performance)
  - [5.1 Environment](#51-environment)
  - [5.2 Data import performance](#52-data-import-performance)


# 1. Install the Plugin

## 1.1 Import Precompiled Binaries

You can import the precompiled HDF5 plugin in the DolphinDB installation package or in the bin directory.

**Linux**

(1) Add the path where the plugin is located to the LIB search path LD_LIBRARY_PATH

```
export LD_LIBRARY_PATH=/path_to_hdf5_plugin/:$LD_LIBRARY_PATH
```

(2) Start the DolphinDB server and import the plugin.

```
loadPlugin("/path_to_hdf5_plugin/PluginHdf5.txt")
```

**Windows**

Load plugin with an absolute path and replace "\\" with "\\\\" or "/".

```
loadPlugin("/path_to_hdf5_plugin/PluginHdf5.txt")
```

## 1.2 (Optional) Compiling

After compilation, you can import the plugin in the same way as described in section 1.1 Install Precompiled Plugin.

**Compile on Linux**

Install CMake

```
sudo apt install cmake
```

Install c-blosc

```
#download source code at https://github.com/Blosc/c-blosc/releases/tag/v1.21.1
cd c-blosc-1.21.1
mkdir build
cd build
cmake -DCMAKE_C_FLAGS="-fPIC -std=c11" ..
make -j
cp blosc/src/blosc.h /path_to_hdf5_plugin/include/c-blosc
cp blosc/src/blosc-export.h /path_to_hdf5_plugin/include/c-blosc
cp blosc/libblosc.a /path_to_hdf5_plugin/lib
```

Install HDF5 development kit

```
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

```
mkdir build
cd build
cp /path_to_dolphindb/libDolphinDB.so ./
cmake ..
make
```

**Compile on Windows**

Use MinGW to compile hdf5 1.13.1.zip in the MSYS2 environment.

* Use ``Mingw-w64-x86_64``

* Install make

```
pacman -S make
```

Open the MSYS2 terminal and navigate into the directory where hdf5-1.13.1 is decompressed

```
 CFLAGS="-std=c11" CXXFLAGS="-std=c++11" ./configure --host=x86_64-w64-mingw32 --build=x86_64-w64-mingw32 --prefix=/d/hdf5_1.13.1 --enable-cxx --enable-tests=no --enable-tools=no with_pthread=no
```

Open src/H5pubconf.h and add the following #define in the end of it

```
#ifndef H5_HAVE_WIN32_API
#define H5_HAVE_WIN32_API 1
#endif

#ifndef H5_HAVE_MINGW
#define H5_HAVE_MINGW 1
#endif
```

Start compiling

```
make -j8
+ make install -j8
```

Copy the compiled file to the HDF5 plugin directory

```
cp $HOME/hdf5/include/* /path_to_hdf5_plugin/include_win/hdf5
cp $HOME/hdf5/lib/libhdf5.a /path_to_hdf5_plugin/build
cp $HOME/hdf5/lib/libhdf5_cpp.a /path_to_hdf5_plugin/build
cp $HOME/hdf5/lib/libhdf5_hl.a /path_to_hdf5_plugin/build
cp $HOME/hdf5/lib/libhdf5_hl_cpp.a /path_to_hdf5_plugin/build
```

Open Command Prompt
Build c_blosc

```
cd c_blosc-1.21.1
mkdir build
cd build
cmake  ../ -G "MinGW Makefiles"
mingw32-make -j8
copy ./blosc/libblosc.a /path_to_hdf5_plugin/build/
```

Copy libDolphinDB.dll to HDF5 plugin directory

```
copy /path_to_dolphindb/libDolphinDB.dll /path_to_hdf5_plugin/build
```

Compile HDF5 plugin

```
cmake  ../ -G "MinGW Makefiles"
mingw32-make -j
```


# 2. Methods

## 2.1 hdf5::ls

**Syntax**

hdf5::ls(fileName)

**Parameters**  

* fileName: a string indicating the HDF5 file name.

**Details**

List all the HDF5 objects in a table (including dataset and group) and object type. For the dataset, we will return the size (column size first followed by row size). For example, DataSet{(7,3)} represents 7 columns and 3 rows.

**Example**

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

output
        objName      objType
        ----------------------
        /            Group      
        /type_name   NamedDataType


```

## 2.2 hdf5::lsTable

**Syntax**

hdf5::lsTable(fileName)

**Parameters**   

* fileName: a string indicating the HDF5 file name.

**Details**

List all the table information in a table, i.e., HDF5 `dataset` information, including table name, size, and type.

**Example**

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

## 2.3 hdf5::extractHDF5Schema

**Syntax**

hdf5::extractHDF5Schema(fileName, datasetName)

**Parameters** 

* fileName: a string indicating the HDF5 file name.

* datasetName: the dataset name, i.e., the table name of type string. It can be obtained by using `ls` or `lsTable`.

**Details**

Generate the schema table for the input data file. The schema table has 2 columns: column names and their data types.

**Example**

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

## 2.4 hdf5::loadHDF5

**Syntax**

hdf5::loadHDF5(fileName,datasetName,[schema],[startRow],[rowNum])

**Parameters** 

* fileName: a string indicating the HDF5 file name.
* datasetName: the dataset name, i.e., the table name of type string. It can be obtained by using `ls` or `lsTable`.
* schema: a table with column names and data types of columns. If there is a need to change the data type of a column that is automatically determined by the system,  the schema table needs to be modified and used as an argument in `loadHdf5`.
* startRow: an integer indicating the start row to read. If not specified, the dataset will be read from the beginning.
* rowNum: an integer indicating the number of rows to read. If not specified, `loadHdf5` will read until the end of data.

**Details**

* Load an HDF5 file into a DolphinDB in-memory table.
* The number of rows to read is defined in the HDF5 file, rather than the number of rows in the output DolphinDB table.
* Supported data types, as well as data conversion rules are visible in the [Data Types](#Data Types) section.

**Example**

```
hdf5::loadHDF5("/smpl_numeric.h5","sint")

output:
        col_0	col_1	col_2	col_3	col_4	col_5	col_6
        (758)	8	(325,847)	87	687	45	90
        61	0	28	77	546	789	45
        799	5,444	325,847	678	90	54	0


scm = table(`a`b`c`d`e`f`g as name, `CHAR`BOOL`SHORT`INT`LONG`DOUBLE`FLOAT as type)
hdf5::loadHdf5("../hdf5/h5file/smpl_numeric.h5","sint",scm,1,1)

output:
        a	b	c	d	e	f	g
        '='	false	28	77	546	789	45

```

> **Note: the dimension of the dataset must be less than or equal to 2, only 2D or 1D tables can be parsed**

## 2.5 hdf5::loadPandasHDF5

**Syntax**

hdf5::loadPandasHDF5(fileName,groupName,[schema],[startRow],[rowNum])

**Parameters**

* fileName: a STRING scalar indicating the HDF5 file name saved by Pandas.
* groupName: the identifier of the group, i.e. the key name.
* schema: a table with column names and data types of columns. If there is a need to change the data type of a column that is automatically determined by the system, the schema table needs to be modified and used as an argument in `loadPandasHDF5`.
* startRow: an integer indicating the start row to read. If not specified, the dataset will be read from the beginning.
* rowNum: an integer indicating the number of rows to read. If not specified, `loadPandasHDF5` will read until the end of data.

**Details**

Load an HDF5 file saved by Pandas into a DolphinDB in-memory table. The number of rows to read is defined in the HDF5 file, rather than the number of rows in the output DolphinDB table. Regarding data types and data type conversion, see [Data Types](#Data Types).

**Example**

```
hdf5::loadPandasHDF5("/home/ffliu/Data/data.h5","/s",,1,1)

output:
        A	 B	C  D  E
        28 77	54 78 9
```

## 2.6 hdf5::loadHDF5Ex

**Syntax**

hdf5::loadHDF5Ex(dbHandle,tableName,[partitionColumns],fileName,datasetName,[schema],[startRow],[rowNum],[transform])

**Parameters** 

* dbHandle and tableName: If the input data is to be saved into the distributed database, the database handle and table name should be specified.
* partitionColumns: a string scalar/vector indicating partitioning column(s).
* fileName: a string indicating the HDF5 file name.
* datasetName: the dataset name, i.e., the table name of type string. It can be obtained by using `ls` or `lsTable`.
* schema: a table with column names and data types of columns. If there is a need to change the data type of a column that is automatically determined by the system,  the schema table needs to be modified and used as an argument in `loadHdf5Ex`.
* startRow: an integer indicating the start row to read. If not specified, the dataset will be read from the beginning.
* rowNum: an integer indicating the number of rows to read. If not specified, `loadHdf5` will read until the end of data.
* transform: an unary function. The parameter of the function must be a table. If parameter transform is specified, we need to first execute createPartitionedTable and then load the data. The system will apply the function specified in parameter transform and then save the results into the database.

**Details**

Load an HDF5 file into a distributed table in a distributed database. The result is a table object with the loaded metadata. The number of rows to read is defined in the HDF5 file, rather than the number of rows in the output DolphinDB table. Regarding data types and data type conversion, see [Data Types](#Data Types).

**Example**

* SEQ partitioned table on disk
```
db = database("seq_on_disk", SEQ, 16)
hdf5::loadHDF5Ex(db,`tb,,"/large_file.h5", "large_table")
```

* SEQ in-memory partitioned table
```
db = database("", SEQ, 16)
hdf5::loadHDF5Ex(db,`tb,,"/large_file.h5", "large_table")
```

* Non-SEQ partitioned table on disk
```
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

## 2.7 hdf5::HDF5DS

**Syntax**

hdf5::HDF5DS(fileName,datasetName,[schema],[dsNum])

**Parameters** 
* fileName: a string indicating the HDF5 file name.
* datasetName: the dataset name, i.e., the table name of type string. It can be obtained by using `ls` or `lsTable`.
* schema: a table with column names and data types of columns. If there is a need to change the data type of a column that is automatically determined by the system,  the schema table needs to be modified and used as an argument in `hdf5DS`.
* dsNum: the number of data sources to be generated. `HDF5DS` will divide the whole table equally into `dsNum` tables. If not specified, it will generate one data source.

**Details**

Generate a tuple of data sources according to the input file name and dataset name.

**Example**

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
> Note: HDF5 does not support concurrent reads.

Wrong example:

```
ds = hdf5::HDF5DS("/smpl_numeric.h5", "sint", ,3)
res = mr(ds, def(x) : x)
```
Correct example (set parameter parallel of function mr to false) :

```
ds = hdf5::HDF5DS("/smpl_numeric.h5", "sint", ,3)
res = mr(ds, def(x) : x,,,false)
```

## 2.8 hdf5::saveHDF5

**Syntax**

hdf5::saveHDF5(table, fileName, datasetName, [append], [stringMaxLength])

**Parameters** 

* table: The table will be saved.
* fileName: a string indicating the HDF5 file name.
* datasetName: the dataset name, i.e., the table name of type string. It can be obtained by using `ls` or `lsTable`.
* append: If append data to an existed table or not. Logical type. Default value is false.
* stringMaxLength: Maximum length of string. Default is 16. Only effect string and symbol type in table.

**Details**

Save the table in DolphinDB to certain dataset in HDF5 file. Supported data types and data conversion rules are visible in the [Data Types](#Data Types) section.

**Example**

```
hdf5::saveHDF5(tb, "example.h5", "dataset name in hdf5")
```
> Note: Null values are not supported in the HDF5 file. If there are NULL values in DolphinDB tables, they will be saved as the default value defined in [Data Types](#Data Types) section. To read h5 files generated by the HDF5 plugin through python, you can use the h5py library. 

For example, 
```
import h5py f = h5py.File("/home/workDir/dolphindb_src/build/test.h5", 'r') print(f['aaa']['TimeStamp']) print(f['aaa']['StockID'])
```

# 3. Data Types

The floating point and integer types in the file are first converted to H5T_NATIVE_* type (via H5Tget_native_type)

## 3.1 integer

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

## 3.2 float

| type in HDF5      | default value in HDF5 | corresponding c type | corresponding dolphindb type |
| ----------------- | --------------------- | :------------------- | :--------------------------- |
| H5T_NATIVE_FLOAT  | +0.0f                 | float                | float                        |
| H5T_NATIVE_DOUBLE | +0.0                  | double               | double                       |

Note: IEEE754 floating point types are all signed numbers.

* All floating points types can be converted to the numeric type ```(bool,char,short,int,long,float,double)``` in dolphindb. **overflow** may occur. e.g. the maximum value of an `float` will be returned when converting DOUBLE to FLOAT.

## 3.3 time
| type in HDF5   | default type in HDF5    | corresponding c type | corresponding dolphindb type |
| -------------- | ----------------------- | :------------------- | :--------------------------- |
| H5T_UNIX_D32BE | 1970.01.01T00:00:00     | 4 bytes integer      | DT_TIMESTAMP                 |
| H5T_UNIX_D32LE | 1970.01.01T00:00:00     | 4 bytes integer      | DT_TIMESTAMP                 |
| H5T_UNIX_D64BE | 1970.01.01T00:00:00.000 | 8 bytes integer      | DT_TIMESTAMP                 |
| H5T_UNIX_D64LE | 1970.01.01T00:00:00.000 | 8 bytes integer      | DT_TIMESTAMP                 |

* The predefined time type of HDF5 is **posix time**, 32-bit or 64-bit. The time type of HDF5 lacks the official definition. In this plug-in, the 32-bit time type ** represents the number of seconds from 1970**, The 64-bit ** is accurate to the millisecond**. All time types are uniformly converted by the plugin into a 64-bit integer and then converted to the `timestamp` type in dolphindb

* All data types above can be converted to time-related types in dolphindb`(date,month,time,minute,second,datetime,timestamp,nanotime,nanotimestamp)`

## 3.4 string

| type in HDF5 | default value in HDF5 | corresponding c type | corresponding dolphindb type |
| ------------ | --------------------- | :------------------- | :--------------------------- |
| H5T_C_S1     | “”                    | char*                | DT_STRING                    |

* H5T_C_S1,includes ```fixed-length```string and```variable-length```string

* `string` type can be converted to a string-related type converted to dolphindb```(string,symbol)```

## 3.5 enum

| type in HDF5 | corresponding c type | corresponding dolphindb type |
| ------------ | :------------------- | :--------------------------- |
| ENUM         | enum                 | DT_SYMBOL                    |

* Enum type will be converted to a symbol variable in dolphindb. It is worth noting that the enumeration value and size relationship ** of each string will not be saved**. For example, if an enum variable is defined as HDF5_ENUM{"a"=100,"b"=2000,"c"=30000}, it will be converted to SYMBOL{"a"=3,"b"=1"c"=2}.
* Enum type can be converted to a string-related type converted to dolphindb(string,symbol).

## 3.6 compound and array

| type in HDF5 | corresponding c type | corresponding dolphindb type |
| ------------ | :------------------- | :--------------------------- |
| H5T_COMPOUND | struct               | \                            |
| H5T_ARRAY    | array                | \                            |

* Compound types and array types, as long as these complex types do not contain unsupported types, nested parsing will be supported.
* Complex type conversion depends on its internal sub data type.



# 4. Table Struct

## 4.1 Simple datatype table struct

For simple data types, the table imported into dolphindb from  HDF5 file will keep the same.

**Simple data types in HDF5**

|     | 1       | 2       |
| --- | :------ | :------ |
| 1   | int(10) | int(67) |
| 2   | int(20) | int(76) |

**Converted data types in DolphinDB**

|     | col_1 | col_2 |
| --- | :---- | :---- |
| 1   | 10    | 67    |
| 2   | 20    | 76    |

## 4.2 complex datatype table struct

For complex data types, the data types in DolphinDB table depends on the structure of complex types

**Table of compound type in HDF5**

|     | 1                        | 2                        |
| --- | :----------------------- | :----------------------- |
| 1   | struct{a:1 b:2 c:3.7}    | struct{a:12 b:22 c:32.7} |
| 2   | struct{a:11 b:21 c:31.7} | struct{a:13 b:23 c:33.7} |

**The corresponding table in DolphinDB after converting**
|     | a    | b    | c    |
| --- | :--- | :--- | :--- |
| 1   | 1    | 2    | 3.7  |
| 2   | 11   | 21   | 31.7 |
| 3   | 12   | 22   | 32.7 |
| 4   | 13   | 23   | 33.7 |

**Table of array in HDF5**

|     | 1             | 2               |
| --- | :------------ | :-------------- |
| 1   | array(1,2,3)  | array(4,5,6)    |
| 2   | array(8,9,10) | array(15,16,17) |

**The corresponding table in DolphinDB after converting**

|     | array_1 | array_2 | array_3 |
| --- | :------ | :------ | :------ |
| 1   | 1       | 2       | 3       |
| 2   | 4       | 5       | 6       |
| 3   | 8       | 9       | 10      |
| 4   | 15      | 16      | 17      |

**Table of nested compound types in HDF5**

For table of nested complex data types in HDF5, we use prefix ```A```  to represent this as an array and  ```C``` as a compound type.

|     | 1                                                         | 2                                                          |
| --- | :-------------------------------------------------------- | :--------------------------------------------------------- |
| 1   | struct{a:array(1,2,3)<br>  b:2<br>  c:struct{d:"abc"}}    | struct{a:array(7,8,9)<br>  b:5<br>  c:struct{d:"def"}}     |
| 2   | struct{a:array(11,21,31)<br>  b:0<br>  c:struct{d:"opq"}} | struct{a:array(51,52,53)<br>  b:24<br>  c:struct{d:"hjk"}} |


**The corresponding table in DolphinDB after importing**

|     | Aa_1 | Aa_2 | Aa_3 | b    | Cc_d |
| --- | :--- | :--- | :--- | :--- | :--- |
| 1   | 1    | 2    | 3    | 2    | abc  |
| 2   | 7    | 8    | 9    | 5    | def  |
| 3   | 11   | 21   | 31   | 0    | opq  |
| 4   | 51   | 52   | 53   | 24   | hjk  |


# 5. Performance

## 5.1 Environment

* cpu: i7-7700 3.60GHZ
* ssd: read 460~500MB/S

## 5.2 Data import performance

* int  
    * rows 1024 * 1024 * 16 
    * cols  64
    * fileSize 4G
    * time 8s
* usigned int
    * rows 1024 * 1024 * 16 
    * cols 64
    * fileSize 4G
    * time 9s
* variable-length string
    * rows 1024 * 1024 
    * cols 64
    * fileSize 3.6G
    * time 17s
* compound
    * sub types include 9 data types: str,str,double,int,long,float,int,short, and char
    * dataVolume: 1024 * 1024 * 62
    * fileSize 3.9G
    * time 10 s
* compound array
    * sub types include 72 data types: (str,str,double,int,long,float,int,short,char) * 8
    * dataVolume: 1024 * 128 * 62
    * fileSize 3.9G
    * time 15s
