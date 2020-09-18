# DolphinDB MongoDB Plugin

DolphinDB mongodb插件可以建立与mongodb服务器的连接，然后导入数据到DolphinDB的内存表中。

## 1. 安装构建

这里我们要用到mongodb-c-driver，它的依赖库包括snappy，ICU，openssl。

### 1.1 安装版本1.0.2的openssl
 ```
wget https://www.openssl.org/source/old/1.0.2/openssl-1.0.2i.tar.gz
tar -xzf openssl-1.0.2i.tar.gz
cd openssl-1.0.2i
./config --prefix=/usr/local/openssl1.0.2 -fPIC
make 
sudo make install
```
--prefix是为了指定安装位置，后面会使用到这个版本的openssl的头文件和静态库。

### 1.2 安装snappy
```
wget https://github.com/google/snappy/archive/1.1.7.tar.gz
tar -zxf 1.1.7.tar.gz
cd snappy-1.1.7/cmake
CXXFLAGS="-fPIC" cmake ..
make
sudo make install
```
### 1.3 安装ICU
```
wget https://github.com/unicode-org/icu/releases/download/release-52-2/icu4c-52_2-src.tgz
tar -xzf icu4c-52_2-src.tgz
cd icu/source
./configure
make
sudo make install
```
### 1.4 安装mongo-c-driver

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
### 1.5 准备依赖库

将对应分支的libDolphinDB.so以及其它依赖库拷贝到DolphinDBPlugin/mongodb/。

```
cd DolphinDBPlugin/mongodb/
cp /path/to/libDolphinDB.so . 
cp /usr/local/lib/libmongoc-1.0.so.0 .
cp /usr/local/lib/libbson-1.0.so.0 .
cp /usr/local/lib/libicudata.so.52 .
```

### 1.6 编译插件
在以上步骤都完成之后，在DolphinDBPlugin/mongodb/目录下在命令行输入make即可构建mongodb插件动态库。如果编译成功，会生成 libPluginMongodb.so 文件。

```
cd DolphinDBPlugin/mongodb/
make
```

将 mongodb目录拷贝到DolphinDB server/plugins。

### 1.7 插件加载

```
cd DolphinDB/server //进入DolphinDB server目录
export LD_LIBRARY_PATH=/path/to/mongodbPlugin/:$LD_LIBRARY_PATH //指定动态库位置 
./dolphindb //启动 DolphinDB server
 loadPlugin("/path/to/mongodbPlugin/PluginMongodb.txt") //加载插件
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

