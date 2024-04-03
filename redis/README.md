# DolphinDB Redis 插件使用说明

通过 DolphinDB 的 Redis 插件，用户可以建立连接到指定 IP 和端口的 Redis 服务器，并进行数据操作。Redis 插件使用了 Hiredis 库，这是一个轻量级的 Redis C 客户端库。

## 在插件市场安装插件

### 安装步骤

(1) 在 DolphinDB 客户端中使用 listRemotePlugins 命令查看插件仓库中的插件信息。
```DolphinDB
login("admin", "123456")
listRemotePlugins()
```

(2) 使用 installPlugin 命令完成插件安装。
```DolphinDB
installPlugin("redis")
```
返回：<path_to_redis_plugin>/PluginRedis.txt

(3) 使用 loadPlugin 命令加载插件。
```DolphinDB
loadPlugin("redis")
```

## 函数接口

### redis::connect

**语法**

redis::connect(host, port)

**详情**

与 Redis server 建立一个连接，返回一个 Redis 连接的句柄。

**参数**

- host: 要连接的 Redis server 的 IP 地址，类型为 STRING。
- port: 要连接的 Redis server 的端口号，类型为 INT。

**返回值**

返回创建的 Redis 连接的句柄。

**示例**

假设 redis server 监听在 127.0.0.1:6379 端口。

```DolphinDB
conn = redis::connect("127.0.0.1",6379)
```

### redis::run

**语法**

redis::run(conn, arg1, arg2, arg3, ...)

**详情**

执行 Redis 命令，注意如果 Redis 设置有密码，需要首先 redis::run(conn, "AUTH", "password") 来获取权限。返回值可以是 Redis 可以返回的任何数据类型，如 string, list, 或 set。

注意如果运行时报错，那么该连接不能再被使用，应该 release 这个连接并重新 connect 建立一个新的连接。如果继续使用之前的连接，会一直报同样的报错。

**参数**

- conn: 通过 redis::connect 获得的 Redis 连接句柄。
- arg1: SET, GET 等 Redis 命令，类型为STRING。
- arg2, arg3, ...: Redis 命令所需的额外参数。

**返回值**

返回命令运行结果。

**示例**

运行 SET, GET 等 Redis 命令,并自动在 DolphinDB 数据和 Redis 请求/响应值之间转换 DolphinDB 的类型; 例如下面 redis::run(conn, "SET", "abc","vabc") 自动将 DolphinDB 的字符串 `"abc"` 转化为了 Redis 的字符串 `"abc"` 和 `"vabc"`

```DolphinDB
conn = redis::connect("127.0.0.1",6379)
redis::run(conn, "SET", "abc", "vabc")
val = redis::run(conn, "GET", "abc")
val == "vabc"
```

### redis::batchSet

**语法**

redis::batchSet(conn, keys, values)

**详情**

批量执行 Redis 的 set 操作，可通过 subscribeTable 函数来订阅流数据表。注意如果运行时报错，那么该连接不能再被使用，应该 release 这个连接并重新 connect 建立一个新的连接。如果继续使用之前的连接，会一直报同样的报错。

**参数**

- conn: 通过 redis::connect 获得的 Redis 连接句柄。
- keys: 要设置的 keys，为 String 标量或者向量。
- values: 要设置的 values，为 String 标量或者向量。

**示例**

```DolphinDB
conn = redis::connect("127.0.0.1",6379)

redis::batchSet(conn, "k1", "v1")

keys = ["k2", "k3", "k4"]
values = ["v2", "v3", "v4"]
redis::batchSet(conn, keys, values)
```

### redis::batchHashSet

**语法**

redis::batchHashSet(conn, ids, fieldData)

**详情**

批量执行 Redis 的 HSET 操作。注意如果运行时报错，那么该连接不能再被使用，应该 release 这个连接并重新 connect 建立一个新的连接。如果继续使用之前的连接，会一直报同样的报错。

**参数**

- conn: 通过 redis::connect 获得的 Redis 连接句柄。
- ids: 一个 String 类型数组，每一个元素作为 HSET 中的一个 key 对应 fieldData 表中的一行数据。
- fieldData: 一个每列都是 String 类型的表，每列列名作为 Redis HSET 中的 field，值作为 HSET 中的 value。

**示例**

