# DolphinDB EncoderDecoder Plugin

DolphinDB 提供 EncoderDecoder 插件用于高效解析、转换数据。

该插件可以高效地将表格中的 json 数据转换为 DolphinDB 表格数据。

EncoderDecoder 插件目前只支持 Linux 版本。

## 1. 安装插件

### 1.1. 在插件市场安装插件

#### 版本要求

DolphinDB Server: 2.00.10 及更高版本

#### 安装步骤

在 DolphinDB 客户端中使用 listRemotePlugins 命令查看插件仓库中的插件信息。

```DolphinDB
login("admin", "123456")
listRemotePlugins(, "http://plugins.dolphindb.cn/plugins/")
```

使用 installPlugin 命令完成插件安装。

```DolphinDB
installPlugin("EncoderDecoder")
```

返回：<path_to_EncoderDecoder_plugin>/PluginEncoderDecoder.txt

使用 loadPlugin 命令加载插件（即上一步返回的.txt文件）。

```DolphinDB
loadPlugin("<path_to_EncoderDecoder_plugin>/PluginEncoderDecoder.txt")
```

### 1.2. Linux 编译安装

#### 1.2.1. 安装 Protocol Buffers

下载 [Protol Buffers v3.17.1](https://github.com/protocolbuffers/protobuf/releases/download/v3.17.1/protobuf-cpp-3.17.1.tar.gz).

``` shell
tar -xzvf protobuf-cpp-3.17.1.tar.gz
cd protobuf-3.17.1
mkdir -p cmake/build/release
cd cmake/build/release
cmake ../.. -DCMAKE_BUILD_TYPE=Release -Dprotobuf_BUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON -Dprotobuf_BUILD_TESTS=OFF
make
make check

cp -r src/google/ <path_to_plugins>/EncoderDecoder/include/
cp libprotobuf.a <path_to_plugins>/EncoderDecoder/lib/
```

#### 1.2.2. 编译插件

``` shell
cd <path_to_plugins>/EncoderDecoder
mkdir build
cd build
cmake ..
make
```

注意：编译之前请确保 libDolphinDB.so 在 gcc 可搜索的路径中。可使用 LD_LIBRARY_PATH 指定其路径，或者直接将其拷贝到 build 目录下。

#### 1.2.3. DolphinDB 加载插件

```DolphinDB
loadPlugin("<path_to_EncoderDecoder_plugin>/PluginEncoderDecoder.txt")
```

## 2. 用户接口

EncoderDecoder 插件的一般使用形式：

- 通过插件函数，获取 coder 对象

  每个 coder 对象有自己转换的函数，并且有规定格式的输入与输出，输出端可以是表格、方法。
 coder 对象具有自己的方法，可以在脚本中直接调用它的方法。

- 调用 coder 对象的 parse 或者 parseAndHandle 方法，往 coder 对象中放入数据进行解析，解析结果会输出到 coder 对象的输出端。

### 2.1. coder 对象的基本使用

每当需要解析数据时，通过插件函数获取 coder 对象

以 jsonDecoder 为例：

``` DolphinDB
colNames = ["cint","clong","cdouble","cfloat","ctimestamp","avint"]
colTypes = [INT,LONG,DOUBLE,FLOAT,TIMESTAMP,INT[]]
outputTable = table(100:0,colNames,colTypes)
coder = EncoderDecoder::jsonDecoder(colNames, colTypes, outputTable)
```

具体不同数据格式的 coder 对象 如何设定 转换函数、输入、输出 详见各类 coder 的说明。目前插件仅支持一种 jsonDecoder。

#### 2.1.1. coder.parseAndHandle

该方法将输入的表数据 obj 进行批量 解析、转换 处理，并输出到创建 coder 时传入的输出 handler 中。

如果该 coder 未指定输出 handler，则无法执行该函数，执行时会报错 `To use method parseAndHandle, the handler of the coder object must be specified`.

**语法**

``` DolphinDB
coder.parseAndHandle(obj)
```

**参数**

- `obj`：具体输入参数要求见各类 coder 的说明，通常输入参数可以为 scalar，vector 或者 内存表。

**返回值**

布尔类型

解析、转换 成功返回 true，得到的 table 会输出到 coder 的输出端。解析失败返回 false，输出相关解析报错信息到日志

#### 2.1.2. coder.parse

该方法将输入的表数据 obj 进行批量 解析/编码 处理，直接返回结果。

**语法**

``` DolphinDB
coder.parse(obj)
```

**参数**

- `obj`：具体输入参数要求见各类 coder 的说明，通常输入参数可以为 scalar，vector 或者 内存表。

**返回值**

内存表

解析成功返回一个内存表，解析失败会输出具体错误信息到日志，或者抛出异常，视不同的 coder 而定。

### 2.2. json

#### 2.2.1. EncoderDecoder::jsonDecoder

**语法**

``` DolphinDB
EncoderDecoder::jsonDecoder(colNames, colTypes, [handler], [workerNum=1], [batchSize=0], [throttle=1], [isMultiJson=false])
```

**参数**

- `colNames`：指定要解析的 json 数据中的 key 的名称。它是 json 中的键，类型为 string 类型的 vector。

- `colTypes`: 指定要解析的 json 数据中的 value 的类型。它是 json 中的值类型，类型为 int 类型的 vector。

- `handler`：可以是一个表，用于输出解析结果，也可以是一个函数对象，当前 decoder 的输出将作为该函数的输入进行处理。

- `workerNum`：处理解析运算的工作线程数量。默认值是 1.

- `batchSize`：是一个整数。若为正数，表示未处理消息的数量达到 batchSize 时，handler 才会处理消息。若未指定或为非正数，每一批次的消息到达之后，handler 就会马上处理。默认值是 0。

- `throttle`：是一个浮点数，单位为秒，默认值为 1。表示继上次 handler 处理消息之后，若 batchSize 条件一直未达到，多久后再次处理消息。

- `isMultiJson`：是一个布尔值，默认为 false. 表示一行内是否存在多个 JSON 字符串。如果设置为 true, 会先进行字符串分割再进行解析，需要注意，设置为 true 时要求 json 内字符串格式的 value 不能带字符 `}` 和 `{`

**返回值**

解析 json 数据的 coder 对象

**jsonDecoder 对象函数 parseAndHandle**

`parseAndHandle` 方法解析 json 数据，输出到 handler 中

  方法的输入参数可以为 scalar，vector 或者 内存表。如果是 scalar 和 vector 类型则必须为 string 类型，如果是内存表，则必须只有一列，且列类型为 string 类型

  如果 schema 不匹配，抛出异常 `Usage: coder.parseAndHandle(obj). Obj must be a table with one STRING column`。

  如果其中的 json 数据格式非法，则会在 log 中输出 `syntax error while parsing value (具体异常信息)` 相关行，但仍然会输出字段全为空的结果到目标表中。

**jsonDecoder 对象函数 parse**

`parse` 方法解析输入的数据，输入参数类型同 parseAndHandle。返回 table。如果 json 输入非法，会输出字段全为空的结果。如果某个字段不出现，则会被自动赋空值。

**数据类型与转换**

colTypes 支持填写 BOOL、CHAR、INT、LONG、FLOAT、DOUBLE、STRING、BLOB 以及 arrayVector 类型 BOOL[]、INT[]、LONG[]、FLOAT[]、DOUBLE[]。

转换规则

| DDB类型 \ JSON 类型 | bool | number | string | bool array | number array | string array |
| -------- | ---- | ------ | ------ | ---------- | ------------ | ------------ |
| BOOL     | √ | √ | √ 非空即为 true | X | X | X |
| CHAR     | √ | X | 长度 <= 1的 string 支持 | X | X | X |
| INT      | √ | √ | X | X | X | X |
| LONG     | √ | √ | X | X | X | X |
| FLOAT    | √ | √ | X | X | X | X |
| STRING   | √ | √ | √ | X | X | X |
| BLOB     | √ | √ | √ | X | X | X |
| BOOL[]   | X | X | X | √ | √ | √ 非空即为 true |
| INT[]    | X | X | X | √ | √ | X |
| LONG[]   | X | X | X | √ | √ | X |
| FLOAT[]  | X | X | X | √ | √ | X |
| DOUBLE[] | X | X | X | √ | √         | X        |

*不在表格中出现的类型转换均为不支持*


精度说明：超出 double、long 等上限的数据，可以转为 STRING 类型保留精度。

**示例**

创建数据
``` DolphinDB
str={"int": -637811349, "long": 637811349473772538,"float":-0.004 ,"double": 12133.938,"string": "2022-02-22 13:55:47.377"}
data = [str.toStdJson(),str.toStdJson(),str.toStdJson(),str.toStdJson()]
appendData = table(data as `string)
```

使用未指定输出的 jsonDecoder 调用 parse 进行解析
``` DolphinDB
colNames = ["int", "long","float","double","string"]
colTypes = [INT, LONG, FLOAT , DOUBLE, STRING]
coder1 = EncoderDecoder::jsonDecoder(colNames, colTypes)
coder1.parse(appendData)
```
输出如下
```
int        long               float  double                string
---------- ------------------ ------ --------------------- -----------------------
-637811349 637811349473772538 -0.004 12133.938000000000101 2022-02-22 13:55:47.377
-637811349 637811349473772538 -0.004 12133.938000000000101 2022-02-22 13:55:47.377
-637811349 637811349473772538 -0.004 12133.938000000000101 2022-02-22 13:55:47.377
-637811349 637811349473772538 -0.004 12133.938000000000101 2022-02-22 13:55:47.377
```

使用指定输出及其他参数的 jsonDecoder 调用 parseAndHandle 进行解析
``` DolphinDB
handler = table(1:0, colNames, colTypes)
coder2 = EncoderDecoder::jsonDecoder(colNames, colTypes, handler, 4, 100, 0.01)
coder2.parseAndHandle(appendData)
```
handler 内容如下
```
int        long               float  double                string
---------- ------------------ ------ --------------------- -----------------------
-637811349 637811349473772538 -0.004 12133.938000000000101 2022-02-22 13:55:47.377
-637811349 637811349473772538 -0.004 12133.938000000000101 2022-02-22 13:55:47.377
-637811349 637811349473772538 -0.004 12133.938000000000101 2022-02-22 13:55:47.377
-637811349 637811349473772538 -0.004 12133.938000000000101 2022-02-22 13:55:47.377
```

使用指定 handler 为函数的 jsonDecoder 调用 parseAndHandle 进行解析
``` DolphinDB
colNames2 = ["int", "long","float","double","date", "time"]
colTypes2 = [INT, LONG, FLOAT , DOUBLE, TIMESTAMP, TIMESTAMP]
dest = table(1:0, colNames2, colTypes2)

