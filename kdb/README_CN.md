# DolphinDB KDB+ Plugin

DolphinDB的KDB+数据导入插件，提供两种方式，可以将用户存储在磁盘上的KDB+数据导入DolphinDB,并支持全部数据类型。

#### 方法 1 连接kdb+api导入

连接kdb+数据库，在kdb+数据库中加载数据并通过kdb的c-api导入DolphinDB数据库

操作顺序：connect() -> loadTable() -> close()

不适用场景：

1. 表中有 nested column
2. tablePath是文件夹情况下, 数据文件不直接存在在tablePath文件夹下

#### 方法 2 loadFile

直接读取磁盘上的kdb+文件, 导入DolphinDB数据库

操作顺序：loadFile()

不适用场景：

1. 单个表
2. 采用1(q IPC) 3(snappy) 4(L4Z)种压缩方法持久化的数据
3. 表中有 nested column
4. 数据文件不直接存在在tablePath下



### 各类表文件加载方式

在满足上述条件的情况下，KDB+四种表文件加载方式说明

- 单张表 object
  
  使用**第一种方法**，连接kdb+api导入
  
- 拓展表 splayed table

  **两种方法都可以加载**

  tablePath指定为存有数据文件的文件夹，symPath指定为存储symbol枚举数据的文件路径

  如果未压缩、使用gzip压缩，则推荐使用第二种方法。执行效率更高

- 分区表 partition table

  本插件无法通过指定根目录，加载整个分区表、数据库

  但是可以将tablePath指定为每个分区下的表格，分别加载其中数据，然后通过脚本在DolphinDB中组成一张完整的表格

- 分段表 segment table

  同partition table，可以将各段的各分区中的表读入，然后通过脚本在DolphinDB中组成一张完整的表格



## 1 预编译安装

### 1.1 初始环境配置

#### KDB+

如果需要通过第一种方式从kdb+中导入数据，则需要已安装kdb+数据库

官网申请64位社区版license：https://kx.com/developers/download-licenses/ 

#### zlib

插件中解压缩功能依赖zlib

Ubuntu：
```linux shell
sudo apt install zlib1g
```

CentOS：
```linux shell
yum install -y zlib zlib-devel
```

windows

http://www.winimage.com/zLibDll/

**Windows环境下kdb+本体使用zlib过程中可能存在问题，因此建议connect()连接linux环境下的kdb+运行本插件**

## 2 编译安装

### 2.1 编译安装

#### linux

```linux shell
cd /path/to/plugins/kdb
mkdir build
cd build
cmake ..
make
```

