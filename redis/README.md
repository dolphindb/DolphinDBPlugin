# DolphinDB Redis Plugin

## 1. 安装
### 1.1 编译

需要 cmake 3.0 以上，gcc 版本需要和编译 DolphinDB server 的 gcc 对应；
（依赖 redis 的 c 客户端库 hiredis 的 [v1.1.0](https://github.com/redis/hiredis/tree/v1.1.0) 版本，已经放到了 contrib/hiredis 中，无需手动下载）

```bash
mkdir build
cd build
cmake ..
make -j16
```

### 1.2 安装

(1) 添加插件所在路径到 LIB 搜索路径 LD_LIBRARY_PATH

```
export LD_LIBRARY_PATH=path_to_redis_plugin/:$LD_LIBRARY_PATH
```

(2) 启动 DolphinDB server 并导入插件

```
loadPlugin("path_to_redis_plugin/PluginRedis.txt")
```

## 2. 用户接口

### 2.1 redis::connect

##### 语法

redis::connect(host, port)

##### 参数

- host: 要连接的 Redis server 的 IP 地址，类型为 STRING。
- port: 要连接的 Redis server 的端口号，类型为 INT。

##### 详情

与 Redis server 建立一个连接，返回一个 Redis 连接的句柄。

##### 例子

假设 redis server 监听在 127.0.0.1:6379 端口

```
conn = redis::connect("127.0.0.1",6379)
```

### 2.2 redis::run

##### 语法

redis::run(conn, arg1, arg2, arg3, ...)

##### 参数

- conn: 通过 redis::connect 获得的 Redis 连接句柄。
- arg1: SET, GET 等 Redis 命令，类型为STRING。
- arg2, arg3, ...: Redis命令所需的额外参数。

##### 详情

执行 Redis 命令，注意如果 Redis 设置有密码，需要首先 redis::run(conn, "AUTH", "password") 来获取权限。返回值可以是 Redis 可以返回的任何数据类型，如 string, list, 或 set。

##### 例子

运行 SET, GET 等 redis 命令,并自动在 DolphinDB 数据和 redis 请求/响应值之间转换 DolphinDB 的类型; 例如下面 redis::run(conn, "SET", "abc","vabc") 自动将 DolphinDB 的字符串 `"abc"` 转化为了 redis 的字符串 `"abc"` 和 `"vabc"`

```
conn = redis::connect("127.0.0.1",6379)
redis::run(conn, "SET", "abc", "vabc")
val = redis::run(conn, "GET", "abc")
val == "vabc"
```

### 2.3 redis::batchSet

##### 语法

redis::batchSet(conn, keys, values)

##### 参数

- conn: 通过 redis::connect 获得的 Redis 连接句柄。
- keys: 要设置的 keys，为 String 标量或者向量。
- values: 要设置的 values，为 String 标量或者向量。

##### 详情

批量执行 redis 的 set 操作，可通过 subscribeTable 函数来订阅流数据表

##### 例子

```
conn = redis::connect("127.0.0.1",6379)

redis::batchSet(conn, "k1", "v1")

keys = ["k2", "k3", "k4"]
values = ["v2", "v3", "v4"]
redis::batchSet(conn, keys, values)
```

订阅流数据表示例：
```
loadPlugin("path/PluginRedis.txt");
go

dropStreamTable(`table1)

colName=["key", "value"]
colType=["string", "string"]
enableTableShareAndPersistence(table=streamTable(100:0, colName, colType), tableName=`table1, cacheSize=10000, asynWrite=false)

def myHandle(conn, msg) {
	redis::batchSet(conn, msg[0], msg[1])
}

conn = redis::connect("replace_with_redis_server_ip",6379)
subscribeTable(tableName="table1", handler=myHandle{conn})

n = 1000000
for(x in 0:n){
	insert into table1 values("key" + x, "value" + x)
}

t = table(n:0, [`id, `val], [`string, `string])
for(x in 0:n){
	insert into t values("key" + x, redis::run(conn, "GET", "key" + x))
}

ret = exec count(*) from t

assert "test", n==ret
```

### 2.4 redis::release

##### 语法

redis::release(conn)

##### 参数

conn: 通过 redis::connect 获得的 Redis 连接句柄。

##### 详情

关闭与 Redis server 的连接conn。

##### 例子

```
conn = redis::connect("127.0.0.1",6379)
redis::release(conn)
```

### 2.5 redis::releaseAll

##### 语法

redis::releaseAll()

##### 详情

关闭所有与 Redis server 的连接。

##### 例子

```
conn = redis::releaseAll()
```

### 2.6 redis::getHandle

##### 语法

redis::getHandle(token)

##### 参数

token: 通过 redis::getHandleStatus 返回表中的第一列得知，用于唯一标识一个 redis 连接。

##### 详情

获取 token 对应的 redis 句柄。

##### 例子

```
handle = redis::getHandle(token)
```

### 2.7 redis::getHandleStatus

##### 语法

redis::getHandleStatus()

##### 详情

返回一张表描述当前所有已建立的连接。token 列是该连接的唯一标识，可通过 redis::getHandle(token) 来获取句柄；address 是连接的 redis server 的 "ip:port" 网络地址；createdTime 是该连接创建的时间。

##### 例子

```
status = redis::getHandleStatus()
```
