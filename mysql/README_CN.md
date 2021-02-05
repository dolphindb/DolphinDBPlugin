# DolphinDB MySQL Plugin

DolphinDB的MySQL导入插件可将MySQL中的数据表或语句查询结果高速导入DolphinDB，并且支持数据类型转换。本插件的部分设计参考了来自Yandex.Clickhouse的mysqlxx组件。

## 1. 安装

### 1.1 预编译安装

用户可以导入预编译好的MySQL插件（DolphinDB安装包中或者bin目录下)。


在DolphinDB中执行以下命令导入MySQL插件：

Linux环境：
```
loadPlugin("/path/to/plugins/mysql/PluginMySQL.txt")
```

Windows环境(假设安装在C盘上)：

```
loadPlugin("C:/path/to/mysql/PluginMySQL.txt")
```

### 1.2 编译安装

可使用以下方法编译MySQL插件，编译成功后通过以上方法导入。


#### 在Linux环境中编译安装

##### 环境准备

安装 [git](https://git-scm.com/) 和 [CMake](https://cmake.org/)。

Ubuntu用户只需要在命令行输入以下命令即可：
```bash
$ sudo apt-get install git cmake
```

然后通过更新git子模块来下载[mariadb-connector-c](https://github.com/MariaDB/mariadb-connector-c)的源文件。
```
$ git submodule update --init --recursive
```


安装cmake：
```
sudo apt-get install cmake
```

##### cmake编译

构建插件内容：
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../path/to/mysql_plugin/
make -j`nproc`
```

**注意:** 编译之前请确保libDolphinDB.so在gcc可搜索的路径中。可使用LD_LIBRARY_PATH指定其路径，或者直接将其拷贝到build目录下。

编译之后目录下会产生libPluginMySQL.so文件。


#### 在Windows环境中编译安装

##### 在Windows环境中需要使用CMake和MinGW编译

* 下载安装[MinGW](http://www.mingw.org/)。确保将bin目录添加到系统环境变量Path中。 

* 下载安装[cmake](https://cmake.org/)。

##### cmake编译

在编译开始之前，要将libDolphinDB.dll和包含curl头文件的文件夹拷贝到build文件夹内。

构建插件内容：
```
mkdir build                                                        # 新建build目录
cp path_to_libDolphinDB.dll/libDolphinDB.dll build                 # 拷贝 libDolphinDB.dll 到build目录下
cp -r curl build                                                   # 拷贝 curl 头文件到build目录下
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../path_to_mysql_plugin/ -G "MinGW Makefiles"
mingw32-make -j4
```

## 2.用户接口

> 请注意：使用插件函数前需使用`loadPlugin`函数导入插件。

### 2.1 mysql::connect

#### 语法

mysql::connect(host, port, user, password, db)

#### 参数

* host: MySQL服务器的地址，类型为string。
* port: MySQL服务器的端口，类型为int。
* user: MySQL服务器的用户名，类型为string。
* password: MySQL服务器的密码，类型为string。
* db: 要使用的数据库名称，类型为string。

#### 详情

与MySQL服务器建立一个连接。返回一个MySQL连接的句柄，用于`load`与`loadEx`等操作。

#### 例子

```
conn = mysql::connect(`127.0.0.1, 3306, `root, `root, `DolphinDB)
```

### 2.2 mysql::showTables

#### 语法

mysql::showTables(connection)

#### 参数

* connection: 通过`mysql::connect`获得的MySQL连接句柄。

#### 详情

列出建立MySQL连接时指定的数据库中包含的所有表。

#### 例子

```
conn = mysql::connect(`192.168.1.16, 3306, `root, `root, `DolphinDB)
mysql::showTables(conn)

output:
  Tables_in_DolphinDB
  -------------------
  US
```

### 2.3 mysql::extractSchema

#### 语法

mysql::extractSchema(connection, tableName)

#### 参数

* connection: 通过`mysql::connect`获得的MySQL连接句柄。
* tableName: MySQL表名，类型为string。

#### 详情

生成指定数据表的结构。

#### 例子

```
conn = mysql::connect(`192.168.1.16, 3306, `root, `root, `DolphinDB)
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

### 2.4 mysql::load

#### 语法

mysql::load(connection, table_or_query, [schema], [startRow], [rowNum])

#### 参数

* connection: 通过`mysql::connect`获得的MySQL连接句柄。
* table_or_query: 一张MySQL中表的名字，或者类似 select * from table limit 100 的合法MySQL查询语句，类型为string。
* schema: 包含列名和列的数据类型的表。如果我们想要改变由系统自动决定的列的数据类型，需要在schema表中修改数据类型，并且把它作为`load`函数的一个参数。
* startRow: 读取MySQL表的起始行数，若不指定，默认从数据集起始位置读取。若'table_or_query'是查询语句，则这个参数不起作用。
* rowNum: 读取MySQL表的行数，若不指定，默认读到数据集的结尾。若'table_or_query'是查询语句，则这个参数不起作用。

#### 详情

将MySQL表或者SQL查询结果导入DolphinDB中的内存表。支持的数据类型以及数据转化规则可见用户手册数据类型章节。

#### 例子

```
conn = mysql::connect(`192.168.1.18, 3306, `root, `root, `DolphinDB)
tb = mysql::load(conn, `US,,0,123456)
select count(*) from tb
```

```
conn = mysql::connect(`127.0.0.1, 3306, `root, `root, `DolphinDB)
tb = mysql::load(conn, "SELECT PERMNO FROM US LIMIT 123456")
select count(*) from tb
```

```
mysql::load(conn, "SELECT now(6)");
```

### 2.5 mysql::loadEx

#### 语法

mysql::loadEx(connection, dbHandle,tableName,partitionColumns,table_or_query,[schema],[startRow],[rowNum],[transform])

#### 参数

* connection: 通过`mysql::connect`获得的MySQL连接句柄。
* dbHandle 与 tableName: 若要将输入数据文件保存在分布式数据库中，需要指定数据库句柄和表名。
* partitionColumns: 字符串标量或向量，表示分区列。在组合分区中，partitionColumns是字符串向量。
* table_or_query: MySQL中表的名字，或者类似 select * from table limit 100 的合法MySQL查询语句，类型为string。
* schema: 包含列名和列的数据类型的表。若要修改由系统自动检测的列的数据类型，需要在schema表中修改数据类型，并且把它作为`load`函数的一个参数。
* startRow: 读取MySQL表的起始行数，若不指定，默认从数据集起始位置读取。若'table_or_query'是查询语句，则这个参数不起作用。
* rowNum: 读取MySQL表的行数，若不指定，默认读到数据集的结尾。若'table_or_query'是查询语句，则这个参数不起作用。
* transform: 导入到DolphinDB数据库前对MySQL表进行转换，例如替换列。

#### 详情

将MySQL中的表或查询结果转换为DolphinDB数据库的分布式表，然后将表的元数据加载到内存中。支持的数据类型,以及数据转化规则可见数据类型章节。

#### 例子

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


##### 直接原表导入
```
db = database("", RANGE, 0 50000 10000)
tb = mysql::loadEx(conn, db,`tb, `PERMNO, `US)
```

##### 通过SQL导入
```
db = database("", RANGE, 0 50000 10000)
tb = mysql::loadEx(conn, db,`tb, `PERMNO, "SELECT * FROM US LIMIT 100");
```

* 将数据导入DFS分布式文件系统中的分区表

##### 直接原表导入
```
db = database("dfs://US", RANGE, 0 50000 10000)
mysql::loadEx(conn, db,`tb, `PERMNO, `US)
tb = loadTable("dfs://US", `tb)
```

##### 通过SQL导入
```
db = database("dfs://US", RANGE, 0 50000 10000)
mysql::loadEx(conn, db,`tb, `PERMNO, "SELECT * FROM US LIMIT 1000");
tb = loadTable("dfs://US", `tb)
```

##### 导入前对MySQL表进行转换

```
db = database("dfs://US", RANGE, 0 50000 10000)
def replaceTable(mutable t){
	return t.replaceColumn!(`svalue,t[`savlue]-1)
}
t=mysql::loadEx(conn, db, "",`stockid, 'select  * from US where stockid<=1000000',,,,replaceTable)

```


* 将数据导入DFS分布式文件系统中的分区表


## 3. 支持的数据类型

### 3.1  整型

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

* DolphinDB中数值类型均为有符号类型。为了防止溢出，所有无符号类型会被转化为高一阶的有符号类型。例如，无符号CHAR转化为有符号SHORT，无符号SHORT转化为有符号INT，等等。64位无符号类型不予支持。
* DolphinDB不支持 unsigned long long 类型。若MySQL中的类型为 bigint unsigned, 可在`load`或者`loadEx`的schema参数中设置为DOUBLE或者FLOAT。
* DolphinDB中各类整形的最小值为NULL值，如 CHAR 的-128，SHORT的-32,768，INT的-2,147,483,648以及LONG的-9,223,372,036,854,775,808。

### 3.2 浮点数类型

| MySQL类型  | 对应的DolphinDB类型 |
| ---------- | :------------------ |
| double     | DOUBLE              |
| decimal    | DOUBLE              |
| newdecimal | DOUBLE              |
| float      | FLOAT               |

注：IEEE754浮点数类型皆为有符号数。

以上浮点类型皆可转化为DolphinDB中的数值相关类型(BOOL, CHAR, SHORT, INT, LONG, FLOAT, DOUBLE)。

### 3.3 时间类型

| MySQL类型 | 对应的DolphinDB类型 |
| --------- | :------------------ |
| date      | DATE                |
| time      | TIME                |
| datetime  | DATETIME            |
| timestamp | TIMESTAMP           |
| year      | INT                 |

以上类型皆可转化为DolphinDB中的时间相关类型(DATE, MONTH, TIME, MINUTE, SECOND, DATETIME, TIMESTAMP, NANOTIME, NANOTIMESTAMP)。

### 3.4 字符串类型

| MySQL类型           | 对应的DolphinDB类型 |
| ------------------- | :------------------ |
| char  (len <= 10)   | SYMBOL              |
| varchar (len <= 10) | SYMBOL              |
| char  (len > 10)    | STRING              |
| varchar (len > 10)  | STRING              |
| other string types  | STRING              |

长度不超过10的char和varchar将被转化为SYMBOL类型，其余转化为STRING类型。

string类型可以转化为转化为DolphinDB中的字符串相关类型(STRING, SYMBOL)。

### 3.5 枚举类型

| MySQL类型 | 对应的DolphinDB类型 |
| --------- | :------------------ |
| enum      | SYMBOL              |

enum类型可以转化为DolphinDB中的字符串相关类型(STRING, SYMBOL)，默认转化为SYMBOL类型。

## 4. 导入数据性能

### 4.1 硬件环境

* CPU: i7-7700 3.60GHZ
* 硬盘: SSD，读速为每秒460~500MB。

### 4.2  数据集导入性能

美国股票市场从1990年至2016年的每日数据，共50,591,907行，22列，6.5GB。 导入耗时160.5秒。
