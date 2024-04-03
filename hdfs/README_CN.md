# DolphinDB HDFS 插件使用说明

DolphinDB 的 HDFS 插件可以从 Hadoop 的 hdfs 之中读取文件的信息，或者是将 hdfs 上的文件传至本地。

## 准备工作

执行 Linux 命令，指定插件运行时需要的动态库路径，注意必须在设置完共享库查找路径后再启动 DolphinDB。

1. 安装 java 环境

    ``` shell
    yum install java
    yum install java-1.8.0-openjdk-devel
    ```

2. 寻找系统的 libjvm.so，选择要使用的 java 版本。

    ``` shell
    find /usr/-name "libjvm.so" // 寻找 JAVA 环境
    export JAVA_HOME=/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.362.b08-1.el7_9.x86_64 // 需要更改为实际的 java 路径
    export LD_LIBRARY_PATH=$JAVA_HOME/jre/lib/amd64/server:$LD_LIBRARY_PATH // 指定查找共享库的路径，确保 DolphinDB 启动时可以找到 jvm 库
    ```

3. 下载 3.2.2 版本的 hadoop

    下载 [hadoop-3.2.2](https://archive.apache.org/dist/hadoop/common/hadoop-3.2.2/hadoop-3.2.2.tar.gz) 并解压。

    ``` shell
    cd hadoop-3.2.2
    tar -zxvf hadoop-3.2.2.tar.gz
    export HADOOP_PREFIX=/hdd1/DolphinDBPlugin/hadoop-3.2.2 // 需要设置为实际路径
    export CLASSPATH=$($HADOOP_PREFIX/bin/hadoop classpath --glob):$CLASSPATH
    export LD_LIBRARY_PATH=$HADOOP_PREFIX/lib/native:$LD_LIBRARY_PATH
    ```

## 在插件市场安装插件

### 版本要求

DolphinDB Server: 2.00.10 及更高版本

注： 目前仅支持 x64 的 Linux 版本。

### 安装步骤

1. 在 DolphinDB 客户端中使用 [listRemotePlugins](../../funcs/l/listRemotePlugins.dita) 命令查看插件仓库中的插件信息。

    ```
    login("admin", "123456")
    listRemotePlugins(, "http://plugins.dolphindb.cn/plugins/")
    ```

2. 使用 [installPlugin](../../funcs/i/installPlugin.dita) 命令完成插件安装。

    ```
    installPlugin("HDFS")
    ```

   返回：<path_to_HDFS_plugin>/PluginHDFS.txt

3. 使用 loadPlugin 命令加载插件（即上一步返回的.txt 文件）。

    ```
    loadPlugin("<path_to_HDFS_plugin>/PluginHDFS.txt")
    ```

   ** 注意 **：若使用 Windows 插件，加载时必须指定绝对路径，且路径中使用 "\\\\" 或 "/" 代替 "\\"。

## 3 用户接口

### 3.1 connect

#### 语法

``` Dolphin Script
hdfs::connect(nameNode, [port], [userName], [kerbTicketCachePath], [keytabPath], [principal], [lifeTime])
```

#### 参数

- `nameNode`：字符串标量，是 hdfs 所在的 IP 地址，如果是本地的也可以使用 “localhost”, 也可以填完整的集群地址。如果这个参数中填了完整的集群地址，则不需要再填 port，为 hadoop 的集群配置项 fs.defaultFS 的值。

- `port`：整型标量，可选，hdfs 开放的端口号，如果是本地，一般为 9000. 如果 nameNode 填写的是完整的 server 地址，则不需要填 port 这个参数.

- `userName`：字符串标量，可选，是可以选择的登录的用户名

- `kerbTicketCachePath`：字符串标量，可选，连接到 hdfs 时要使用的 Kerberos 路径。为 hdfs 集群配置项的 hadoop.security.kerberos.ticket.cache.path 的值。
    如果没有指定后三项，则该位置是已经生成的 ticket 的路径
    如果指定了后面的三项，则该路径是需要生成的 ticket 存储的路径

- `keytabPath`：字符串标量，可选，是 kerberos 认证中，用于验证获得票据的 keytab 文件所在路径

- `principal`：字符串标量，可选，是 kerberos 认证中指定的需要验证的 principal。

- `lifeTime`：字符串标量，可选，是生成的票据的生存期，D 代表天，H 代表小时，M 代表分钟，S 代表秒。"4h5m" 代表 4 小时 5 分钟，"1d2s" 表示 1 天 2 秒。默认生存期为 1 天

#### 详情

返回值是一个建立连接之后的句柄，如果建立连接失败则会抛出异常

#### 示例

``` Dolphin Script
// 连接普通的 HDFS
conn=hdfs::connect("default",9000);

// 连接 kerberos 认证的 HDFS
keytabPath = "/path_to_keytabs/node.keytab"
cachePath = "/path_to_krb5Cache/cache"
principal = "user/example.com@DOLPHINDB.COM"
lifeTime = "1d3h"
connKerb5=hdfs::connect(`kerb5_url, 9001, , cachePath, keytabPath, principal, lifeTime)
```

### 3.2 disconnect

#### 语法

``` Dolphin Script
disconnect(hdfsFS)
```

#### 参数

- `hdfsFS`：connect 函数返回的句柄

#### 详情

用以取消已经建立的连接

### 3.3 exists

#### 语法

``` Dolphin Script
exists(hdfsFS, path)
```

#### 参数

- `hdfsFS`：connect 函数返回的句柄
- `path`：字符串标量，是想判断是否存在于 HDFS 系统中的路径

#### 详情

用来判断某一个指定的路径是否存在，如果不存在则报错，如果存在则没有返回值

### 3.4 coHDFS

#### 语法

``` Dolphin Script
coHDFS(hdfsFS1, src, hdfsFS2, dst)
```

#### 参数

- `hdfsFS1`：connect 函数返回的句柄
- `src`：字符串标量，是源文件的路径
- `hdfsFS2`：connect 函数返回的句柄
- `dst`：字符串标量，目标文件的路径

#### 详情

用来将一个 hdfs 中某一路径的文件拷贝到另一 hdfs 的某一路径之中，如果未成功则报错，如果成功则没有返回值

### 3.5 move

#### 语法

``` Dolphin Script
move(hdfsFS1,src,hdfsFS2,dst)
```

#### 参数

- `hdfsFS1`：connect 函数返回的句柄
- `src`：是源文件的路径
- `hdfsFS2`：connect 函数返回的句柄
- `dst`：目标文件的路径

#### 详情

用来将一个 hdfs 中某一路径的文件移动到另一 hdfs 的某一路径之中，如果未成功则报错，如果成功则没有返回值

### 3.6 delete

#### 语法

``` Dolphin Script
delete(hdfsFS, path, recursive)
```

#### 参数

- `hdfsFS`：connect 函数返回的句柄
- `path`： 是想要删除的文件的路径
- `recursive`：表示是否递归删除想要删除的目录

#### 详情

用来删除某一个目录或文件，如果未成功则报错，如果成功则没有返回值

### 3.7 rename

#### 语法

``` Dolphin Script
rename(hdfsFS, oldPath, newPath)
```

#### 参数

- `hdfsFS`：connect 函数返回的句柄
- `oldPath`：是重命名之前文件的路径
- `newPath`：重命名之后文件的路径。如果路径已经存在并且是一个目录，源文件将被移动到其中；如果路径存在并且是一个文件，或者缺少父级目录，则会报错。

#### 详情

用来将文件重命名，也可用于移动文件，如果未成功则报错，如果成功则没有返回值

### 3.8 createDirectory

#### 语法

``` Dolphin Script
createDirectory(hdfsFS, path)
```

#### 参数

- `hdfsFS`：connect 函数返回的句柄
- `path`：想要创建的文件夹的路径

#### 详情

用来创建一个空文件夹，如果未成功则报错，如果成功则没有返回值

### 3.9 chmod

#### 语法

``` Dolphin Script
chmod(hdfsFS, path, mode)
```

#### 参数

- `hdfsFS`：connect 函数返回的句柄
- `path`：想要修改权限的文件的路径
- `mode`：想要修改成为的权限值

#### 详情

用来修改某一文件或某一文件夹的使用权限，如果未成功则报错，如果成功则没有返回值

### 3.10 getListDirectory

#### 语法

``` Dolphin Script
getListDirectory(hdfsFS, path)
```

#### 参数

- `hdfsFS`：connect 函数返回的句柄
- `path`：目标目录

#### 详情

返回一个包含目标目录所有信息的句柄，如果未成功则会抛出异常

### 3.11 listDirectory

#### 语法

``` Dolphin Script
listDirectory(fileInfo)
```

#### 参数

- `fileInfo`：getListDirectory 函数返回的句柄

#### 详情

列出目标目录下所有文件的详细信息

### 3.12 freeFileInfo

#### 语法

``` Dolphin Script
freeFileInfo(fileInfo)
```

#### 参数

- `fileInfo`：getListDirectory 函数返回的句柄

#### 详情

用来释放目录信息所占用的空间

### 3.13 readFile

#### 语法

``` Dolphin Script
readFile(hdfsFS, path, handler)
```

#### 参数

- `hdfsFS`：connect 函数返回的句柄
- `path`：是想要读取的文件所在的路径
- `handler`：是只能够接受两个传入参数的用来处理文件字节流的函数

#### 详情

从 hdfs 的服务器中读取数据，调用 handler 函数将数据处理后存放在内存表中，返回值为该内存表。
handler 可以理解为反序列化接口，将 hdfs 中的文件反序列化为 dolphindb 的 table。参数有两个，一个是文件字节流的 buf 地址，一个是文件的长度。readFile 函数从 hdfs 中读取文件之后将文件的内容保存到 buf 指向的 buffer 中，并且缓存内容的长度。
handler 内部根据长度从 buffer 读取内容，进行反序列化，并保存到 dolphindb table 中。
目前已支持的函数有 orc 插件中的 orc::loadORCHdfs, parquet 插件中的 parquet::loadParquetHdfs。如果在 hdfs 中保存了其他格式，则需要根据具体的格式再开发反序列化的接口。

#### 示例

``` Dolphin Script
// 加载 ORC 插件
loadPlugin("<path_to_ORC_plugin>/PluginOrc.txt")