def parserDef(msg, mutable dest) {
    t = table(msg[`int] as `int, msg[`long] as `long, msg[`float] as `float, msg[`double] as `double, temporalParse(msg[`string] ,"yyyy-MM-dd HH:mm:ss.SSS")  as `date);
    t.update!(`name, now());
    dest.append!(t);
}
coder3 = EncoderDecoder::jsonDecoder(colNames, colTypes, parserDef{, dest}, 4, 100, 1)
coder3.parseAndHandle(appendData)
```

dest 表格内容如下
```
int        long               float  double                date                    time
---------- ------------------ ------ --------------------- ----------------------- -----------------------
-637811349 637811349473772538 -0.004 12133.938000000000101 2022.02.22T13:55:47.377 2023.10.26T11:22:00.140
-637811349 637811349473772538 -0.004 12133.938000000000101 2022.02.22T13:55:47.377 2023.10.26T11:22:00.140
-637811349 637811349473772538 -0.004 12133.938000000000101 2022.02.22T13:55:47.377 2023.10.26T11:22:00.140
-637811349 637811349473772538 -0.004 12133.938000000000101 2022.02.22T13:55:47.377 2023.10.26T11:22:00.140
```
### 2.2 protobuf

**注意**

本插件支持的 protobuf 格式为 3.0，该格式不支持设定默认值，因此各个类型的空值与默认值都会当作默认值进行处理。

详见[Protocol Buffers: Default Values](https://developers.google.com/protocol-buffers/docs/proto3#default)


#### EncoderDecoder::extractProtobufSchema

##### 语法
``` dolphindb
EncoderDecoder::extractProtobufSchema(schemaPath, [toArrayVector=false], [messageName])
```


##### 参数

- `schemaPath`：STRING, 指定要解析的 protobuf 数据格式，为一个.proto 文件的路径。
 如果路径不存在则抛出异常 file 路径名 does not exist.
如果该 proto 无法解析也抛出异常 Failed to parse .proto definition

- `toArrayVector`：BOOL，可选参数，默认为false。决定是否需要将部分 repeated 关键字修饰的字段解析为 ARRAY VECTOR。true 则解析为 arrayVector。如果存在嵌套 repeat 字段，则仅将最内部 repeat 字段转换为 ARRAY VECTOR，非最内层的 repeat 字段仍作拍平处理。
  **注意** 如果存在 repeated message 字段，且字段内部只有一层原始类型，则整个 repeated message 中的字段都会被处理为 ARRAY VECTOR。

- `messageName`：STRING，可选参数，指需要被解析的 Message Type 名字，如果没有指定，则默认需要被解析的是 .proto 文件中出现的第一个 Message Type。

##### 返回值
返回一个由列名 name (类型 STRING) 、数据类型名称 typeString (类型 STRING)、数据类型枚举值 typeInt (类型 INT) 三列组成的表。

- 字段名：如果存在字段嵌套，则通过下划线将嵌套的字段名连接起来，作为 dolphindb 表的列名。

- 数据类型：根据类型转换表转换为 DolphinDB 类型。如果遇到不支持的类型，类型列填为 "Unsupported" 类型转换表如下
arrayVector 为 false 时，出现了有多个非嵌套关系的 repeated 字段，这些字段类型全部填为 "Unsupported" 。
arrayVector 为 true 时，出现了多个非嵌套关系的 重复嵌套 repeated 字段，这些字段全部填为 "Unsupported" 。

简单例子：
``` protobuf
syntax = "proto3";

message test_repeat_2{
    double col1 = 1;
    message repeatMsg {
        float col2 = 1;
    }
    repeated repeatMsg msg1 = 2;
}
```
该 protobuf schema 对应的 DolphinDB Table schema 为
|      name     |   typeString  | typeInt  |
| ------------- | ------- |------- |
| col1          | DOUBLE  |  16 |
| msg1_col2     | FLOAT   |  15 |

##### 转换规则
- 类型转换表 protobuf to DolphinDB

| protobuf 类型 | dolphindb 类型 | toArrayVector 为 true 时的 dolphindb 类型 | 备注                       |
| ------------- | ------- | ---------------------- | ---------------------- |
| double        | DOUBLE  | DOUBLE[] |                        |
| float         | FLOAT   | FLOAT[] |                        |
| int32         | INT     | INT[] |                        |
| int64         | LONG    | LONG[] |                        |
| uint32        | LONG    | LONG[] |                        |
| uint64        | LONG    | LONG[] | 溢出则转换为 LONG 最大值  |
| sint32        | INT     | INT[] |                        |
| sint64        | LONG    | LONG[] |                        |
| fixed32       | LONG    | LONG[] |                        |
| fixed64       | LONG    | LONG[] | 溢出则转换为 LONG 最大值  |
| sfixed32      | INT     | INT[] |                        |
| sfixed64      | LONG    | LONG[] |                        |
| bool          | BOOL    | BOOL[] |                        |
| string        | STRING  | BLOB |  toArrayVector 为 true 时，可以根据 schema 转换为其他计算类型的 arrayVector，如果未指定，则拼接为 json 列表格式的字符串  |
| bytes         | BLOB    | BLOB |   同上，且如果 blob 中含有特殊字符，转换结果可能出现乱码（不推荐使用）                   |
| enum          | 不支持   | 不支持 ||
| map          | 不支持   | 不支持 | 某些情况下，带有一个 map 的类型的消息可以被处理，但是不推荐使用|
| group          | 不支持   | 不支持 ||

- protobuf 格式要求：

1. 一个.proto 文件中，最外层最好只有一个 message 定义。内部可以有多个子 message 定义。如果外层有多个 message 定义则取第一个 message 进行解析。

2. 推荐语法为 proto3

3. 不支持 extension 字段，不推荐有 optional 字段

4. 如果 toArrayVector 为 false，则不能有多个非嵌套关系的 repeated 字段，因为这无法拍平为一张表
如果 toArrayVector 为 true，则不能有多个非嵌套关系的 重复嵌套 repeated 字段，因为这无法在存在 arrayVector 时，拍平为一张表

5. 不能含有 oneof 关键字，因为一旦 oneof 中存在多种类型将无法自动拍平进入一张表格

6. 嵌套的 proto message 会以'_'作为连接符相互连接，需要用户保证连接后的结果不重复

#### EncoderDecoder::protobufDecoder

##### 语法

``` dolphindb
EncoderDecoder::protobufDecoder(schemaPath, [handler], [workerNum=1], [batchSize=0], [throttle=1], [toArrayVector=false], [schema], [messageName], [useNullAsDefault=false])
```

##### 参数

- `schemaPath`：STRING，指定要解析的 protobuf 数据格式，为一个 .proto 文件的路径

- `handler`：是一元函数、或数据表，用于处理输出的数据。

- `workerNum`：INT，处理解析运算的工作线程数量。默认值是 1，内部调用 server ploop 进行批量解析

- `batchSize`：INT。若为正数，表示未处理消息的数量达到 batchSize 时，handler 才会处理消息。若未指定或为非正数，每一批次的消息到达之后，handler 就会马上处理。默认值是 0。

- `throttle`：FLOAT，单位为秒，默认值为 1。表示继上次 handler 处理消息之后，若 batchSize 条件一直未达到，多久后再次处理消息。

- `toArrayVector`：BOOL，表示是否需要将 repeated 关键字修饰的字段解析为 ARRAY VECTOR。

- `schema`：表对象，用于指定各字段的数据类型。有 name 和 typeString 两列。schema 中不存在字段将不会被解析。若不指定则以原类型读取所有字段。handler 如果是表，它的列名与列类型需要与 schema 中完全一致。
  **如果 string 类型的 repeated 字段没有指定转换，则存为 BLOB 格式，数据内容为所有字段放入一个 array 中，以 json 格式存储，如 "["n1", "n2"]"，在 dolphindb 中也可以通过 str.parseExpr().eval() 的方式转换为 DolphinDB 的列表。**

- `messageName`：STRING，指需要被解析的 Message Type 名字，如果没有指定，则默认需要被解析的是 .proto 文件中出现的第一个 Message Type。

- `useNullAsDefault`：BOOL，默认为 false，指代是否要将没有显示指定默认值的字段指定为 ddb 空值。

  如果为 true，所有的空字段的值都会被解析为 ddb 的空值，注意如果 proto 序列化文件中有字段被显示指定为默认值（proto2 规则支持默认值被显示序列化），则该字段的值 **不会** 被解析为空值。

  如果为 false，则会采用 protobuf 规定的默认值 INTEGER: 0; FLOAT: 0.0; LITERAL: ""。

##### 返回值

解析 protobuf 数据的 coder 对象

- `parseAndHandle(data)` 方法进行 protobuf 数据的解析，并输出到 handler 中。方法的输入参数可以为：a.STRING SCALAR; b.STRING VECTOR; c.只有一列 STRING 类型列的内存表。

- `parse(data)` 方法进行 protobuf 数据的解析，并返回 table

  如果根据创建 coder 时传入的 schema，仍然有 protobuf 格式要求中不支持的字段（即 protobuf Decoder 解析规则 中会返回 “Unsupported" 为 typeString 的字段）。则解析会失败，输出具体的解析失败原因：

    - repeated 字段不可解析： Failed to parse the protobuf messages due to an excessive number of non-nested repeated fields.

    - 重名问题： Failed to parse the protobuf messages due to duplicate column names in the parsed table

    - 指定了不可解析的类型： Unsupported data type of field 字段名

具体格式要求与类型转换见 2.4


####  protobuf 完整示例

``` DolphinDB shell
// 用法1
coder = EncoderDecoder::protobufDecoder("path/to/proto");
coder.parse(data);

// 用法2
schema = EncoderDecoder::extractProtobufSchema("path/to/proto");
outputTable = table(1:0, schema[`name], schema[`type]);
coder = EncoderDecoder::protobufDecoder("path/to/proto", outputTable,3,1,0.1);
coder.parseAndHandle(data); //data 为只有一列 string 列的表，存储 proto 原始数据，输出到了 outputTable 表中
```