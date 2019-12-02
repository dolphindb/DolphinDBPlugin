# DolphinDB MySQL Plugin

DolphinDB的MySQL导入插件可将MySQL中的数据表或语句查询结果导入DolphinDB，并且支持数据类型转换。本插件的部分设计参考了来自Yandex.Clickhouse的mysqlxx组件。

# 安装构建

## 准备工作

安装 [git](https://git-scm.com/) 和 [CMake](https://cmake.org/)。

Ubuntu用户只需要在命令行输入以下命令即可：

```bash
$ sudo apt-get install git cmake
```

然后通过更新git子模块来下载[mariadb-connector-c](https://github.com/MariaDB/mariadb-connector-c)的源文件。

```
$ git submodule update --init --recursive
```

## 在Windows下编译安装

### 使用CMake和MinGW编译

**Note:** [cmake](https://cmake.org/) 是一个流行的项目构建工具，有助于解决第三方依赖的问题。

**Note:** [MinGW](http://www.mingw.org/) 是"Minimalist GNU for Windows"的缩写，它是一个原生Microsoft Windows应用程序的极简主义开发环境。

构建插件内容

```
mkdir build                                                        # 新建build目录
cp path_to_libDolphinDB.dll/libDolphinDB.dll build                 # 拷贝 libDolphinDB.dll 到build目录下
cp -r curl build                                                   # 拷贝 curl 头文件到build目录下
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../path_to_mysql_plugin/ -G "MinGW Makefiles"
mingw32-make -j4
```

**Remember:** 在编译开始之前，记得将`libDolphinDB.dll`和包含`curl`头文件的文件夹拷贝到`build`文件夹内。

## 在Linux下编译安装

### 使用cmake编译
**Note:** [cmake](https://cmake.org/) 是一个流行的项目构建工具，有助于解决第三方依赖的问题。

首先安装cmake

```
sudo apt-get install cmake
```

构建插件内容

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../path_to_mysql_plugin/
make -j`nproc`
```

**注意:** 编译之前请确保libDolphinDB.so在gcc可搜索的路径中,可使用LD_LIBRARY_PATH指定其路径，或者直接将其拷贝到`build`目录下。

编译之后目录下会产生libPluginHdf5.so文件。

# 用户接口

**注意:** 使用api前需使用 `loadPlugin("/path_to_PluginMySQL.txt/PluginMySQL.txt")` 导入插件。

## mysql::connect

### 语法

* `mysql::connect(host, port, user, password, db)`

### 参数

* `host`: MySQL服务器的地址，类型为`string`.
* `port`: MySQL服务器的端口，类型为`int`.
* `user`: MySQL服务器的用户名，类型为`string`.
* `password`: MySQL服务器的密码，类型为`string`.
* `db`: 要使用的数据库名称，类型为`string`.

### 详情

* 与MySQL服务器建立一个连接。返回一个MySQL连接的句柄，用于`load`、`loadEx`等操作。

### 例子

```
conn = mysql::connect(`localhost, 3306, `root, `root, `DolphinDB)
```

## mysql::showTables

### 语法

* `mysql::showTables(connection)`

### 参数

* `connection`: 通过`mysql::connect`获得的MySQL连接句柄。

### 详情

* 列出建立MySQL连接时指定的数据库中包含的所有表。

### 例子

```
conn = mysql::connect(`localhost, 3306, `root, `root, `DolphinDB)
mysql::showTables(conn)

output:
  Tables_in_DolphinDB
  -------------------
  US
```

## mysql::extractSchema

### 语法

* `mysql::extractSchema(connection, tableName)`

### 参数

* `connection`: 通过`mysql::connect`获得的MySQL连接句柄。
* `tableName`:  MySQL表名，类型为`string`.

### 详情
* 生成指定数据表的结构。

### 例子

```
conn = mysql::connect(`localhost, 3306, `root, `root, `DolphinDB)
mysql::extractSchema(conn, `US)

output:
        name    type   DolphinDBType
        PERMNO  int(11)     INT
        date    date        DATE
        SHRCD   int(11)     INT
        TICKER  varchar(10) STRING
        ...
        PRC     double      DOUBLE
```

## mysql::load

### 语法

* `mysql::load(connection, table_or_query, [schema], [startRow], [rowNum])`

### 参数

* `connection`: 通过`mysql::connect`获得的MySQL连接句柄。
* `table_or_query`: 一张MySQL中表的名字，或者类似`select * from table limit 100`的合法MySQL查询语句，类型为`string`。
* `schema`: 包含列名和列的数据类型的表。如果我们想要改变由系统自动决定的列的数据类型，需要在schema表中修改数据类型，并且把它作为load函数的一个参数。
* `startRow`: 读取MySQL表的起始行数，若不指定，默认从数据集起始位置读取。
* `rowNum`: 读取MySQL表的行数，若不指定，默认读到数据集的结尾。若`table_or_query`是查询语句，则这个参数被忽略。

### 详情

* 将MySQL表或者SQL查询结果导入DolphinDB中的内存表。
* 支持的数据类型以及数据转化规则可见用户手册数据类型章节。

### 例子

```
conn = mysql::connect(`localhost, 3306, `root, `root, `DolphinDB)
tb = mysql::load(conn, `US,,0,123456)
select count(*) from tb
```

```
conn = mysql::connect(`localhost, 3306, `root, `root, `DolphinDB)
tb = mysql::load(conn, "SELECT PERMNO FROM US LIMIT 123456")
select count(*) from tb
```

```
mysql::load(conn, "SELECT now(6)");
```

## mysql::loadEx

### 语法

* `mysql::loadEx(connection, dbHandle,tableName,partitionColumns,table_or_query,[schema],[startRow],[rowNum])`

### 参数

* `connection`: 通过`mysql::connect`获得的MySQL连接句柄。
* `dbHandle` and `tableName`: 如果我们要将输入数据文件保存在分布式数据库中，需要指定数据库句柄和表名。
* `partitionColumns`: 字符串标量或向量，表示分区列。在组合分区中，partitionColumns是字符串向量。
* `table_or_query`: 一张MySQL中表的名字，或者类似`select * from table limit 100`的合法MySQL查询语句，类型为`string`。
* `schema`: 包含列名和列的数据类型的表。如果我们想要改变由系统自动决定的列的数据类型，需要在schema表中修改数据类型，并且把它作为load函数的一个参数。
* `startRow`: 读取MySQL表的起始行数，若不指定，默认从数据集起始位置读取。
* `rowNum`: 读取MySQL表的行数，若不指定，默认读到数据集的结尾。若`table_or_query`是查询语句，则这个参数被忽略。

### 详情

* 将MySQL中的表或SQL查询结果转换为DolphinDB数据库的分布式表，然后将表的元数据加载到内存中。
* 支持的数据类型,以及数据转化规则可见数据类型章节。

### 例子

* 将数据导入磁盘上的分区表
```
dbPath = "C:/..."
db = database(dbPath, RANGE, 0 500 1000)
mysql::loadEx(conn, db,`tb, `PERMNO, `US)
tb = loadTable(dbPath, `tb)
```

```
dbPath = "C:/..."
db = database(dbPath, RANGE, 0 500 1000)
mysql::loadEx(conn, db,`tb, `PERMNO, "SELECT * FROM US LIMIT 1000");
tb = loadTable(dbPath, `tb)
```

* 将数据导入内存中的分区表
```
db = database("", RANGE, 0 50000 10000)
tb = mysql::loadEx(conn, db,`tb, `PERMNO, `US)
```

```
db = database("", RANGE, 0 50000 10000)
tb = mysql::loadEx(conn, db,`tb, `PERMNO, "SELECT * FROM US LIMIT 100");
```

* 将数据导入DFS分布式文件系统中的分区表
```
db = database("dfs://US", RANGE, 0 50000 10000)
mysql::loadEx(conn, db,`tb, `PERMNO, `US)
tb = loadTable("dfs://US", `tb)
```

```
db = database("dfs://US", RANGE, 0 50000 10000)
mysql::loadEx(conn, db,`tb, `PERMNO, "SELECT * FROM US LIMIT 1000");
tb = loadTable("dfs://US", `tb)
```

# 支持的数据类型

##  整型

| MySQL类型          | 对应的DolphinDB类型 |
| ------------------ | :------------------ |
| tinyint            | CHAR                |
| tinyint unsigned   | SHORT               |
| smallint           | SHORT               |
| smallint unsigned  | INT                 |
| mediumint          | INT                 |
| mediumint unsigned | INT                 |
| int                | INT                 |
| int unsigned       | LONG                |
| bigint             | LONG                |
| bigint unsigned    | (不支持) LONG       |

* DolphinDB中数值类型都为有符号类型,为了防止溢出,所有无符号类型会被转化为高一阶的有符号类型。例如，无符号CHAR转化为有符号SHORT，无符号SHORT转化为有符号INT，等等。64位无符号类型不予支持。
* DolphinDB不支持 unsigned long long 类型，如果mysql中的类型为`bigint unsigned`, 可在load或者loadEx的schema参数里面设置为`DOUBLE`或者`FLOAT`。
* DolphinDB中各类整形的最小值为NULL值，如`CHAR`的`-128`，`SHORT`的`-32,768`， `INT`的`-2,147,483,648`以及`LONG`的`-9,223,372,036,854,775,808`。


## 浮点数类型

| MySQL类型  | 对应的DolphinDB类型 |
| ---------- | :------------------ |
| double     | DOUBLE              |
| decimal    | DOUBLE              |
| newdecimal | DOUBLE              |
| float      | FLOAT               |

注:IEEE754浮点数类型皆为有符号数。

* 以上浮点类型皆可转化为DolphinDB中的数值相关类型(BOOL, CHAR, SHORT, INT, LONG, FLOAT, DOUBLE)。

## 时间类型

| MySQL类型 | 对应的DolphinDB类型 |
| --------- | :------------------ |
| date      | DATE                |
| time      | TIME                |
| datetime  | DATETIME            |
| timestamp | TIMESTAMP           |
| year      | INT                 |

* 以上类型皆可以转化为DolphinDB中的时间相关类型(DATE, MONTH, TIME, MINUTE, SECOND, DATETIME, TIMESTAMP, NANOTIME, NANOTIMESTAMP)。

## 字符串类型

| MySQL类型           | 对应的DolphinDB类型 |
| ------------------- | :------------------ |
| char  (len <= 10)   | SYMBOL              |
| varchar (len <= 10) | SYMBOL              |
| char  (len > 10)    | STRING              |
| varchar (len > 10)  | STRING              |
| other string types  | STRING              |

* 长度小于等于10的`char`和`varchar`将被转化为`SYMBOL`类型，其余转化为`STRING`类型。
* string类型可以转化为转化为DolphinDB中的字符串相关类型(`STRING`,`SYMBOL`)。

## 枚举类型

| MySQL类型 | 对应的DolphinDB类型 |
| --------- | :------------------ |
| enum      | SYMBOL              |

* enum类型可以转化为DolphinDB中的字符串相关类型(`STRING`,`SYMBOL`)，默认转化为`SYMBOL`类型。

# 导入数据性能

## 硬件环境

* CPU: i7-7700 3.60GHZ
* 硬盘: SSD， 读速为每秒460~500MB/s。

## 导入数据

* 美国股票市场从1990年至2016年的每日数据，共有22列50,591,907行, 6.5GB。

## 导入耗时

160.5秒