// 使用其中的 orc::loadORCHdfs 函数读取存在 HDFS 系统上的 ORC 格式的文件
re=hdfs::readFile(conn,'/tmp/testFile.orc',orc::loadORCHdfs)
```

### 3.14 writeFile

#### 语法

``` Dolphin Script
writeFile(hdfsFS, path, tb, handler)
```

#### 参数

- `hdfsFS`：connect 函数返回的句柄
- `path`：是想要读取的文件所在的路径
- `tb`：要保存的内存表
- `handler`：是接受一个内存表作为参数，将内存表转换为数据流的函数

#### 详情

将内存表以特定格式存放在 hdfs 中。
这里的 handler 和 readFile 中的 handle 是对应的关系，是序列化的接口，将 dolphindb 的 table 序列化成字节流，并保存到文件中。'tb' 是要保存的对象，handler 是序列化的接口，将 'tb' 对象序列化到一个 buffer 中。handler 的参数只有一个，被序列化的 table 对象，返回值是一个 vector，第一个元素是序列化后的 buffer 地址，第二个元素是 buffer 中内容的长度。在 writeFile 函数内部，会先调用 handler，将 tb 进行序列化，并获取 buffer 地址和长度，将 buffer 中的内容写入 hdfs 中的 buffer 里。
目前支持的 handler 只有 parquet 插件中的 parquet::saveParquetHdfs 函数，如果需要新增其他格式的序列化接口，则需要定制开发。

#### 示例

``` Dolphin Script
// 加载 Parquet 插件
loadPlugin("<path_to_Parquet_plugin>/PluginOrc.txt")

