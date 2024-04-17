# DolphinDB TCPSocket 插件使用说明

通过 DolphinDB 的 TCPSocket 插件，用户可以创建 TCP 连接并连接到指定的 IP 和端口，然后进行数据的接收和发送。

## 1. 在插件市场安装插件

### 1.1 版本要求

- DolphinDB Server: 2.00.10 及更高版本

    注意：目前仅支持 x64 和 arm 的 Linux 版本。

### 1.2 安装步骤

1. 在DolphinDB 客户端中使用 [listRemotePlugins](https://docs.dolphindb.cn/zh/funcs/l/listRemotePlugins.html) 命令查看插件仓库中的插件信息。

   ```
   login("admin", "123456")
   listRemotePlugins(, "http://plugins.dolphindb.cn/plugins/")
   ```

2. 使用 [installPlugin](https://docs.dolphindb.cn/zh/funcs/i/installPlugin.html) 命令完成插件安装。

   ```
   installPlugin("tcpsocket")
   ```

   返回：<path_to_TCPSocket_plugin>/PluginTCPSocket.txt

3. 使用 loadPlugin 命令加载插件（即上一步返回的.txt文件）。

   ```
   loadPlugin("<path_to_TCPSocket_plugin>/PluginTCPSocket.txt")
   ```

## 2. 函数接口

### 2.1 TCPSocket::createSubJob

**语法**

`TCPSocket::createSubJob(ip, port, handler, [config])`

**详情**

创建一个 TCP 连接，连接指定的 IP 和 port，并创建一个后台任务，接收 TCP 的数据。

注意： `TCPSocket::createSubJob` 创建的 job 不会随着 session 关闭自动退出，必须手动调用 `TCPSocket::cancelSubJob` 结束。

**参数**

- ip: STRING 类型的常量。
- port: INT 类型的常量。
- handler: 用于解析从 TCP 的连接里接收到的数据。
    handler 需要的函数签名为：
  ```
    def hdndler(data){
    }
  ```
    其中输入的 data 是一个表，包含如下列：
    - bytes：接收到的 TCP 数据，类型为 BLOB。最大长度为 10240 字节。
    - isHead：是否是新的数据，表示该行所接的 TCP 数据是否和上一行是连续的。每次 TCP 连接后，第一行数据的 *isHead* 为 true。
- config: 配置参数。一个字典，类型为(STRING, ANY)。目前支持 *maxRows*。
  - maxRows: 每次执行 parser 时的最多的数据行数。默认为128。

**返回值**

返回一个 STRING 的常量，可用于后续 `TCPSocket::cancelSubJob` 使用。

### 2.2 TCPSocket::getSubJobStat

**语法**

`TCPSocket::getSubJobStat()`

**详情**

获取所有的 TCP 后台任务信息，包含已经通过 `TCPSocket::cancelSubJob` 取消的任务。

发生错误时，都会记录到 *lastErrMsg*、*lastFailedTimestamp* 里。

- 会记录到 *failedMsgCount* 的情况：parser 失败，插入到表失败。
- 只会记录到 *lastErrMsg*、*lastFailedTimestamp* 的情况：TCP 连接失败、连接断开、读取 TCP 数据失败时。

**返回值**

返回一个表，包含如下列：

- `tag`：TCP 任务标识。STRING 类型。
- `startTime`: 任务创建时间。NANOTIMESTAMP 类型。
- `endTime`: 任务取消时间。NANOTIMESTAMP 类型。任务可以通过 `TCPSocket::cancelSubJob` 来取消。
- `firstMsgTime`: 第一条数据的接收时间。NANOTIMESTAMP 类型。
- `lastMsgTime`: 上一条消息的接收时间。NANOTIMESTAMP 类型。
- `processedMsgCount`: 成功处理的消息行数。LONG 类型。
- `failedMsgCount`： 处理失败的消息行数。LONG 类型。
- `lastErrMsg`: 上一次处理失败的错误信息。STRING 类型。
- `lastFailedTimestamp`: 上一次处理失败的时间。NANOTIMESTAMP 类型。

### 2.3 TCPSocket::cancelSubJob

**语法**

`TCPSocket::cancelSubJob(tag)`

**详情**

取消一个 TCP 的后台任务

**参数**

- tag: TCP 任务标识，类型为 STRING 常量。可以通过 `TCPSocket::getSubJobStat()` 查询获得或者 `TCPSocket::createSubJob` 函数返回获得。

### 2.4 TCPSocket::connect

**语法**

`TCPSocket::connect(ip, port)`

**详情**

创建一个阻塞模式的 TCP 连接。`TCPSocket::connect` 创建的连接。

注意：在当前 session 关闭时，插件会自动调用 `TCPSocket::close` 以关闭连接。

**参数**

- ip: STRING 类型的常量。
- port: INT 类型的常量。

**返回值**

返回一个 TCP 的 socket 资源，可用于后续在 `TCPSocket::read` 和 `TCPSocket::close` 中使用。

**异常**

如果连接失败，会抛出异常:

 `[PLUGIN::TCPSocket]: failed to connect to host:port with IO error type ${IO_ERROR}`

### 2.5 TCPSocket::read

**语法**

`TCPSocket::read(socket, [size])`

**详情**

读取 TCP 的 socket 里的数据。

**参数**

- socket: 通过 `TCPSocket::connect` 创建的 socket 连接。
- size: 读取的最大字节数。类型为 INT 的常量。默认是 10240 字节数。

**返回值**

返回一个 BLOB 类型的常量。

**异常**

如果读取数据失败，会抛出异常:

 `[PLUGIN::TCPSocket]: failed to read from socket with IO error type ${IO_ERROR}`

### 2.6 TCPSocket::write

**语法**

`TCPSocket::write(socket, data)`

**详情**

向 TCP 的 socket 写数据。

**参数**

- socket: 通过 `TCPSocket::connect` 创建的 socket 连接。
- data: 需要往 socket 写的数据。BLOB 或 STRING 类型的常量。

**返回值**

如果写入成功，会返回 true。如果写入失败，会抛出异常。

**异常**

如果写入数据失败，会抛出异常:

 `[PLUGIN::TCPSocket]: failed to write to socket with IO error type ${IO_ERROR}`

### 2.7 TCPSocket::close

**语法**

`TCPSocket::close(socket)`

**详情**

关闭一个 TCP 的 socket 连接。

**参数**

- socket: 通过 `TCPSocket::connect` 创建的 socket 连接。

**返回值**

返回 true。

**异常**

如果是发生了 IO 错误，会抛出异常：

 `[PLUGIN::TCPSocket]: failed to close the socket with IO error type ${IO_ERROR}`

## 3. 使用示例

```
def handler(mutable table1, data){
	table1.append!(data)
}
share table(1:0, `bytes`isHead, [BLOB, BOOL]) as t
config = dict(STRING, ANY)
config[`maxRows] = 8192
	
TCPSocket::createSubJob("192.168.1.38", 20002, handler{t}, config)
```

## 附录

### 预编译安装

#### Linux

预先编译的插件文件存放在 `DolphinDBPlugin/TCPSocket/bin/linux64` 目录下。通过 DolphinDB，执行以下命令可导入 TCPSocket 插件：

```
cd DolphinDB/server //进入DolphinDB server目录
./dolphindb //启动 DolphinDB server
loadPlugin("<PluginDir>/TCPSocket/bin/linux64/PluginTCPSocket.txt") //加载插件
```

### 自行编译

#### Linux

```
mkdir build
cd build
cmake  ../
make
```

> **注意**：编译之前请确保 libDolphinDB.so 在 gcc 可搜索的路径中。可使用 `LD_LIBRARY_PATH` 指定其路径，或者直接将其拷贝到 build 目录下。

编译后目录下会产生两个文件：libPluginTCPSocket.so 和 PluginTCPSocket.txt。
