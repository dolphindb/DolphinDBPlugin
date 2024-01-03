# DolphinDB Arrow Plugin

[Apache Arrow](https://arrow.apache.org/) defines a columnar memory format, which combines the benefits of columnar data structures with in-memory computing. With the DolphinDB Arrow plugin, you can use the Arrow format to interact with the DolphinDB server through Python API with automatic data type conversion.

**Note: Starting from 2.00.11, the plugin name has been changed from "formatArrow" to "Arrow".**

The version of Apache Arrow used in this document is *9.0.0*.

For the DolphinDB Arrow plugin, please make sure you use DolphinDB server version 2.00.11 or higher. 

## 1. Install Plugin

### 1.1 (Optional) Manually Compile Plugin - Linux

#### Configure the Environment

a. Compile the plugin development kit

```
git clone https://github.com/apache/arrow.git
cd arrow/cpp
mkdir build
cd build
cmake .. 
make -j
```

b. After compilation, save the following files to your DolphinDB plugin directory:

| **File**                                | **Target Directory** |
| :-------------------------------------- | :------------------- |
| arrow/cpp/src/arrow                     | ./include            |
| arrow/cpp/build/release/libarrow.so.900 | ./build              |

#### Compile Plugin

Before compilation, make sure the directories of *libDolphinDB.so* (included in the server package) and *libarrow.so.900* have been added to the GCC search path. You can set *LD_LIBRARY_PATH* or directly save the files under the *build* directory.

```
cd /path/to/plugins/Arrow
mkdir build
cd build
cmake ..
make
```

### 1.2 Load Plugin

Alternatively, you can skip the manual compilation steps and simply download the precompile file *libarrow.so.900*. Save the files under your DolphinDB plugin directory. 

To load the plugin, enter the following command in DolphinDB:

```
loadFormatPlugin("/path/to/plugin/PluginArrow.txt")
```

## 2. Usage Example

a. Enter the following command on the DolphinDB server side to load the plugin:

```
loadFormatPlugin("path/to/Arrow/PluginArrow.txt")
```

b. On the Python API side, execute the following script:

```
import dolphindb as ddb
import dolphindb.settings as keys
s = ddb.session("192.168.1.113", 8848, "admin", "123456", protocol=keys.PROTOCOL_ARROW)
pat = s.run("table(1..10 as a)")

print(pat)
-------------------------------------------
pyarrow.Table
a: int32
----
a: [[1,2,3,4,5,6,7,8,9,10]]
```

Note: Currently, the DolphinDB server doesn't support enabling compression when the Arrow protocol is used.

## 3. Supported Data Types

### DolphinDB to Arrow 

When DolphinDB transfers data to the Python application through Python API, the data type mappings between DolphinDB and Arrow are as follows:

| DolphinDB     | Arrow                   |
| :------------ | :---------------------- |
| BOOL          | boolean                 |
| CHAR          | int8                    |
| SHORT         | int16                   |
| INT           | int32                   |
| LONG          | int64                   |
| DATE          | date32                  |
| MONTH         | date32                  |
| TIME          | time32(ms)              |
| MINUTE        | time32(s)               |
| SECOND        | time32(s)               |
| DATETIME      | timestamp(s)            |
| TIMESTAMP     | timestamp(ms)           |
| NANOTIME      | time64(ns)              |
| NANOTIMESTAMP | timestamp(ns)           |
| DATEHOUR      | timestamp(s)            |
| FLOAT         | float32                 |
| DOUBLE        | float64                 |
| SYMBOL        | dictionary(int32, utf8) |
| STRING        | utf8                    |
| IPADDR        | utf8                    |
| UUID          | fixed_size_binary(16)   |
| INT128        | fixed_size_binary(16)   |
| BLOB          | large_binary            |
| DECIMAL32(X)  | decimal128(38, X)       |
| DECIMAL64(X)  | decimal128(38, X)       |


Note: 
- Array vectors of the types listed above (excluding the Decimal types) are also supported. 
- When converting Arrow-formatted data sent from the DolphinDB server to a pandas.DataFrame, the DolphinDB *NANOTIME*  type is converted to Arrow *time64* type. The NANOTIME value must be a multiple of 1,000, otherwise the error `Value xxxxxxx has non-zero nanoseconds` is raised.
- Starting from version 2.00.11, the byte order of downloaded UUID/INT128 data matches the upload order, instead of reversing it.