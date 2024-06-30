# DolphinDB HDFS 插件使用说明

HDFS是 Hadoop 的分布式文件系统（Hadoop Distributed File System），实现大规模数据可靠的分布式读写。DolphinDB 提供了 HDFS 插件，支持从 Hadoop 的 hdfs 之中读取文件的信息，读写 hdfs 上的 Parquet 或者 ORC 格式文件。

本文档仅介绍编译构建方法。通过 [文档中心-hdfs](https://docs.dolphindb.cn/zh/plugins/hdfs/hdfs.html) 查看接口介绍；通过 [CHANGELOG.md](./CHANGELOG.md) 查看版本发布记录。

## 编译构建

### Linux 编译构建

** 编译环境搭建 **

``` shell
#从 Hadoop 的官网下载 Hadoop 软件
https://hadoop.apache.org
# 对于 ubuntu 用户来说
sudo apt install cmake
# 对于 Centos 用户来说
sudo yum install cmake
```

** 编译安装 **

``` shell
cd hdfs
mkdir build
cd build
cmake .. -DHADOOP_DIR=/path_to_your_hadoop/home
make
```
