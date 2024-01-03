# DolphinDB kdb+ Plugin

DolphinDB 的 kdb+ 插件支持通过 loadTable 和 loadFile 接口将 kdb+ 数据表和 Q 语言数据类型导入 DolphinDB 内存表。

kdb+ 插件目前支持版本：[relsease200](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/kdb/README_CN.md), [release130](https://github.com/dolphindb/DolphinDBPlugin/blob/release130/kdb/README_CN.md)。您当前查看的插件版本为 release200，请使用 DolphinDB 2.00.X 版本 server。若使用其它版本 server，请切换至相应插件分支。

使用插件前，推荐阅读 [README插件指南](../README_CN.md)

## 1 环境配置

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

## 2 插件加载与编译

### 2.1 预编译安装

可以直接使用已编译好的插件，在 `bin/` 文件夹中根据服务器操作系统选择适合的插件。

请注意插件的版本应与 DolphinDB 客户端版本相同，通过切换分支获取相应版本。

### 2.2 编译安装

用户也可以自行编译，方法如下：

#### Linux 系统编译

```linux shell
cd /path/to/plugins/kdb
mkdir build
cd build
cmake ..
make
```
注意：编译之前请确保 libDolphinDB.so 在 gcc 可搜索的路径中。可通过 ```export LD_LIBRARY_PATH=/path_to_dolphindb/:$LD_LIBRARY_PATH``` 指定其路径，或者直接将其拷贝到 build 目录下。

<!-- Windows 还没有发布，暂时隐藏
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
-->

### 2.3 加载插件
在 DolphinDB 客户端运行以下命令加载插件，需要将目录替换为 PluginKDB 文本文件所在的位置：

```DolphinDB shell
loadPlugin("/path/to/plugin/PluginKDB.txt")
```

## 3 用户接口

### 3.1 connect

**语法**

``` shell
connect(host, port, usernamePassword)
```

**参数**

- `host` 是要连接的 kdb+ 数据库所在的主机地址

- `port` 是要连接的 kdb+ 数据库所在的监听端口

- `usernamePassword` 是一个字符串，表示所要连接的 kdb+ 数据库的登录用户名和密码，格式为："username:password"

   该参数可以为空。若启动 kdb+ 时没有指定用户名和密码，则该参数为空或任意字符串。

**详情**

建立与 kdb+ 服务器之间的连接。返回一个连接句柄。

如果建立连接失败，则会抛出异常。包括以下三种情况：
1. 身份认证异常，用户名或密码错误
2. 连接端口异常，对应主机的端口不存在
3. 超时异常，建立连接超时

**示例：**
假设登录 kdb+ 数据库的用户名和密码（admin:123456）存储在 `../passwordfiles/usrs` 中，且 kdb+ 服务器和 DolphinDB server 都位于同一个主机上。

kdb+ 终端执行：
```
kdb shell：         q -p 5000 -U ../passwordfiles/usrs   // 注意 -U 一定需要大写
```
DolphinDB 客户端执行：

```
// 若开启 kdb+ 时指定了用户名和密码
handle = kdb::connect("127.0.0.1", 5000, "admin:123456")

// 若开启 kdb+ 时未指定用户名和密码
handle = kdb::connect("127.0.0.1", 5000)
```

### 3.2 loadTable

**语法**

``` shell
loadTable(handle, tablePath, symPath)
```

**参数**

- `handle` 是 `connect` 返回的连接句柄

- `tablePath` 是一个字符串，表示需要读取的表文件路径。如果是 splayed table, partitioned table 或 segmented table，则指定为表文件目录；如果是 single object，则指定为表文件

- `symPath` 是一个字符串，表示表文件对应的 sym 文件路径

  该参数可以为空，此时必须保证表内不包含被枚举的 symbol 列。

注意：路径中建议使用'/'分隔。

**详情**

连接 kdb+ 数据库，通过 kdb+ 数据库加载数据，再将数据导入 DolphinDB 内存表。

> 在 kdb+ 数据库中，symbol 类型可以通过枚举存入 sym 文件，在表中使用 int 类型代替字符串进行存储，以减少字符串重复存储所占的空间，因此如果需要读取的表包含了枚举的 symbol 列，则需要读入 sym 文件才能正确读取表内的 symbol 列。

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

**语法**

``` shell
loadFile(tablePath, symPath)
```

**参数**

- `tablePath` 是一个字符串，表示需要读取的表文件路径。只能是 splayed table, partitioned table 或 segmented table 的表文件目录

- `symPath` 是表文件对应的 sym 文件路径

  该参数可以为空，此时必须保证表内不包含被枚举的 symbol 列。

注意：路径中建议使用'/'分隔。

**详情**

直接读取磁盘上的 kdb+ 数据文件，将其存入 DolphinDB 内存表。

> 在 kdb+ 数据库中，symbol 类型可以通过枚举存入 sym 文件，在表中使用 int 类型代替字符串进行存储，以减少字符串重复存储所占的空间，因此如果需要读取的表包含了被枚举的 symbol 列，则需要读入 sym 文件才能正确读取表内的 symbol 列。

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

**语法**

``` shell
close(handle)
```

**参数**

- `handle` 是连接句柄

**详情**

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

1. 待导入的表中不应包含除 char 类型以外的 nested column。
2. loadTable 指定的加载路径应为单个表文件，或表路径（表为拓展表、分区表或分段表时）。

### 4.2 通过 loadFile 导入

操作顺序：loadFile()

注意事项：

1. 无法读取单个表（single object）。
2. 只能读取采用 gzip 压缩方法持久化的数据。
3. 待导入的表中不应包含除 char 类型以外的 nested column。
4. loadFile 指定的加载路径分区下的表路径。
5. 如果导入的表中存在 sorted、unique 、partitioned 、true index 等列属性，建议使用loadTable，减少出错的可能

## 5 kdb+ 各类表文件加载方式说明

分别说明 kdb+ 四种表文件的导入方式：

- 单张表（single object）

  只能使用 `loadTable()` 导入。

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

  `loadTable()` 或 `loadFile()`

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

**kdb+ 基本类型**

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

**kdb+ 其他类型**

由于 kdb+ 中常常使用 char nested list 存储字符串类型数据，因此 dolphindb kdb+ 插件支持了 char nested list 到 dolphindb string 类型的转换。

其他复杂类型如 anymap、dictionary 以及其他基本类型的 nested list 无法支持到 dolphindb 内置类型的转换。

| kdb+               | DolphinDB     | 字节长度 | 备注                                                                              |
| ------------------ | ------------- | -------- | -------------------------------------------------------------------------------- |
| char nested list   | STRING        | 不超过65535 |                                                                                  |


## 7. 测试方法

为方便开发者进行自测，kdb 插件自2.00.11版本起提供自行测试的数据、脚本。以下将介绍测试文件和测试方法。

**测试文件的组成**

测试必需的文件在本插件文件夹下 test 文件夹中。以下为详细说明：

``` shell
kdb
├── lib
├── src
├── test                  // 插件目录下 test 文件夹，包含所有测试需要的文件
│   ├── data              // 测试时需要用到的一些数据文件，包括 kdb 持久化文件和一些验证文件
│   │   └── ...
│   ├── setup             // 配置参数存放文件夹
│   │   └── settings.txt  // 具体存放配置参数的 txt 文件，在运行测试命令前需要修改
│   └── test_kdb.txt      // dolphindb kdb 插件测试脚本文件
├── CMakeLists.txt
└── ...
```

**测试方法**

1. 修改 `test/setup/settings.txt` 文件下的参数。以下是 kdb 测试脚本运行的一些必要配置：
    - DATA_DIR： kdb 测试数据，在 `/test/data` 目录下提供，需修改测试时所放目录前缀。
    - HOST：启动的 kdb+ 数据库的 IP 地址。
    - PORT：启动的 kdb+ 数据库设置的监听端口。
    - usernamePassword：连接启动的 kdb+ 数据库的用户名和密码。
    - pluginTxtPath：编译后的插件 txt 文件路径，需在同目录下有对应的插件动态库文件。

2. 启动一个可以连接的 kdb+ 数据库，启动方法参考 [connecting-to-a-kdb-process](https://code.kx.com/q/wp/capi/#connecting-to-a-kdb-process)。

3. 运行 dolphindb。

4. 运行以下 dolphindb 脚本：
    ``` dolphindb
    login(`admin,`123456);
    test("<plugin_src_path>/test/test_kdb.txt");
    ```
    测试结果将会显示在屏幕上。其他 test 函数的使用方法与配置可以参考 DolphinDB 的 `test` 函数。

# Release Notes:

## v2.00.10

### 新增功能

1. 新增对 char 类型的 nested list 列读取的支持。

### bug修复

1. 修复loadTable不指定symPath会crash的问题。
2. 修复sym文件的文件名不为sym时，不生效的问题。
3. 修复关闭kdb进程再执行kdb::loadTable()会造成dolphindb宕机的问题。

## v2.00.11

### 新增功能

1. 提供了开发者自测数据与脚本。
