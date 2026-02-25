# DolphinDB MongoDB Plugin

DolphinDB MongoDB plugin enables you to connect to the MongoDB server and import its data to DolphinDB in-memory tables.

The DolphinDB MongoDB plugin has the branches [release 200](https://github.com/dolphindb/DolphinDBPlugin/tree/release200/mongodb) and [release130](https://github.com/dolphindb/DolphinDBPlugin/tree/release130/mongodb). Each plugin version corresponds to a DolphinDB server version. You're looking at the plugin documentation for release200. If you use a different DolphinDB server version, please refer to the corresponding branch of the plugin documentation.

- [DolphinDB MongoDB Plugin](#dolphindb-mongodb-plugin)
  - [1. Compile and Install Plugin](#1-compile-and-install-plugin)
    - [1.1 Install Precompiled Plugin](#11-install-precompiled-plugin)
    - [1.2 (Optional) Manually Compile Plugin](#12-optional-manually-compile-plugin)
    - [1.3  (Optional) Compile Dependency Libraries](#13--optional-compile-dependency-libraries)
  - [2. Methods](#2-methods)
    - [2.1 mongodb::connect](#21-mongodbconnect)
    - [2.2 mongodb::load](#22-mongodbload)
    - [2.3 mongodb::aggregate](#23-mongodbaggregate)
    - [2.4 mongodb::close](#24-mongodbclose)
    - [2.5 mongodb::parseJson](#25-mongodbparsejson)
    - [2.6 mongodb::getCollections](#26-mongodbgetcollections)
  - [3. Query Examples](#3-query-examples)
  - [4. Data Type Support](#4-data-type-support)
    - [4.1 Integral](#41-integral)
    - [4.2 Float](#42-float)
    - [4.3 Temporal](#43-temporal)
    - [4.4 Literal](#44-literal)

## 1. Compile and Install Plugin

### 1.1 Install Precompiled Plugin

#### Linux

Enter the following command and specify the path for dynamic shared libraries that are required by the plugin:

```
export LD_LIBRARY_PATH=<PluginDir>/mongodb/bin/linux64:$LD_LIBRARY_PATH
```

Download the precompiled plugin file under the [DolphinDBPlugin/mongodb/bin](https://github.com/dolphindb/DolphinDBPlugin/tree/release200/mongodb/bin)/linux64 directory. Execute the following script in DolphinDB to load the plugin.

```
cd DolphinDB/server //enter the DolphinDB server directory
./dolphindb //start the DolphinDB server
 loadPlugin("<PluginDir>/mongodb/build/linux64/PluginMongodb.txt") //load the plugin
```

#### Windows

Download all the precompiled plugin files (including the dynamic libraries) under the [DolphinDBPlugin/mongodb/bin](https://github.com/dolphindb/DolphinDBPlugin/tree/release200/mongodb/bin)/win64 directory. Execute the following script in DolphinDB to load the MongoDB plugin to DolphinDB:

```
 loadPlugin("<PluginDir>/mongodb/bulid/win64/PluginMongodb.txt")
```

### 1.2 (Optional) Manually Compile Plugin

#### Prerequisites

Download the precompiled dependency libraries. Find the dependency libraries for Linux64 and Win64 under the [/mongo/bin](https://github.com/dolphindb/DolphinDBPlugin/tree/release200/mongodb/bin) directory. You can compile the plugin with CMake and G++ on Linux or MinGW on Windows.

#### Linux

**Build with CMake**

Install CMake

```
sudo apt-get install cmake
```

Build plugin

```
mkdir build
cd build
cmake  ../
make
```

**Note**: Before compilation, please make sure the DolphinDB library file *libDolphinDB.so* is under the GCC search path. You can add the plugin path to the library search path LD_LIBRARY_PATH or copy it to the *build* directory.

After compilation, new files *libPluginMongodb.so* and *PluginMongodb.txt* are generated under the build directory. 

#### Windows

**Prerequisites:**

- Download and install [MinGW](http://www.mingw.org/). Add the generated bin directory to the PATH environment variable.
- Download and install [CMake](https://cmake.org/).

**Build with CMake**

Before the compilation, add the *libDolphinDB.dll* file to the build directory.

Build plugin:

```
mkdir build           #create build directory
cp <ServerDir>/libDolphinDB.dll build     # copy libDolphinDB.dll to the build directory
cd build
cmake  ../ -G "MinGW Makefiles"
mingw32-make -j4
```

After compilation, new files *libPluginMongodb.dll* and *PluginMongodb.txt* are generated under the build directory. The four dynamic libraries under /mongodb/bin/windows are also copied to the build directory.

### 1.3  (Optional) Compile Dependency Libraries

We provide precompiled dependency libraries (libmongoc, libbson, libicudata, libicuuc) at the [/mongodb/bin](https://github.com/dolphindb/DolphinDBPlugin/tree/release200/mongodb/bin) directory. Optionally, you can manually compile them by following the steps described in this section.

#### Linux

**Install OpenSSL 1.0.2**

```
wget https://www.openssl.org/source/old/1.0.2/openssl-1.0.2i.tar.gz
tar -xzf openssl-1.0.2i.tar.gz
cd openssl-1.0.2i
./config --prefix=/usr/local/openssl1.0.2 -fPIC
make 
sudo make install
```

Add "--prefix" to specify a location for all files to build to. The head and static libraries of OpenSSL 1.0.2 will be used during the installation of mongo-c-driver in a later step.

**Install snappy**

```
wget https://github.com/google/snappy/archive/1.1.7.tar.gz
tar -zxf 1.1.7.tar.gz
cd snappy-1.1.7/cmake
CXXFLAGS="-fPIC" cmake ..
make
sudo make install
```

**Install ICU**

```
wget https://github.com/unicode-org/icu/releases/download/release-52-2/icu4c-52_2-src.tgz
tar -xzf icu4c-52_2-src.tgz
cd icu/source
./configure
make
sudo make install
```

**Install mongo-c-driver**

Specify the environment variables based on the directory you just specified when installing OpenSSL. 

```
export OPENSSL_ROOT_DIR=/usr/local/openssl1.0.2
export OPENSSL_CRYPTO_LIBRARY=/usr/local/openssl1.0.2/lib
export OPENSSL_INCLUDE_DIR=/usr/local/openssl1.0.2/include/

wget https://github.com/mongodb/mongo-c-driver/releases/download/1.13.0/mongo-c-driver-1.13.0.tar.gz
tar -xzf mongo-c-driver-1.13.0.tar.gz
cd mongo-c-driver-1.13.0
mkdir cmake-build
cd cmake-build
cmake -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=OFF ..
```

Now we can check in the terminal whether all dependency libraries of mongodb-c-driver have been installed.

```
make
sudo make install
```

**Add dependency libraries**

Copy *libDolphinDB.so* and other dependency libraries to the build directory (*DolphinDBPlugin/mongodb/bin/linux*).

```
cd DolphinDBPlugin/mongodb/bin/linux
cp <ServerDir>/libDolphinDB.so . 
cp /usr/local/lib/libmongoc-1.0.so.0 .
cp /usr/local/lib/libbson-1.0.so.0 .
cp /usr/local/lib/libicudata.so.52 .
cp /usr/local/lib/libicuuc.so.52 .
```

## 2. Methods

### 2.1 mongodb::connect

**Syntax**

mongodb::connect(host, port, user, password, [db])

**Arguments**

- host: a string indicating the MongoDB server address
- port: a integer indicating the port number of the MongoDB server
- user: a string indicating the username to the MongoDB server. If the MongoDB authentication service is disabled, enter an empty string "".
- password: a string indicating the password to the MongoDB server. If the MongoDB authentication service is disabled, enter an empty string "".
- db: a string indicating the MongoDB database for user authentication. The database must have the credential information on the specified user. If this parameter is not specified, the "admin" database from the MongoDB server (specified by *host*) is used.

**Details**

Set up connection with the MongoDB server. Return a connection handle that can be used when calling `mongodb::load` and other methods.

**Examples**

```
conn = mongodb::connect(`localhost, 27017, `root, `root, `DolphinDB)
conn2 = mongodb::connect(`localhost, 27017, `root, `root)
```

### 2.2 mongodb::load

**Syntax**

mongodb::load(connection, collcetionName, query, option, [schema])

**Arguments**

- connection: the MongoDB connection handle returned by `mongodb::connect`
- collectionName: the name of a MongoDB collection. There are two options to specify this parameter: 
  - specify only the \`collection name - the system searches for the collection in the database (*db*) specified in `mongodb::connect`
  - specify "databaseName:collectionName" - the system searches for the collection in the specified database
- query: a JSON STRING indicating the query conditions in MongoDB. For example: `{ "aa" : { "$numberInt" : "13232" } }`, `{ "datetime" : { "$gt" : {"$date":"2019-02-28T00:00:00.000Z" }} }`
- option: a JSON STRING indicating MongoDB query options. For example: `{"limit":123}` limits  the number of records or documents returned in the query.  
- schema: a table containing column names and columns types. You can optionally change the default column types through the schema table and pass it to `mongodb::load` as an argument.

**Details**

Import the query result in MongoDB into a DolphinDB in-memory table. For information on data type support, see the [Data Type](https://dolphindb.com/help/DataTypesandStructures/DataTypes/index.html) section in the DolphinDB User Manual.

**Examples**

```
conn = mongodb::connect(`localhost, 27017, `root, `root, `DolphinDB)
query='{ "datetime" : { "$gt" : {"$date":"2019-02-28T00:00:00.000Z" }} }'
option='{"limit":1234}'
tb=mongodb::load(conn, `US,query,option)
select count(*) from tb
tb2 = mongodb::load(conn, 'dolphindb:US',query,option)
select count(*) from tb
schema=table(`item`type`qty as name,`STRING`STRING`INT as type)
tb2 = mongodb::load(conn, 'dolphindb:US',query,option,schema)
```

### 2.3 mongodb::aggregate

**Syntax**

mongodb::aggregate(connection, collcetionName, pipeline, option, [schema])

**Arguments**

- connection: the MongoDB connection handle returned by `mongodb::connect`
- collectionName: the name of a MongoDB collection. There are two options to specify this parameter: 
  - specify only the `collection name - the system searches for the collection in the database (db) specified in mongodb::connect
  - specify "databaseName:collectionName" - the system searches for the collection in the specified database
- pipeline: a STRING indicating the MongoDB aggregation pipeline. For example, `{$group : {_id : "$by_user", num_tutorial : {$sum : 1}}}`.
- option: a STRING indicating MongoDB query options. It is JSON document in BSON format. For example: `{"limit":123}` limits  the number of records or documents returned in the query.  
- schema: a table containing column names and columns types. You can optionally change the default column types through the schema table and pass it to `mongodb::load` as an argument.

**Details**

Import the result of an aggregate operation in MongoDB into a DolphinDB in-memory table. For information on data type support, see the [Data Type](https://dolphindb.com/help/DataTypesandStructures/DataTypes/index.html) section in the DolphinDB User Manual. 

**Examples**

```
conn = mongodb::connect(`localhost, 27017, "", "", `DolphinDB)
pipeline = "{ \"pipeline\" : [ { \"$project\" : { \"str\" : \"$obj1.str\" } } ] }"
option="{}"
mongodb::aggregate(conn, "test1:collnetion1",pipeline,option)
```

### 2.4 mongodb::close

**Syntax**

mongodb::close(connection)

**Arguments**

- the MongoDB connection handle returned by `mongodb::connect`

**Details**

Close a MongoDB server connection.

**Examples**

```
conn = mongodb::connect(`localhost, 27017, `root, `root, `DolphinDB)
query=`{ "datetime" : { "$gt" : {"$date":"2019-02-28T00:00:00.000Z" }} }
option=`{"limit":1234}
tb = mongodb::load(conn, `US,query,option)
select count(*) from tb
mongodb::close(conn)
```

### 2.5 mongodb::parseJson

**Syntax**

mongodb::parseJson(str, keys, colnames, colTypes)

**Details**

Parse a JSON string and convert it into a DolphinDB in-memory table. Return the in-memory table.

**Arguments**

- str: a STRING vector indicating the JSON string to be converted
- keys: a STRING vector indicating the keys of *str*
- colnames: a STRING vector indicating the column names in the result table. The column names correspond to the keys of *str*.
- colTypes: a vector indicating the column types in the result table. Supported DolphinDB data types: BOOL, INT, FLOAT, DOUBLE, STRING and array vector of BOOL[], INT[] and DOUBLE[] type. You can convert the int, float and double types in JSON into any of the INT, FLOAT or DOUBLE type in DolphinDB.

**Example**

```
data = ['{"a": 1, "b": 2}', '{"a": 2, "b": 3}']
 mongodb::parseJson(data, 
`a`b, 
`col1`col2,
[INT, INT] )
```

### 2.6 mongodb::getCollections

**Syntax**

mongodb::getCollections([databaseName])

**Arguments**

- databaseName: a STRING indicating a MongoDB database. If this parameter is not specified, the database specified in `mongodb::connect` is used.

**Details**

Get the names of all the collections in the specified database.

**Examples**

```
conn = mongodb::connect("192.168.1.38", 27017, "", "")
mongodb::getCollections(conn, "dolphindb")
```

## 3. Query Examples

```
query='{"dt": { "$date" : "2016-06-22T00:00:00.000Z" } }';
query='{"bcol" : false }';
query='{"open" : { "$numberInt" : "13232" } }';
query='{"vol" : { "$numberLong" : "1242434"} }';
query=' {"close" : { "$numberDouble" : "1.2199999999999999734" }';
query='{"low" : { "$gt" : { "$numberDecimal" : "0.219711" } } }';
query='{"uid" : { "$oid" : "1232430aa00000000000be0a" } }';
query=' {"sid" : { "$symbol" : "fdf" } }';
query='{"symbol" : "XRPUSDT.BNC"}';
query='{"ts" : { "$date" : { "$numberLong" : "1600166651000" } }';
query='{}';
option='{}';
con=mongodb::connect(`localhost,27017,`admin,`123456,`dolphindb);
res=mongodb::load(con,`collection1,query,option);
mongodb::close(con);
t = select * from res
```

## 4. Data Type Support

### 4.1 Integral

| Data Type in MongoDB | Data Type in DolphinDB |
| :------------------- | :--------------------- |
| int32                | INT                    |
| int64(long)          | LONG                   |
| bool                 | BOOL                   |

In DolphinDB, the smallest value of each integral type (e.g. -2,147,483,648 for INT and -9,223,372,036,854,775,808 for LONG) is a NULL value.

### 4.2 Float

| Data Type in MongoDB | Data Type in DolphinDB |
| :------------------- | :--------------------- |
| double               | DOUBLE                 |
| decimal128           | DOUBLE                 |

### 4.3 Temporal

| Data Type in MongoDB | Data Type in DolphinDB |
| :------------------- | :--------------------- |
| double               | DOUBLE                 |
| decimal128           | DOUBLE                 |

### 4.4 Literal

| Data Type in MongoDB | Data Type in DolphinDB |
| :------------------- | :--------------------- |
| string               | STRING                 |
| symbol               | STRING                 |
| oid                  | STRING                 |
