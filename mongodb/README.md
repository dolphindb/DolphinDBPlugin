# DolphinDB MongoDB Plugin

DolphinDB mongodb插件可以建立与mongodb服务器的连接，然后导入数据到DolphinDB的内存表中。

## 1. 安装构建

### 1.1 预编译安装

#### Linux
执行Linux命令，指定插件运行时需要的动态库路径
``` shell
export LD_LIBRARY_PATH=<PluginDir>/mongodb/bin/linux64:$LD_LIBRARY_PATH //指定动态库位置 
```

在DolphinDBPlugin/httpClient/bin/linux64目录下有预先编译的插件文件，在DolphinDB中执行以下命令导入mongodb插件：

```
cd DolphinDB/server //进入DolphinDB server目录
./dolphindb //启动 DolphinDB server
 loadPlugin("<PluginDir>/mongodb/build/linux64/PluginMongodb.txt") //加载插件
```

#### Windows

在window版本mongodb插件中，编译完成后会把所包含的动态库复制到生成的libPluginMongodb.dll的同一目录下，window系统在同一目录下搜索依赖的动态库。

在DolphinDBPlugin/httpClient/bin/win64目录下有预先编译的插件文件，在DolphinDB中执行以下命令导入mongodb插件：

```
 loadPlugin("<PluginDir>/mongodb/bulid/win64/PluginMongodb.txt") //加载插件
 ```


### 1.2 自行编译

要用到mongodb-c-driver，它的依赖库包括snappy，ICU，openssl。

在<PluginDir>/mongodb/bin目录下有对应与linux64和win64的依赖库，只需要cmake和对应编译器(linux为g++,window为MinGW)即可在本地编译mongodb插件。

#### Linux

##### 使用cmake构建：

安装cmake：

```
sudo apt-get install cmake
```
构建插件内容：

```
mkdir build
cd build
cmake  ../
make
```

**注意**:编译之前请确保libDolphinDB.so在gcc可搜索的路径中。可使用`LD_LIBRARY_PATH`指定其路径，或者直接将其拷贝到build目录下。

编译后目录下会产生文件libPluginMongodb.so和PluginMongodb.txt。

#### Windows

##### 在Windows环境中需要使用CMake和MinGW编译