#### Windows：
- 下载安装[MinGW](http://www.mingw.org/)。确保将bin目录添加到系统环境变量Path中。
- 下载安装[cmake](https://cmake.org/)。

在编译开始之前，要将libDolphinDB.dll拷贝到build文件夹。

```
mkdir build                                                        # 新建build目录
cp <ServerDir>/libDolphinDB.dll build                 # 拷贝 libDolphinDB.dll 到build目录下
cd build
cmake  ../ -G "MinGW Makefiles"
mingw32-make -j4
```

### 2.2 Dolphindb加载插件

```DolphinDB shell
loadPlugin("/path/to/plugin/PluginKDB.txt")
```

## 3 用户接口

### 3.1 connect

#### 语法

``` shell
connect(host, port, usernamePassword)
```

#### 参数

- 'host'是要连接的kdb+的主机地址

- 'port'是要连接的kdb+的监听端口

- 'usernamePassword'是所要连接的kdb+数据库 "用户名" + ":" + "密码"

   可以为空，如果kdb在启动时没有指定用户名和密码。不填写该参数或者填写任意字符串都可以进行连接

#### 详情

此方法通过建立与kdb+之间的连接，先将数据读入kdb+数据库，以此从kdb+中读取表格数据存入DolphinDB

**此方法需要预先启动KDB+, 并设置相同的端口、用户名、密码**

示例：
假设KDB用户名密码 admin:123456 存储在 ../passwordfiles/usrs中，同时两者都本地 

```
KDB shell：         q -p 5000 -U ../passwordfiles/usrs   // 注意-U一定需要大写
DolphinDB shell：   handle = kdb::connect("127.0.0.1", 5000, "admin:123456")
```

返回值是一个建立与kdb+数据库连接之后的句柄，如果建立连接失败则会抛出异常。异常分为三种：
1. 身份认证异常，用户名密码错误
2. 连接端口异常，对应主机端口不存在
3. 超时异常，建立连接超时

#### 例子

```
// 开启kdb时指定了用户名密码
handle = kdb::connect("127.0.0.1", 5000, "admin:123456")

// 开启kdb时未指定用户密码
handle = kdb::connect("127.0.0.1", 5000)
```

### 3.2 loadTable

#### 语法

``` shell
loadTable(handle, tablePath, symPath)
```

#### 参数

- 'handle'是用于读取数据的handle

- 'tablePath'是需要读取表格文件的路径，可以是文件夹，也可以是单个表文件

- 'symPath'是对应表格文件的sym文件路径
  
  可以为空，但是必须保证数据内没有已经枚举的symbol类型数据

路径名称建议采取使用'/'隔开的方式。

#### 详情

此方法是通过连接kdb+数据库，先在kdb+数据库中加载数据，再将数据导入DolphinDB数据库。因此，必须传入一个handle才能读取数据。

返回值是一个table变量，可以直接存入DolphinDB中

*在kdb+数据库中，symbol类型可以通过枚举存入sym文件中，在表格中只使用int存储，减少字符串重复存储所占空间，因此如果需要读取的表格有对应的sym枚举，则需要读入sym文件才能读取表格内的symbol数据*
*同时，由于通过在kdb+中先加载的方式读取数据，因此读取一个含有symbol数据的拓展表时，不管指不指定sym文件，都能得到结果，但是如果不指定sym表，symbol数据列只会读出long long类型数据。因此请留意所要读取的表是否含有symbol列*

#### 例子

```
// 表格存在symbol类型，并进行了枚举
DATA_DIR="/path/to/data/kdb_sample"
Txns = kdb::loadTable(handle, DATA_DIR + "/2022.06.17/Txns", DATA_DIR + "/sym")

// 拓展表中不存在symbol类型，或者单张表中symbol未进行枚举
DATA_DIR="/path/to/data/kdb_sample"
Txns = kdb::loadTable(handle, DATA_DIR + "/2022.06.17/Txns", DATA_DIR)
```

### 3.3 loadFile

#### 语法

``` shell
loadFile(tablePath, symPath)
```

#### 参数

- 'tablePath'是需要读取表格文件的路径，只能是文件夹，不能是单张表文件

- 'symPath'是对应表格文件的sym文件路径
  
  可以为空，但是必须保证数据内没有已经枚举的symbol类型数据

路径名称建议采取使用'/'隔开的方式。

#### 详情

此方法是通过直接读取在磁盘上的kdb+数据文件，存入DolphinDB。

返回值是一个table变量，可以直接存入DolphinDB中。

*在kdb+数据库中，symbol类型可以通过枚举存入sym文件中，在表格中只使用int存储，减少字符串重复存储所占空间，因此如果需要读取的表格有对应的sym枚举，则需要读入sym文件才能读取表格内的symbol数据。*

#### 例子

```
//表格存在symbol类型，并进行了枚举
DATA_DIR="/path/to/data/kdb_sample"
Txns = kdb::loadFile(handle, DATA_DIR + "/2022.06.17/Txns", DATA_DIR + "/sym")


表格中不存在symbol类型
DATA_DIR="/path/to/data/kdb_sample"
Txns = kdb::loadFile(handle, DATA_DIR + "/2022.06.17/Txns", DATA_DIR)
```

### 3.4 close 

#### 语法

``` shell
close(handle)
```

#### 参数

- 'handle'是需要关闭的handle

#### 详情

关闭与KDB+建立连接的handle
#### 例子

```
kdb::close(handle)
```

## Appendix

``` DolphinDB shell
loadPlugin("/home/slshen/DolphinDBPlugin/kdb/build/PluginKDB.txt")
go
// 连接kdb数据库
handle = kdb::connect("127.0.0.1", 5000, "admin:123456")

// 指定文件路径
DATA_DIR="/home/slshen/KDB/data/kdb_sample"
TEST_DATA_DIR="/home/slshen/KDB/data"

// 通过kdb+ api，加载数据到DolphinDB
Daily = kdb::loadTable(handle, DATA_DIR + "/2022.06.17/Daily/", DATA_DIR + "/sym")
Minute = kdb::loadTable(handle, DATA_DIR + "/2022.06.17/Minute", DATA_DIR + "/sym")
Ticks = kdb::loadTable(handle, DATA_DIR + "/2022.06.17/Ticks/", DATA_DIR + "/sym")
Orders = kdb::loadTable(handle, DATA_DIR + "/2022.06.17/Orders", DATA_DIR + "/sym")
Syms = kdb::loadTable(handle, DATA_DIR + "/2022.06.17/Syms/", DATA_DIR + "/sym")
Txns = kdb::loadTable(handle, DATA_DIR + "/2022.06.17/Txns", DATA_DIR + "/sym")
kdb::close(handle)

// 直接读磁盘文件，加载数据到DolphinDB
Daily2 = kdb::loadFile(DATA_DIR + "/2022.06.17/Daily", DATA_DIR + "/sym")
Minute2= kdb::loadFile(DATA_DIR + "/2022.06.17/Minute/", DATA_DIR + "/sym")
Ticks2 = kdb::loadFile(DATA_DIR + "/2022.06.17/Ticks/", DATA_DIR + "/sym")
Orders2 = kdb::loadFile(DATA_DIR + "/2022.06.17/Orders/", DATA_DIR + "/sym")
Syms2 = kdb::loadFile(DATA_DIR + "/2022.06.17/Syms/", DATA_DIR + "/sym")
Txns2 = kdb::loadFile(DATA_DIR + "/2022.06.17/Txns/", DATA_DIR + "/sym")
```
