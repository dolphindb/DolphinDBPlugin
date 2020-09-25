# DolphinDB MongoDB Plugin

DolphinDB mongodb插件可以建立与mongodb服务器的连接，然后导入数据到DolphinDB的内存表中。

## 1. 安装构建

#### 1.1编译安装

#### Linux

###### 使用cmake构建：

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

##### Windows

###### 在Windows环境中需要使用CMake和MinGW编译

- 下载安装[MinGW](http://www.mingw.org/)。确保将bin目录添加到系统环境变量Path中。
- 下载安装[cmake](https://cmake.org/)。

###### 使用cmake构建：

在编译开始之前，要将libDolphinDB.dll拷贝到build文件夹。

构建插件内容：

```
mkdir build                                                        # 新建build目录
cp path_to_libDolphinDB.dll/libDolphinDB.dll build                 # 拷贝 libDolphinDB.dll 到build目录下
cd build
cmake  ../ -G "MinGW Makefiles"
mingw32-make -j4
```

编译后目录下会产生文件libPluginMongodb.dll和PluginMongodb.txt，还会把/path/to/mongodbPlugin/bin/windows下的4个动态库拷贝到该目录下。


#### 1.2 插件加载
#### Linux
```
cd DolphinDB/server //进入DolphinDB server目录
export LD_LIBRARY_PATH=/path/to/mongodbPlugin/mongodb/bin/linux:$LD_LIBRARY_PATH //指定动态库位置 
./dolphindb //启动 DolphinDB server
 loadPlugin("/path/to/mongodbPlugin/build/PluginMongodb.txt") //加载插件
```

#### Windows
在window版本mongodb插件中，编译完成后会把所包含的动态库复制到生成的libPluginMongodb.dll的同一目录下，window系统在同一目录下搜索依赖的动态库。

所以只需要执行脚本loadPlugin即可加载mongodb插件。
```
 loadPlugin("/path/to/mongodbPlugin/bulid/PluginMongodb.txt") //加载插件
 ```
##  2. 用户接口

### 2.1 mongodb::connect

#### 语法

mongodb::connect(host, port, user, password, db)

#### 参数

* host: MongoDB服务器的地址，类型为string。
* port: MongoDB服务器的端口，类型为int。
* user: MongoDB服务器的用户名，类型为string。
* password: MongoDB服务器的密码，类型为string。
* db: 要使用的数据库名称，类型为string。

#### 详情

与MongoDB服务器建立一个连接。返回一个MongoDB连接的句柄，用于load。

#### 例子

```
conn = mongodb::connect(`localhost, 27017, `root, `root, `DolphinDB)
```

### 2.2 mongodb::load

#### 语法

mongodb::load(connection,collcetion, query, option)

#### 参数

* connection: 通过mongodb::connect获得的MongoDB连接句柄。
* collcetionName: 一个MongoDB中集合的名字，或者类似select * from table limit 100的合法MySQL查询语句，类型为string。
* query: MongoDB查询条件，保留bson格式的json文档，类似:{ "aa" : { "$numberInt" : "13232" } }、{ "datetime" : { "$gt" : {"$date":"2019-02-28T00:00:00.000Z" }} }，类型为string。
* option: MongoDB查询选项，保留bson格式的json文档，类似:{"limit":123}对查询结果在MongoDB中进行预处理再返回，类型为string。

#### 详情

将MongoDB的查询结果导入DolphinDB中的内存表。支持的数据类型以及数据转化规则可见用户手册数据类型章节。

#### 例子

```
conn = mongodb::connect(`localhost, 27017, `root, `root, `DolphinDB)
query=`{ "datetime" : { "$gt" : {"$date":"2019-02-28T00:00:00.000Z" }} }
option=`{"limit":1234}
tb = mysql::load(conn, `US,query,option)
select count(*) from tb
```

### 2.3. mongodb::close

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
tb = mysql::load(conn, `US,query,option)
select count(*) from tb
mongodb::close(conn)
```

### 2.4. 查询数据示例
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

DolphinDB中各类整形的最小值为NULL值，例如：INT的-2,147,483,648以及LONG的-9,223,372,036,854,775,808。


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

