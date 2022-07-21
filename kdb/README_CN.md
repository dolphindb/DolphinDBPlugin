# DolphinDB kdb+ Plugin

通过 DolphinDB 的 kdb+ 数据导入插件，可以将用户存储在磁盘上的 kdb+ 数据表导入 DolphinDB 数据库内存表。该插件支持导入所有 Q 语言的数据类型。目前支持两种导入模式：通过 loadTable 导入和通过 loadFile 导入。

## 1 预编译安装

### 1.1 初始化环境配置

#### 安装 kdb+ 数据库

如果通过第一种方式从 kdb+ 中导入数据，则需要先安装 kdb+ 数据库。

官网申请64位社区版 license：https://kx.com/developers/download-licenses/ 

#### 安装 zlib 包

插件中解压缩功能依赖 zlib

Ubuntu 系统：
```linux shell
sudo apt install zlib1g
```

CentOS 系统：
```linux shell
yum install -y zlib zlib-devel
```

Windows 系统：

http://www.winimage.com/zLibDll/

## 2 插件编译与加载

### 2.1 编译安装

#### Linux 系统编译

```linux shell
cd /path/to/plugins/kdb
mkdir build
cd build
cmake ..
make
```

#### Windows 系统编译
- 下载安装[MinGW](http://www.mingw.org/)。确保将 bin 目录添加到系统环境变量 Path 中。
- 下载安装[cmake](https://cmake.org/)。

编译开始之前，需要将 libDolphinDB.dll 拷贝至 build 文件夹。

```
mkdir build                                           # 新建 build 目录
cp <ServerDir>/libDolphinDB.dll build                 # 拷贝 libDolphinDB.dll 到 build 目录下
cd build
cmake  ../ -G "MinGW Makefiles"
mingw32-make -j4
```

### 2.2 DolphinDB 加载插件

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

- 'host' 是要连接的 kdb+ 数据库所在的主机地址

- 'port' 是要连接的 kdb+ 数据库所在的监听端口

- 'usernamePassword' 是一个字符串，表示所要连接的 kdb+ 数据库的登录用户名和密码，格式为："username:password"

   该参数可以为空。若启动 kdb+ 时没有指定用户名和密码，则该参数为空或任意字符串。

#### 详情

建立与 kdb+ 服务器之间的连接。返回一个连接句柄。

如果建立连接失败，则会抛出异常。包括以下三种情况：
1. 身份认证异常，用户名或密码错误
2. 连接端口异常，对应主机的端口不存在
3. 超时异常，建立连接超时

**示例：**  
假设登录 kdb+ 数据库的用户名和密码（admin:123456）存储在 ../passwordfiles/usrs 中，且 kdb+ 服务器和 DolphinDB server 都位于同一个主机上。 

```
kdb shell：         q -p 5000 -U ../passwordfiles/usrs   // 注意 -U 一定需要大写
DolphinDB shell：   handle = kdb::connect("127.0.0.1", 5000, "admin:123456")
```

**例子**

```
// 若开启 kdb+ 时指定了用户名和密码
handle = kdb::connect("127.0.0.1", 5000, "admin:123456")

// 若开启 kdb+ 时未指定用户名和密码
handle = kdb::connect("127.0.0.1", 5000)
```

### 3.2 loadTable

#### 语法

``` shell
loadTable(handle, tablePath, symPath)
```

#### 参数

- 'handle' 是 `connect` 返回的连接句柄

- 'tablePath' 是一个字符串，表示需要读取的表文件路径。如果是 splayed table, partitioned table 或 segmented table，则指定为表文件目录；如果是 single object，则指定为表文件

- 'symPath' 是一个字符串，表示表文件对应的 sym 文件路径
  
  该参数可以为空，此时必须保证表内不包含被枚举的 symbol 列。

路径建议使用'/'分隔。

#### 详情

连接 kdb+ 数据库，通过 kdb+ 数据库加载数据，再将数据导入 DolphinDB 内存表。

>在 kdb+ 数据库中，symbol 类型可以通过枚举存入 sym 文件，在表中使用 int 类型代替字符串进行存储，以减少字符串重复存储所占的空间，因此如果需要读取的表包含了枚举的 symbol 列，则需要读入 sym 文件才能正确读取表内的 symbol 列。
>由于通过在 kdb+ 中先加载数据的方式进行导入，当读取一个将 symbol 列枚举后的表时，虽然指定或不指定 sym 文件，都能得到结果，但不指定 sym 文件时，kdb+ 会将 symbol 列读取为 long long 类型的数据。因此请留意所要读取的表是否含有被枚举的 symbol 列。

**例子**

```
// 表中存在被枚举的 symbol 列
DATA_DIR="/path/to/data/kdb_sample"
Txns = kdb::loadTable(handle, DATA_DIR + "/2022.06.17/Txns", DATA_DIR + "/sym")

// 拓展表中不存在 symbol 类型，或者单张表中的 symbol 列未进行枚举
DATA_DIR="/path/to/data/kdb_sample"
Txns = kdb::loadTable(handle, DATA_DIR + "/2022.06.17/Txns", DATA_DIR)
```

### 3.3 loadFile

#### 语法

``` shell
loadFile(tablePath, symPath)
```

#### 参数

- 'tablePath' 是一个字符串，表示需要读取的表文件路径。只能是 splayed table, partitioned table 或 segmented table 的表文件目录

- 'symPath' 是表文件对应的 sym 文件路径
  
  该参数可以为空，此时必须保证表内不包含被枚举的 symbol 列。

路径建议使用'/'分隔。

#### 详情

直接读取磁盘上的 kdb+ 数据文件，将其存入 DolphinDB 内存表。

>在 kdb+ 数据库中，symbol 类型可以通过枚举存入 sym 文件，在表中使用 int 类型代替字符串进行存储，以减少字符串重复存储所占的空间，因此如果需要读取的表包含了被枚举的 symbol 列，则需要读入 sym 文件才能正确读取表内的 symbol 列。

**例子**

```
//表中存在 symbol 类型，并进行了枚举
DATA_DIR="/path/to/data/kdb_sample"
Txns = kdb::loadFile(handle, DATA_DIR + "/2022.06.17/Txns", DATA_DIR + "/sym")


表中不存在 symbol 类型
DATA_DIR="/path/to/data/kdb_sample"
Txns = kdb::loadFile(handle, DATA_DIR + "/2022.06.17/Txns", DATA_DIR)
```

### 3.4 close 

#### 语法

``` shell
close(handle)
```

#### 参数

- 'handle' 是连接句柄

#### 详情

关闭与 kdb+ 服务器建立的连接。

**例子**

```
kdb::close(handle)
```

### 3.5 完整示例

``` DolphinDB shell
loadPlugin("/home/DolphinDBPlugin/kdb/build/PluginKDB.txt")
go
// 连接 kdb+ 数据库
handle = kdb::connect("127.0.0.1", 5000, "admin:123456")

// 指定文件路径
DATA_DIR="/home/kdb/data/kdb_sample"
TEST_DATA_DIR="/home/kdb/data"

// 通过 loadTable，加载数据到 DolphinDB
Daily = kdb::loadTable(handle, DATA_DIR + "/2022.06.17/Daily/", DATA_DIR + "/sym")
Minute = kdb::loadTable(handle, DATA_DIR + "/2022.06.17/Minute", DATA_DIR + "/sym")
Ticks = kdb::loadTable(handle, DATA_DIR + "/2022.06.17/Ticks/", DATA_DIR + "/sym")
Orders = kdb::loadTable(handle, DATA_DIR + "/2022.06.17/Orders", DATA_DIR + "/sym")
Syms = kdb::loadTable(handle, DATA_DIR + "/2022.06.17/Syms/", DATA_DIR + "/sym")
Txns = kdb::loadTable(handle, DATA_DIR + "/2022.06.17/Txns", DATA_DIR + "/sym")
kdb::close(handle)

// 直接读磁盘文件，加载数据到 DolphinDB
Daily2 = kdb::loadFile(DATA_DIR + "/2022.06.17/Daily", DATA_DIR + "/sym")
Minute2= kdb::loadFile(DATA_DIR + "/2022.06.17/Minute/", DATA_DIR + "/sym")
Ticks2 = kdb::loadFile(DATA_DIR + "/2022.06.17/Ticks/", DATA_DIR + "/sym")
Orders2 = kdb::loadFile(DATA_DIR + "/2022.06.17/Orders/", DATA_DIR + "/sym")
Syms2 = kdb::loadFile(DATA_DIR + "/2022.06.17/Syms/", DATA_DIR + "/sym")
Txns2 = kdb::loadFile(DATA_DIR + "/2022.06.17/Txns/", DATA_DIR + "/sym")
```

## 4 导入方法说明
### 4.1 通过 loadTable 导入

操作顺序：connect() -> loadTable() -> close()

注意事项：

1. 待导入的表中不应包含 nested column。
2. loadTable 指定的加载路径应为单个表文件，或分区下的表路径（当表为分区表或分段表时）。

### 4.2 通过 loadFile 导入

操作顺序：loadFile()

注意事项：

1. 无法读取单个表（single object）。
2. 无法读取采用 q IPC (1), snappy (3), L4Z (4) 这三类压缩方法持久化的数据。
3. 待导入的表中不应包含 nested column。
4. loadFile 指定的加载路径分区下的表路径。

## 5 kdb+ 各类表文件加载方式说明

在满足导入方法说明里所列条件的情况下，分别说明 kdb+ 四种表文件的导入方式：

- 单张表（single object）
  
  只能使用**第一种方法**导入。

  举例：
  ``` 
  目录结构：
  path/to/data
  ├── sym
  └── table_name
  ```
  
  ```
  handle = kdb::connect("127.0.0.1", 5000, "username:password");
  table = kdb::loadTable(handle, "path/to/data/table_name", "path/to/data/sym");
  ```
  
- 拓展表（splayed table）

  **两种方法都可以导入**

  如果未压缩或使用 gzip 压缩，则推荐使用第二种方法，导入效率会更高。

  举例：
  ``` 
  目录结构：
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

- 分区表（partitioned table）

  本插件无法通过指定根目录，加载整个分区表、数据库。

  但是可以将 tablePath 指定为每个分区下的表，分别加载其中的数据，然后通过脚本在 DolphinDB 中组成一张完整的表。

  举例：

  ``` 
  目录结构：
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
// 获取文件夹下的所有文件信息
fileRes=files("path/to/data");

// 删除 sym 条目，避免影响数据文件夹读取
delete from fileRes where filename='sym';
name='table_name';
files = exec filename from fileRes;

// 新建数据表，指定 schema
table=table(10:0,`p`ti`date, [SECOND,DOUBLE,DATE])

// 读取各个分区数据
for (file in files) {
        t = kdb::loadFile("path/to/data" +'/' + file +'/' + tablename + '/');

        // 添加分区名所代表的数据
        addColumn(t, ["date"],[DATE])
        length=count(t)
        newCol=take(date(file), length)
        replaceColumn!(t, "date", newCol)

        // 追加数据到 table 中
        table.append!(t);
}
  ```

- 分段表（segmented table）

  同 partitioned table，可以将各分段的各分区中的表读入，然后通过脚本在 DolphinDB 中组成一张完整的表。

## 6 kdb+ 与 DolphinDB 数据类型转换表

| kdb+      | DolphinDB     | 字节长度 | 备注                                                                             |
| --------- | ------------- | -------- | -------------------------------------------------------------------------------- |
| boolean   | BOOL          | 1        |                                                                                  |
| guid      | UUID          | 16       |                                                                                  |
| byte      | CHAR          | 1        | DolphinDB 中没有独立的 byte 类型，转换为长度相同的 CHAR                           |
| short     | SHORT         | 2        |                                                                                  |
| int       | INT           | 4        |                                                                                  |
| long      | LONG          | 8        |                                                                                  |
| real      | FLOAT         | 4        |                                                                                  |
| float     | DOUBLE        | 8        |                                                                                  |
| char      | CHAR          | 1        | kdb+ 中 char 空值（""）当空格（" "）处理，因此不会转换为 DolphinDB CHAR 类型的空值 |
| symbol    | SYMBOL        | 4        |                                                                                  |
| timestamp | NANOTIMESTAMP | 8        |                                                                                  |
| month     | MONTH         | 4        |                                                                                  |
| data      | DATE          | 4        |                                                                                  |
| datetime  | TIMESTAMP     | 8        |                                                                                  |
| timespan  | NANOTIME      | 8        |                                                                                  |
| minute    | MINUTE        | 4        |                                                                                  |
| second    | SECOND        | 4        |                                                                                  |
| time      | TIME          | 4        |                                                                                  |