- 下载安装[MinGW](http://www.mingw.org/)。确保将bin目录添加到系统环境变量Path中。
- 下载安装[cmake](https://cmake.org/)。

###### 使用cmake构建：

在编译开始之前，要将libDolphinDB.dll拷贝到build文件夹。

构建插件内容：

```
mkdir build                                                        # 新建build目录
cp <ServerDir>/libDolphinDB.dll build                 # 拷贝 libDolphinDB.dll 到build目录下
cd build
cmake  ../ -G "MinGW Makefiles"
mingw32-make -j4
```

编译后目录下会产生文件libPluginMongodb.dll和PluginMongodb.txt，还会把<PluginDir>/mongodb/bin/windows下的4个动态库拷贝到该目录下。

>以下编译的mongodb-c-driver、snappy、ICU和openssl的依赖库文件，都可以在<PluginDir>/mongodb/bin目录下找到。

### 1.3编译依赖库

#### Linux

##### 安装版本1.0.2的openssl

 ```
wget https://www.openssl.org/source/old/1.0.2/openssl-1.0.2i.tar.gz
tar -xzf openssl-1.0.2i.tar.gz
cd openssl-1.0.2i
./config --prefix=/usr/local/openssl1.0.2 -fPIC
make 
sudo make install
```
--prefix是为了指定安装位置，后面会使用到这个版本的openssl的头文件和静态库。

##### 安装snappy

```
wget https://github.com/google/snappy/archive/1.1.7.tar.gz
tar -zxf 1.1.7.tar.gz
cd snappy-1.1.7/cmake
CXXFLAGS="-fPIC" cmake ..
make
sudo make install
```

##### 安装ICU

```
wget https://github.com/unicode-org/icu/releases/download/release-52-2/icu4c-52_2-src.tgz
tar -xzf icu4c-52_2-src.tgz
cd icu/source
./configure
make
sudo make install
```

##### 安装mongo-c-driver

需要设置环境变量，在命令行中设置，正是刚刚安装openssl的位置。
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
这里我们可以在终端看到mongodb-c-driver需要的依赖是否安装完全。
```
make
sudo make install
```

##### 准备依赖库

将对应分支的libDolphinDB.so以及其它依赖库拷贝到DolphinDBPlugin/mongodb/bin/linux。

```
cd DolphinDBPlugin/mongodb/bin/linux
cp <ServerDir>/libDolphinDB.so . 
cp /usr/local/lib/libmongoc-1.0.so.0 .
cp /usr/local/lib/libbson-1.0.so.0 .
cp /usr/local/lib/libicudata.so.52 .
cp /usr/local/lib/libicuuc.so.52 .
```

##  2. 用户接口

### 2.1 mongodb::connect

#### 语法

mongodb::connect(host, port, user, password, [db])

#### 参数

* host: MongoDB服务器的地址，类型为string。
* port: MongoDB服务器的端口，类型为int。
* user: MongoDB服务器的用户名，类型为string。如果没有开启mongodb用户权限认证，则填写空字符串""。
* password: MongoDB服务器的密码，类型为string。如果没有开启mongodb用户权限认证，则填写空字符串""。
* db: 验证登录用户的数据库，类型为string。在mongodb中存储对应登录用户的数据库。如果不填写，将以参数`host`指定的mongodb服务器的`admin`数据库进行登录用户验证。

#### 详情

与MongoDB服务器建立一个连接。返回一个MongoDB连接的句柄，用于load。

#### 例子

```
conn = mongodb::connect(`localhost, 27017, `root, `root, `DolphinDB)
conn2 = mongodb::connect(`localhost, 27017, `root, `root)
```

### 2.2 mongodb::load

#### 语法

mongodb::load(connection, collcetionName, query, option, [schema])

#### 参数

* connection: 通过mongodb::connect获得的MongoDB连接句柄。
* collcetionName: 一个MongoDB中集合的名字。有两种参数模式(`collectionName和"databaseName:collectionName")，第一种会查询由mongodb::connect创建的connection的database，第二种是查询指定database中的collection。
采用"databaseName:collectionName"的样式会提供一次对临时数据库的查询，将会访问由mongodb::connect方法创建mongodb连接时设置的数据库或者是当前指定的数据库，
上一次load方法设置的临时数据库不会影响当前数据库的选择。
* query: MongoDB查询条件，保留bson格式的json文档，类似：{ "aa" : { "$numberInt" : "13232" } }、{ "datetime" : { "$gt" : {"$date":"2019-02-28T00:00:00.000Z" }} }，类型为string。
* option: MongoDB查询选项，保留bson格式的json文档，类似：{"limit":123}对查询结果在MongoDB中进行预处理再返回，类型为string。
* schema: 包含列名和列的数据类型的表。如果我们想要改变由系统自动决定的列的数据类型，需要在schema表中修改数据类型，并且把它作为load函数的一个参数。

#### 详情

