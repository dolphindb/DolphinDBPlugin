# DolphinDB formatArrow Plugin

Apache Arrow 将列式数据结构的优势与内存计算相结合。DolphinDB 提供的 formatArrow 支持 API 与 DolphinDB 进行数据传输时使用 Arrow 数据格式，并自动进行数据类型转换。

Arrow 插件目前支持版本： release200。您当前查看的插件版本为 release200，请使用 DolphinDB 2.00.X 版本 server。若使用其他版本 server，请切换至相应插件分支。

默认使用 Arrow 版本为 9.0.0。

## 1 安装插件

### 1.1 Linux 编译

#### 初始化环境配置

(1) 编译 Arrow 开发包：

```
git clone https://github.com/apache/arrow.git
cd arrow/cpp
mkdir build
cd build
cmake .. 
make -j
```

(2) 编译完成后，拷贝以下文件到插件文件夹中的相应目录：

| **文件**                                                   | **目标目录** |
| ------------------------------------------------------------ | -------------------- |
| arrow/cpp/src/arrow                                          | ./include            |
| arrow/cpp/build/release/libarrow.so.900 | ./build          |

或者直接使用 lib 目录下提供的预编译好的 libarrow.so.900

#### 编译插件

```
cd /path/to/plugins/formatArrow
mkdir build
cd build
cmake ..
make
```

注意：编译之前请确保 libDolphinDB.so 和 libarrow.so.900 在 gcc 可搜索的路径中。可使用 LD_LIBRARY_PATH 指定其路径，或者直接将其拷贝到 build 目录下。

#### DolphinDB 加载插件

```
loadFormatPlugin("/path/to/plugin/PluginFormatArrow.txt")
```

## 2 使用示例

#### DolphinDB server

```
loadFormatPlugin("path/to/formatArrow/PluginFormatArrow.txt")
```

#### Python API：

```
import dolphindb as ddb
import dolphindb.settings as keys
s = ddb.session("192.168.1.113", 8848, "admin", "123456", protocol=keys.PROTOCOL_ARROW)
pat = s.run("table(1..10 as a)")

print(pat)
-------------------------------------------
pyarrow.Table
a: int32
----
a: [[1,2,3,4,5,6,7,8,9,10]]
```

注意：现版本中 DolphinDB 服务器不支持使用 Arrow 协议时开启压缩。

## 3 支持的数据类型

### 3.1 DolphinDB -> Arrow

DolphinDB 向 API 传输数据时，Arrow 与 DolphinDB 数据类型转换关系如下：

| DolphinDB       | Arrow                   |
| --------------- | :---------------------- |
| BOOL            | boolean                 |
| CHAR            | int8                    |
| SHORT           | int16                   |
| INT             | int32                   |
| LONG            | int64                   |
| DATE            | date32                  |
| MONTH           | date32                  |
| TIME            | time32(ms)              |
| MINUTE          | time32(s)               |
| SECOND          | time32(s)               |
| DATETIME        | timestamp(s)            |
| TIMESTAMP       | timestamp(ms)           |
| NANOTIME        | time64(ns)              |
| NANOTIMESTAMP   | timestamp(ns)           |
| DATEHOUR        | timestamp(s)            |
| FLOAT           | float32                 |
| DOUBLE          | float64                 |
| SYMBOL          | dictionary(int32, utf8) |
| STRING          | utf8                    |
| IPADDR          | utf8                    |
| UUID            | fixed_size_binary(16)   |
| INT128          | fixed_size_binary(16)   |
| BLOB            | large_binary            |
| DECIMAL32(X)    | decimal128(38, X)       |
| DECIMAL64(X)    | decimal128(38, X)       |

同时支持以上除了 DECIMAL 外的 ArrayVector 类型。

注意：使用 Arrow 数据格式从 DolphinDB Server 获取数据后转换为 pandas.DataFrame 时，DolphinDB 的 NANOTIME 数据类型对应 Arrow 的 time64 数据类型，因此要求进行转换的数值必须为1000的倍数，否则会提示`Value xxxxxxx has non-zero nanoseconds`。