```DolphinDB
loadPlugin("path/PluginRedis.txt");
go

// 生成数据
n=43200
instrument_id = take("HO2305-D-",n)+string(1..n)
time_stamp = timestamp(2024.02.02T09:30:01..2024.02.02T21:30:00)
jiaoyisuo = take("CFFEX",n)
last_price = rand(10.0,n)
volume = rand(100000,n)
bid_price1 = rand(10.0,n)
bid_volume1 = rand(1000,n)
bid_price2 = rand(10.0,n)
bid_volume2 = rand(1000,n)
bid_price3 = rand(10.0,n)
bid_volume3 = rand(1000,n)
bid_price4 = rand(10.0,n)
bid_volume4 = rand(1000,n)
bid_price5 = rand(10.0,n)
bid_volume5 = rand(1000,n)
t = table(instrument_id, time_stamp,jiaoyisuo, last_price,volume, bid_price1,bid_volume1, bid_price2,bid_volume2,bid_price3,bid_volume3,bid_price4,bid_volume4,bid_price5,bid_volume5)
conn=redis::connect("127.0.0.1",6379)

// 批量设置
ids = exec instrument_id from t
fieldData =  select string(time_stamp) as time_stamp,string(jiaoyisuo) as jiaoyisuo,string(last_price) as last_price,string(volume) as volume,string(bid_price1) as bid_price1,string(bid_volume1) as bid_volume1,string(bid_price2) as bid_price2,string(bid_volume2) as bid_volume2,string(bid_price3) as bid_price3,string(bid_volume3) as bid_volume3,string(bid_price4) as bid_price4,string(bid_volume4) as bid_volume4,string(bid_price5) as bid_price5,string(bid_volume5) as bid_volume5 from t

redis::batchHashSet(conn, ids, fieldData)
```

### redis::release

**语法**

redis::release(conn)

**详情**

关闭与 Redis server 的连接 conn。


**参数**

conn: 通过 redis::connect 获得的 Redis 连接句柄。

**示例**

```DolphinDB
conn = redis::connect("127.0.0.1",6379)
redis::release(conn)
```

### redis::releaseAll

**语法**

redis::releaseAll()

**详情**

关闭所有与 Redis server 的连接。

**示例**

```DolphinDB
conn = redis::releaseAll()
```

### redis::getHandle

**语法**

redis::getHandle(token)

**详情**

获取 token 对应的 Redis 句柄。

**参数**

- token: 通过 redis::getHandleStatus 返回表中的第一列得知，用于唯一标识一个 Redis 连接。

**返回值**

返回对应 token 的句柄。

**示例**

```DolphinDB
handle = redis::getHandle(token)
```

### redis::getHandleStatus

**语法**

redis::getHandleStatus()

**详情**

返回一张表描述当前所有已建立的连接。token 列是该连接的唯一标识，可通过 redis::getHandle(token) 来获取句柄；address 是连接的 Redis server 的 "ip:port" 网络地址；createdTime 是该连接创建的时间。

**返回值**

返回一张表，表中信息描述了当前所有已建立的连接。


**示例**

```DolphinDB
status = redis::getHandleStatus()
```

## 使用示例

下面是一个订阅流数据表的示例：

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

## 附录

### 预编译安装

预先编译的插件文件存放在[bin/linux](https://github.com/dolphindb/DolphinDBPlugin/tree/release200.11/redis/bin/linux) 目录。将该目录下的所有文件下载至 DolphinDB server 所在机器的如下目录：/DolphinDB/server/plugins/redis。

启动 DolphinDB 后，加载插件：
```DolphinDB
loadPlugin("/your_plugin_path/PluginRedis.txt")
```

### 手动编译安装

**编译**

需要 cmake 3.0 以上，gcc 版本推荐 gcc4.8.5（依赖 Redis 的 c 客户端库 hiredis 的 [v1.1.0](https://github.com/redis/hiredis/tree/v1.1.0) 版本，已经放到了 contrib/hiredis 中，无需手动下载）

```bash
mkdir build
cd build
cmake ..
make -j16
```

**安装**

添加插件所在路径到 LIB 搜索路径 LD_LIBRARY_PATH
```bash
export LD_LIBRARY_PATH=path_to_redis_plugin/:$LD_LIBRARY_PATH
```

启动 DolphinDB server 并导入插件
```DolphinDB
loadPlugin("path_to_redis_plugin/PluginRedis.txt") 
```