将MongoDB的查询结果导入DolphinDB中的内存表。支持的数据类型以及数据转化规则参见用户手册[数据类型章节](https://www.dolphindb.cn/cn/help/DataTypesandStructures/DataTypes/index.html)。

#### 例子

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

#### 语法

mongodb::aggregate(connection, collcetionName, pipeline, option, [schema])

#### 参数

* connection: 通过mongodb::connect获得的MongoDB连接句柄。
* collcetionName: 一个MongoDB中集合的名字。有两种参数模式(`collectionName和"databaseName:collectionName")，第一种会查询由mongodb::connect创建的connection的database，第二种是查询指定database中的collection。

  采用"databaseName:collectionName"的样式会提供一次对临时数据库的查询，将会访问由mongodb::connect方法创建mongodb连接时设置的数据库或者是当前指定的数据库，
  上一次load方法设置的临时数据库不会影响当前数据库的选择。
* pipeline: MongoDB聚合管道，保留bson格式的json文档，类似：{$group : {_id : "$by_user", num_tutorial : {$sum : 1}}}，类型为string。
* option: MongoDB查询选项，保留bson格式的json文档，类似：{"limit":123}对查询结果在MongoDB中进行预处理再返回，类型为string。
* schema: 包含列名和列的数据类型的表。如果我们想要改变由系统自动决定的列的数据类型，需要在schema表中修改数据类型，并且把它作为load函数的一个参数。

#### 详情

将MongoDB的（聚合）查询结果导入DolphinDB中的内存表。支持的数据类型以及数据转化规则参见用户手册[数据类型章节](https://www.dolphindb.cn/cn/help/DataTypesandStructures/DataTypes/index.html)。

#### 例子

```
conn = mongodb::connect(`localhost, 27017, "", "", `DolphinDB)
pipeline = "{ \"pipeline\" : [ { \"$project\" : { \"str\" : \"$obj1.str\" } } ] }"
option="{}"
mongodb::aggregate(conn, "test1:collnetion1",pipeline,option)
```

### 2.4 mongodb::close

#### 语法

mongodb::close(connection)

#### 参数

* connection: 通过mongodb::connect获得的MongoDB连接句柄。

#### 详情

关闭一个MongoDB连接句柄。

#### 例子

```
conn = mongodb::connect(`localhost, 27017, `root, `root, `DolphinDB)
query=`{ "datetime" : { "$gt" : {"$date":"2019-02-28T00:00:00.000Z" }} }
option=`{"limit":1234}
tb = mongodb::load(conn, `US,query,option)
select count(*) from tb
mongodb::close(conn)
```

### 2.5 mongodb::parseJson

#### 语法

mongodb::parseJson(str, keys, colnames, colTypes)

#### 详情
解析json类型的数据，转换到DolphinDB的内存表并返回该内存表。

#### 参数
* str: 需要解析的json格式的字符串，为string类型的vector。
* keys: 原始json的键，为string类型的vector。
* colnames: 结果表json的键，为string类型的vector。
* colTypes: 向量，表示结果表中的数据类型。
colTypes支持BOOL, INT, FLOAT, DOUBLE, STRING以及BOOL[], INT[], FLOAT[], DOUBLE[]类型的array vector。其中支持将json中的int, float, double类型转换为INT, FLOAT, DOUBLE，且三种类型之间可以相互转换。

#### 例子

```
data = ['{"a": 1, "b": 2}', '{"a": 2, "b": 3}']
 mongodb::parseJson(data, 
`a`b, 
`col1`col2,
[INT, INT] )
```

### 2.6 mongodb::getCollections

#### 语法

mongodb::getCollections([databaseName])

#### 参数

* databaseName: 需要查询的数据库。如果不填，则为mongodb::connect所选的database。

#### 详情

获取指定database的所有集合的名字。

#### 例子

```
conn = mongodb::connect("192.168.1.38", 27017, "", "")
mongodb::getCollections(conn, "dolphindb")
```

## 3 查询数据示例

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


## 3. 支持的数据类型

### 3.1 整型

| MongoDB类型          | 对应的DolphinDB类型 |
| ------------------ | :------------------ |
| int32            | INT                |
| int64(long)   | LONG               |
| bool           | BOOL               |

DolphinDB中各类整型的最小值（例如：INT的-2,147,483,648以及LONG的-9,223,372,036,854,775,808）为NULL值。


### 3.2 浮点数类型

| MongoDB类型  | 对应的DolphinDB类型 |
| ---------- | :------------------ |
| double     | DOUBLE              |
| decimal128    | DOUBLE              |


### 3.3 时间类型

| MongoDB类型 | 对应的DolphinDB类型 |
| --------- | :------------------ |
|date     | timestamp             |

### 3.4 字符串类型

| MongoDB类型           | 对应的DolphinDB类型 |
| ------------------- | :------------------ |
| string   | STRING              |
| symbol | STRING              |
| oid          | STRING             |