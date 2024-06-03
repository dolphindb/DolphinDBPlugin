# DolphinDB Redis 插件使用说明

通过 DolphinDB 的 Redis 插件，用户可以建立连接到指定 IP 和端口的 Redis 服务器，并进行数据操作。Redis 插件使用了 Hiredis 库，这是一个轻量级的 Redis C 客户端库。

本文档仅介绍编译构建方法。通过[文档中心 - Redis](https://docs.dolphindb.cn/zh/plugins/redis.html)查看使用介绍。

## 编译安装

### 在 Linux 下编译安装

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