// 使用其中的 parquet::saveParquetHdfs 函数将内存表以 Parquet 格式写入给定的 HDFS 路径
hdfs::writeFile(conn,'/tmp/testFile.parquet',re,parquet::saveParquetHdfs)
```

## 完整示例

``` Dolphin Script
// 加载 HDFS 插件
loadPlugin("<path_to_HDFS_plugin>/PluginHDFS.txt")

// 连接 HDFS server
fs=hdfs::connect("default",9000);

// 判断指定的路径是否存在
hdfs::exists(fs,"/user/name");
hdfs::exists(fs,"/user/name1");

// 复制文件进行备份
hdfs::coHDFS(fs,"/tmp/testFile.txt",fs,"/tmp/testFile.txt.bk");
hdfs::coHDFS(fs,"/tmp/testFile1.txt",fs,"/tmp/testFile.txt.bk");

// 移动文件
hdfs::move(fs,"/tmp/testFile.txt.bk",fs,"/user/name/input/testFile.txt");
hdfs::move(fs,"/user/name/input/testFile.txt",fs,"/user/name1/testFile.txt");

// 将文件进行重命名
hdfs::rename(fs,"/user/name1/testFile.txt","/user/name1/testFile.txt.rename");

// 创建一个空文件夹
hdfs::createDirectory(fs,"/user/name");

// 修改权限为 600
hdfs::chmod(fs,"/user/name",600);

// 删除创建的文件夹
hdfs::delete(fs,"/user/name",1);

// 获取包含目标目录所有信息的句柄
fileInfo=hdfs::getListDirectory(fs,"/user/name/input/");

// 列出目标目录下所有文件的详细信息
hdfs::listDirectory(fileInfo);

// 用来释放目录信息所占用的空间
hdfs::freeFileInfo(fileInfo);

// 将原本存在 HDFS 系统上的 ORC 格式的文件读到内存表中
loadPlugin("<path_to_ORC_plugin>/PluginOrc.txt")
re=hdfs::readFile(conn,'/tmp/testFile.orc',orc::loadORCHdfs)

// 将内存表以 Parquet 格式写入给定的 HDFS 路径
loadPlugin("<path_to_Parquet_plugin>/PluginParquet.txt")
hdfs::writeFile(conn,'/tmp/testFile.parquet',re,parquet::saveParquetHdfs)

// 断开 HDFS 的连接
hdfs::disconnect(fs);
```

## 附录

### 编译安装方式

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
