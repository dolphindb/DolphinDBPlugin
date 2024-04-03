# DolphinDB kdb+ Plugin

With the DolphinDB kdb+ plugin, you can load the kdb+ tables on your disk into DolphinDB as in-memory tables. This plugin can be used to load all types of Q data structures in DolphinDB. There are two load options: through the `loadTable` method or the `loadFile` method.

The DolphinDB kdb+ plugin has different branches, such as release200 and release130. Each branch corresponds to a DolphinDB server version. Please make sure you are in the correct branch of the plugin documentation.

Before you use this plugin, please read [DolphinDB Plugin](../README.md) first.

- [DolphinDB kdb+ Plugin](#dolphindb-kdb-plugin)
  - [1. Install Precompiled Plugin](#1-install-precompiled-plugin)
    - [1.1 Configure the Environment](#11-configure-the-environment)
    - [1.2 Load Plugin](#12-load-plugin)
  - [2. (Optional) Manually Compile Plugin](#2-optional-manually-compile-plugin)
    - [Linux](#linux)
  - [3. Methods](#3-methods)
    - [3.1 kdb::connect](#31-kdbconnect)
    - [3.2 kdb::loadTable](#32-kdbloadtable)
    - [3.3 kdb::loadFile](#33-kdbloadfile)
    - [3.4 kdb::close](#34-kdbclose)
    - [3.5 A Complete Example](#35-a-complete-example)
  - [4. Load Options](#4-load-options)
    - [4.1 `loadTable`](#41-loadtable)
    - [4.2 `loadFile`](#42-loadfile)
  - [5. Loading Different Types of kdb+ Tables](#5-loading-different-types-of-kdb-tables)
  - [6. Data Type Conversion](#6-data-type-conversion)
  - [7. (Contributors) Testing Your Code](#7-contributors-testing-your-code)

## 1. Install Precompiled Plugin

### 1.1 Configure the Environment

**Install kdb+** 

kdb+ installation is only required for the `loadTable()` method.

Download a 64-bit kdb+ personal edition from the [official website](https://kx.com/kdb-personal-edition-download/).

**Install zlib**

The plugin requires zlib for data decompression.

- Ubuntu

```
sudo apt install zlib1g
```

- CentOS

```
yum install -y zlib zlib-devel
```

### 1.2 Load Plugin

Download the precompiled plugin and library files from the `bin/` directory. Enter the following command in DolphinDB to load the plugin:

```
loadPlugin("/path/to/plugin/PluginKDB.txt")
```

Please note that the plugin version must be consistent with your DolphinDB server version. 

## 2. (Optional) Manually Compile Plugin

### Linux

```
cd /path/to/plugins/kdb
mkdir build
cd build
cmake ..
make
```

After compilation, refer to the description in [1.2 Load Plugin](#12-load-plugin) to load the plugin in DolphinDB.

## 3. Methods

### 3.1 kdb::connect

**Syntax**

connect(host, port, usernamePassword)

**Arguments**

- host: the IP address where the kdb+ server is running.
- port: the port that the kdb+ server is listening on.
- usernamePassword: a STRING indicating the username and password for the kdb+ database you're connecting to. Specify the value in this format: "username:password". If the kdb+ server does not require authentication, leave this parameter empty or specify an arbitrary string.

**Details**

Establish a connection to the kdb+ server. Return a connection handle.

If the connection fails, an exception is thrown. Possible causes are: 

1. The username or password is wrong;
2. The port number for the server does not exist;
3. Timeout

**Examples**

Suppose the username and password ("admin:123456") are stored in *../passwordfiles/usrs*, and the kdb+ server and DolphinDB server are both on the same machine:

```
kdb shell:         q -p 5000 -U ../passwordfiles/usrs   // note that "-U" must be capitalized
DolphinDB shell:   handle = kdb::connect("127.0.0.1", 5000, "admin:123456")
```

If the kdb+ server you're connecting to does not require authentication:

```
handle = kdb::connect("127.0.0.1", 5000)
```

 

### 3.2 kdb::loadTable

**Syntax**

```
loadTable(handle, tablePath, symPath)
```

**Arguments**

- handle: the connection handle returned by `kdb::loadTable`
- tablePath is a STRING indicating the path to the kdb+ table you're loading. It must be the directories of column files of a splayed table, partitioned table or segmented table.
- symPath: is a STRING indicating the path to the sym file for the table. Leave this parameter empty only when the table does not contain an enumerated column of type symbol.

Note: It is recommended to separate the paths with a slash ("/"), instead of a backslash ("\".)

**Details**

Load data from a connected kdb+ database as an in-memory table in DolphinDB.

> In kdb+, symbols containing few distinctive values are saved as integers in sym files through enumeration to reduce storage requirements. Therefore, to load a table containing an enumerated column of type symbol, the system must load the associated sym file first.

**Examples**

```
// the table contains an enumerated symbol column
DATA_DIR="/path/to/data/kdb_sample"
Txns = kdb::loadTable(handle, DATA_DIR + "/2022.06.17/Txns", DATA_DIR + "/sym")

// there's no symbol data in the splayed table, or the symbol column is not enumerated in a single table
DATA_DIR="/path/to/data/kdb_sample"
Txns = kdb::loadTable(handle, DATA_DIR + "/2022.06.17/Txns", DATA_DIR)
```

### 3.3 kdb::loadFile

**Syntax**

```
loadFile(tablePath, symPath)
```

**Arguments**

- tablePath: tablePath is a STRING indicating the path to the kdb+ table you're loading. If it's a splayed table, partitioned table or segmented table, specify the directory of column files. If it's a single object, specify the object file.
- symPath: is a STRING indicating the path to the sym file for the table. Leave this parameter empty only when the table does not contain an enumerated column of type symbol.

Note: It is recommended to separate the paths with a slash ("/"), instead of a backslash ("\".)

**Details**

Directly read the specified kdb+ data files on disk and load the file data to DolphinDB as an in-memory table.

> In kdb+, symbols containing few distinctive values are saved as integers in sym files through enumeration to reduce storage requirements. Therefore, to load a table containing an enumerated column of type symbol, the system must load the associated sym file first.

**Examples**

```
//the table contains an enumerated symbol column
DATA_DIR="/path/to/data/kdb_sample"
Txns = kdb::loadFile(handle, DATA_DIR + "/2022.06.17/Txns", DATA_DIR + "/sym")


//there's no symbol data in the splayed table, or the symbol column is not enumerated in a single table
DATA_DIR="/path/to/data/kdb_sample"
Txns = kdb::loadFile(handle, DATA_DIR + "/2022.06.17/Txns", DATA_DIR)
```

### 3.4 kdb::close

**Syntax**

```
close(handle)
```

**Arguments**

- handle: the connection handle returned by `kdb::loadTable`

**Details**

Close the connection to the kdb+ server.

**Examples**

```
kdb::close(handle)
```

### 3.5 A Complete Example

```
loadPlugin("/home/DolphinDBPlugin/kdb/build/PluginKDB.txt")
go
// connect to the kdb+ database
handle = kdb::connect("127.0.0.1", 5000, "admin:123456")

// specify the file path
DATA_DIR="/home/kdb/data/kdb_sample"

// Load data to DolphinDB through loadTable
Daily = kdb::loadTable(handle, DATA_DIR + "/2022.06.17/Daily/", DATA_DIR + "/sym")
Minute = kdb::loadTable(handle, DATA_DIR + "/2022.06.17/Minute", DATA_DIR + "/sym")
Ticks = kdb::loadTable(handle, DATA_DIR + "/2022.06.17/Ticks/", DATA_DIR + "/sym")
Orders = kdb::loadTable(handle, DATA_DIR + "/2022.06.17/Orders", DATA_DIR + "/sym")
Syms = kdb::loadTable(handle, DATA_DIR + "/2022.06.17/Syms/", DATA_DIR + "/sym")
Txns = kdb::loadTable(handle, DATA_DIR + "/2022.06.17/Txns", DATA_DIR + "/sym")
kdb::close(handle)

// DolphinDB Load data to DolphinDB by reading disk files
Daily2 = kdb::loadFile(DATA_DIR + "/2022.06.17/Daily", DATA_DIR + "/sym")
Minute2= kdb::loadFile(DATA_DIR + "/2022.06.17/Minute/", DATA_DIR + "/sym")
Ticks2 = kdb::loadFile(DATA_DIR + "/2022.06.17/Ticks/", DATA_DIR + "/sym")
Orders2 = kdb::loadFile(DATA_DIR + "/2022.06.17/Orders/", DATA_DIR + "/sym")
Syms2 = kdb::loadFile(DATA_DIR + "/2022.06.17/Syms/", DATA_DIR + "/sym")
Txns2 = kdb::loadFile(DATA_DIR + "/2022.06.17/Txns/", DATA_DIR + "/sym")
```

## 4. Load Options

### 4.1 `loadTable`

For this option, call the plugin methods in the following sequence: connect() → loadTable() → close()

Note:

1. Please make sure that the table to be loaded doesn't contain nested columns.
2. The *tablePath* parameter of `loadTable` must be a single object file, or the directory of column files for a splayed table, partitioned table or segmented table.

### 4.2 `loadFile`

For this option, you only need to call the `loadFile()` method. 

Note:

1. This method cannot read the file of a single object.
2. This method only reads data compressed by gzip.
3. Please make sure that the table to be loaded doesn't contain nested columns.
4. The *tablePath* parameter of `loadFile` must be the directories of column files of a splayed table, partitioned table or segmented table.

## 5. Loading Different Types of kdb+ Tables

 

- single object

A single object can only be loaded using the `loadTable()` method. For example:

```
The directory structure:
path/to/data
├── sym
└── table_name
```

```
handle = kdb::connect("127.0.0.1", 5000, "username:password");
table = kdb::loadTable(handle, "path/to/data/table_name", "path/to/data/sym");
```

- splayed table

A splayed table can be loaded using the `loadTable()` method or the `loadFile()` method. 

If the table is not compressed, or used gzip compression algorithm as it is written to disk, it is recommended to use `loadFile()` for higher efficiency.

For example:

```
The directory structure:
path/to/data
├── sym
└── table_name
    ├── date
    ├── p
    └── ti
```

```
handle = kdb::connect("127.0.0.1", 5000, "username:password");
table1 = kdb::loadTable(handle, "path/to/data/table_name/", "path/to/data/sym");
table2 = kdb::loadTable("path/to/data/table_name/", "path/to/data/sym");
```

- partitioned table

Currently, we don't support loading an entire partitioned table or database by specifying the root directory. Alternatively, you can specify the *tablePath* parameter as the path to the table in each partition to load the tables separately, then combine them into a complete partitioned table in DolphinDB through script.

For example:

```
the directory structure:
path/to/data
├── sym
├── 2019.01.01
│   └── table_name
│       ├── p
│       └── ti
├── 2019.01.02
│   └── table_name
│       ├── p
│       └── ti
└── 2019.01.03
    └── table_name
        ├── p
        └── ti
```

```
// get the information on all files under the directory
fileRes=files("path/to/data");

// delete the sym files and read the data files
delete from fileRes where filename='sym';
name='table_name';
files = exec filename from fileRes;

// create an in-memory table and specify its schema
table=table(10:0,`p`ti`date, [SECOND,DOUBLE,DATE])

// load the data in each partition
for (file in files) {
        t = kdb::loadFile("path/to/data" +'/' + file +'/' + tablename + '/');

        // add a column indicating the partition name to the loaded data
        addColumn(t, ["date"],[DATE])
        length=count(t)
        newCol=take(date(file), length)
        replaceColumn!(t, "date", newCol)

        // append the data to the in-memory table
        table.append!(t);
}
```

- segmented table

A segmented table can be loaded to DolphinDB in the same way as a partitioned table. See the last bullet point.

## 6. Data Type Conversion

| kdb+      | DolphinDB     | Size | Note                                                         |
| :-------- | :------------ | :--- | :----------------------------------------------------------- |
| boolean   | BOOL          | 1    |                                                              |
| guid      | UUID          | 16   |                                                              |
| byte      | CHAR          | 1    | There is no byte type in DolphinDB. Byte data is converted to CHAR of the same length. |
| short     | SHORT         | 2    |                                                              |
| int       | INT           | 4    |                                                              |
| long      | LONG          | 8    |                                                              |
| real      | FLOAT         | 4    |                                                              |
| float     | DOUBLE        | 8    |                                                              |
| char      | CHAR          | 1    | There will be no conversion as a null char ("") is treated as a space (" ") in kdb+. |
| symbol    | SYMBOL        | 4    |                                                              |
| timestamp | NANOTIMESTAMP | 8    |                                                              |
| month     | MONTH         | 4    |                                                              |
| data      | DATE          | 4    |                                                              |
| datetime  | TIMESTAMP     | 8    |                                                              |
| timespan  | NANOTIME      | 8    |                                                              |
| minute    | MINUTE        | 4    |                                                              |
| second    | SECOND        | 4    |                                                              |
| time      | TIME          | 4    |                                                              |

## 7. (Contributors) Testing Your Code

Starting from version 1.30.23/2.00.11, the kdb+ plugin provides test data and test script to help developers validate their code.

**Testing Files**

Files required for testing are placed under the `test` directory of the plugin. Below are the details:

``` shell
kdb
├── lib
├── src
├── test                  // includes all files required for testing
│   ├── data              // data files used in testing, including files for persistence and authentication
│   │   └── ...
│   ├── setup             // folder for configuration parameters
│   │   └── settings.txt  // TXT files containing configuration parameters. Must be updated before running the test.
│   └── test_kdb.txt      // test script for dolphindb kdb+ plugin
├── CMakeLists.txt
└── ...
```

**Testing Steps**
1. Change the parameters in the `test/setup/settings.txt` file. Below are the required parameters for the test script to run: 

    - DATA_DIR: kdb test data which is located under `/test/data`. Specify the correct directory path.
    - HOST: the IP address where the kdb+ server is running.
    - PORT: the port that the kdb+ server is listening on.
    - usernamePasswor: the username and password for the kdb+ server.
    - pluginTxtPath：path to the compiled plugin file in TXT format. Plugin library files must be saved under the same directory.

2. Start a kdb+ server. For details, refer to kdb+ official documentation: [connecting-to-a-kdb-process](https://code.kx.com/q/wp/capi/#connecting-to-a-kdb-process)

3. Start the DolphinDB server.

4. Run the following script in DolphinDB:

    ``` dolphindb
    login(`admin,`123456);
    test("<plugin_src_path>/test/test_kdb.txt");
    ```

The test result will be printed. 

For more information on the use of the `test` function, please refer to the DolphinDB server manual.