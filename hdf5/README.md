# DolphinDB HDF5 Plugin

DolphinDB's HDF5 plugin imports HDF5 datasets into DolphinDB and supports data type conversions.


* [Build](#Build)  
    * [Build with cmake](#Build with cmake)
    * [Build with makefile](#Build with makefile)
* [User-API](#User API)  
    * [hdf5::ls](#hdf5ls)
    * [hdf5::lsTable](#hdf5lstable)
    * [hdf5::extractHdf5Schema](#hdf5extracthdf5schema)
    * [hdf5::loadHdf5](#hdf5loadhdf5)
    * [hdf5::loadHdf5Ex](#hdf5loadhdf5ex)
    * [hdf5::hdf5DS](#hdf5hdf5ds)
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
```
sudo apt-get install cmake
```

Install HDF5 development kit
```
sudo apt-get install libhdf5-dev
```
Build the entire project

```
mkdir build
cd build
cmake ../path_to_hdf5_plugin/
make
```

### Build with makefile

Install HDF5 development kit

```
sudo apt-get install libhdf5-dev
```
Execute "make" command

```
make
```

**Remember:** Before compiling, please make sure that `libDolphinDB.so` is in the gcc searchable path, you can use `LD_LIBRARY_PATH` to specify its path.

The libPluginHdf5.so file will be generated after the compilation.

## For Windows users

The `enable threadsafe` configuration must be used for Windows. It should be set when compiling HDF5.

You may use `hdf5.dll` file that have been prebuilt.

You can also build HDF5 yourself. Download the source code from the [official website of HDF5](https://www.hdfgroup.org/solutions/hdf5/), and follow the instructions to build. If you are to build HDF5 with configure, the `--enable-threadsafe`, `--disable-cxx` and `--enable-shared` options must be used. If you are to build HDF5 with CMake, the `-DHDF5_ENABLE_THREADSAFE:BOOL=ON`, `-DHDF5_BUILD_CPP_LIB:BOOL=OFF`, `-DBUILD_SHARED_LIBS:BOOL=ON` must be added when compiling.

# User-API


**Remember:** Use `loadPlugin("/path_to_file_hdf5.cfg/hdf5.cfg")` to import HDF5 plugin before using the API.

## hdf5::ls

### Syntax

* `hdf5::ls(filename)`

### Parameters  

* `filename`: a HDF5 file name of type `string`.

### Details

* List all the `HDF5 objects in a table including `dataset` and `group` and `(named type) namedType`. For the dataset, we will return the size (column size first followed by row size). For example, DataSet{(7,3)} represents 7 columns and 3 rows.

### Example   
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
## hdf5::lsTable

### Syntax

* `hdf5::lsTable(filename)`

### Parameters  

* `filename`: a HDF5 file name of type `string`.

### Details
* List all the table information in a table, that is, HDF5 `dataset` information, including table name, size, and type.

### Example   
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

## hdf5::extractHdf5Schema

### Syntax

* `hdf5::extractHdf5Schema(fileName,datasetName)`

### Parameters
* `fileName`: a HDF5 file name of type `string`.
* `datasetName`: the dataset name, i.e., the table name of type `string`. It can be obtained by using `ls` or `lsTable`.

### Details
* Generate the schema table for the input data file. The schema table has 2 columns: column names and their data types.

### Example
```
hdf5::extractHdf5Schema("/smpl_numeric.h5","sint")

output:
        name	type
        col_0	INT
        col_1	INT
        col_2	INT
        col_3	INT
        col_4	INT
        col_5	INT
        col_6	INT


hdf5::extractHdf5Schema("/compound.h5","com")

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

## hdf5::loadHdf5

### Syntax

* `hdf5::loadHdf5(fileName,datasetName,[schema],[startRow],[rowNum])`

### Parameters
* `fileName`: a HDF5 file name of type `string`.
* `datasetName`: the dataset name, i.e., the table name of type `string`. It can be obtained by using `ls` or `lsTable`.
* `schema`: a table with column names and data types of columns. If there is a need to change the data type of a column that is automatically determined by the system,  the schema table needs to be modified and used as an argument in `loadHdf5`.
* `startRow`: an integer indicating the start row to read. If not specified, the dataset will be read from the beginning.
* `rowNum`: an integer indicating the number of rows to read. If not specified, `loadHdf5` will read until the end of data.

### Details
* Load an HDF5 file into a DolphinDB in-memory table.
* The number of rows to read is defined in the HDF5 file, rather than the number of rows in the output DolphinDB table.
* Supported data types, as well as data conversion rules are visible in the [Data Types] (#Data Types) section.

### Example
```
hdf5::loadHdf5("/smpl_numeric.h5","sint")

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

## hdf5::loadHdf5Ex

### Syntax

* `hdf5::loadHdf5Ex(dbHandle,tableName,[partitionColumns],fileName,datasetName,[schema],[startRow],[rowNum])`

### Parameters
* `dbHandle`和`tableName`: If the input data is to be saved into the distributed database, the database handle and table name should be specified.
* `partitionColumns`: a string scalar/vector indicating partitioning column(s).
* `fileName`: a HDF5 file name of type `string`.
* `datasetName`: the dataset name, i.e., the table name of type `string`. It can be obtained by using `ls` or `lsTable`.
* `schema`: a table with column names and data types of columns. If there is a need to change the data type of a column that is automatically determined by the system,  the schema table needs to be modified and used as an argument in `loadHdf5Ex`.
* `startRow`: an integer indicating the start row to read. If not specified, the dataset will be read from the beginning.
* `rowNum`: an integer indicating the number of rows to read. If not specified, `loadHdf5` will read until the end of data.

### Details
* Load an HDF5 file into a distributed table in a distributed database. The result is a table object with the loaded metadata.
* The number of rows to read is defined in the HDF5 file, rather than the number of rows in the output DolphinDB table.
* Supported data types, as well as data conversion rules are visible in the [Data Types] (#Data Types) section.

### Example
* SEQ partitioned table on disk
```
db = database("seq_on_disk", SEQ, 16)
hdf5::loadHdf5Ex(db,`tb,,"/large_file.h5", "large_table")
```

* SEQ in-memory partitioned table
```
db = database("", SEQ, 16)
hdf5::loadHdf5Ex(db,`tb,,"/large_file.h5", "large_table")
```

* Non-SEQ partitioned table on disk
```
db = database("non_seq_on_disk", RANGE, 0 500 1000)
hdf5::loadHdf5Ex(db,`tb,`col_4,"/smpl_numeric.h5","sint")
```

* Non-SEQ in-memory partitioned table
```
db = database("", RANGE, 0 500 1000)
t0 = hdf5::loadHdf5Ex(db,`tb,`col_4,"/smpl_numeric.h5","sint")
```

## hdf5::hdf5DS

### Syntax

* `hdf5::hdf5DS(fileName,datasetName,[schema],[dsNum])`

### Parameter
* `fileName`: a HDF5 file name of type `string`.
* `datasetName`: the dataset name, i.e., the table name of type `string`. It can be obtained by using `ls` or `lsTable`.
* `schema`: a table with column names and data types of columns. If there is a need to change the data type of a column that is automatically determined by the system,  the schema table needs to be modified and used as an argument in `hdf5DS`.
* `dsNum`: the number of data sources to be generated. `hdf5DS` will divide the whole table equally into `dsNum` tables. If not specified, it will generate one data source.

### Details
* Generate a tuple of data sources according to the input file name and dataset name.

### Example
```
>ds = hdf5::hdf5DS(smpl_numeric.h5","sint")

>size ds;
1

>ds[0];
DataSource< loadHDF5("/smpl_numeric.h5", "sint", , 0, 3) >

>ds = hdf5::hdf5DS(smpl_numeric.h5","sint",,3)

>size ds;
3

>ds[0];
DataSource< loadHDF5("/smpl_numeric.h5", "sint", , 0, 1) >

>ds[1];
DataSource< loadHDF5("/smpl_numeric.h5", "sint", , 1, 1) >

>ds[2];
DataSource< loadHDF5("/smpl_numeric.h5", "sint", , 2, 1) >
```

# Data Types

The floating point and integer types in the file are first converted to H5T_NATIVE_* type (via H5Tget_native_type)

## integer
| type in HDF5      | corresponding c type        | corresponding dolphindb type |
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


* The numeric types in dolphindb are all signed types. To prevent overflow, all unsigned types are converted to ```high-order signed types, 64-bit unsigned types are not supported ```
* H5T_NATIVE_CHAR corresponds to the char type in c, and char has symbolic dependencies and compilers and platforms. If there is a symbol, it is converted to char in dolphindb, otherwise it is converted to a short in DolphinDB.
* H5T_NATIVE_LONG and H5T_NATIVE_ULONG correspond to the `long` type in c. The size of the `long` depends on the compiler and platform. If the size of the `long` is the same as the `int`, it is the same as the `int` in the conversion process. If the length is the same as that of the `long long`, it will be converted to `long long`.
* All integer types can be converted to the numeric type ```(bool,char,short,int,long,float,double)``` in dolphindb.  **overflow** may occur. e.g. the maximum value of an int will be returned when converting LONG to INT.

## float
| type in HDF5      | corresponding c type | corresponding dolphindb type |
| ----------------- | :------------------- | :--------------------------- |
| H5T_NATIVE_FLOAT  | float                | float                        |
| H5T_NATIVE_DOUBLE | double               | double                       |

Note: IEEE754 floating point types are all signed numbers.

* All floating points types can be converted to the numeric type ```(bool,char,short,int,long,float,double)``` in dolphindb. **overflow** may occur. e.g. the maximum value of an `float` will be returned when converting DOUBLE to FLOAT.

## time
| type in HDF5   | corresponding c type | corresponding dolphindb type |
| -------------- | :------------------- | :--------------------------- |
| H5T_UNIX_D32BE | 4 bytes integer      | DT_TIMESTAMP                 |
| H5T_UNIX_D32LE | 4 bytes integer      | DT_TIMESTAMP                 |
| H5T_UNIX_D64BE | 8 bytes integer      | DT_TIMESTAMP                 |
| H5T_UNIX_D64LE | 8 bytes integer      | DT_TIMESTAMP                 |

* The predefined time type of HDF5 is **posix time**, 32-bit or 64-bit. The time type of HDF5 lacks the official definition. In this plug-in, the 32-bit time type ** represents the number of seconds from 1970**, The 64-bit ** is accurate to the millisecond**. All time types are uniformly converted by the plugin into a 64-bit integer and then converted to the `timestamp` type in dolphindb

* All data types above can be converted to time-related types in dolphindb`(date,month,time,minute,second,datetime,timestamp,nanotime,nanotimestamp)`

## string
| type in HDF5 | corresponding c type | corresponding dolphindb type |
| ------------ | :------------------- | :--------------------------- |
| H5T_C_S1     | char*                | DT_STRING                    |

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

## Data import performance

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
