# DolphinDB HDFS Plugin

DolphinDB的HDFS插件可以从Hadoop的hdfs之中读取文件的信息，或者是将hdfs上的文件传至本地。

HDFS插件目前支持版本：[relsease200](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/hdfs/README.md), [release130](https://github.com/dolphindb/DolphinDBPlugin/blob/release130/hdfs/README.md), [release120](https://github.com/dolphindb/DolphinDBPlugin/blob/release120/hdfs/README.md)。您当前查看的插件版本为release200，请使用DolphinDB 2.00.X版本server。若使用其它版本server，请切换至相应插件分支。

## 1 预编译安装
预先编译的插件文件存放在 DolphinDBPlugin/hdfs/bin/linux64 目录下。将其下载至 /DolphinDB/server/plugins/hdfs。

执行Linux命令，指定插件运行时需要的动态库路径
``` shell
export LD_LIBRARY_PATH=/path/to/plugins/hdfs:$LD_LIBRARY_PATH
find /usr/ -name "libjvm.so" //寻找系统上libjvm.so库的所在文件夹位置。
export LD_LIBRARY_PATH=/path/to/libjvm.so/:$LD_LIBRARY_PATH
```
启动DolphinDB，加载插件：
``` shell
cd DolphinDB/server //进入DolphinDB server目录
./dolphindb //启动 DolphinDB server
loadPlugin("/path/to/plugins/hdfs/PluginHdfs.txt");
```
## 2 编译安装

### 2.1 编译环境搭建

``` shell
#从Hadoop的官网下载Hadoop软件
https://hadoop.apache.org
# 对于ubuntu用户来说
sudo apt install cmake
# 对于Centos用户来说
sudo yum install cmake
```

### 2.2 编译安装

``` shell
cd hdfs
mkdir build
cd build
cmake .. -DHADOOP_DIR=/path/to/your/hadoop/home
make
```

## 3 用户接口

### 3.1 connect

#### 语法

``` shell
conn=hdfs::connect(nameNode, [port], [userName], [kerbTicketCachePath], [keytabPath], [principal], [lifeTime])
```

#### 参数

- 字符串标量，'nameNode'是hdfs所在的IP地址，如果是本地的也可以使用“localhost”, 也可以填完整的集群地址。如果这个参数中填了完整的集群地址，则不需要再填port，为hadoop的集群配置项fs.defaultFS的值。

- 整型标量，可选，‘port’是hdfs开放的端口号，如果是本地，一般为9000. 如果nameNode填写的是完整的server地址，则不需要填port这个参数.

- 字符串标量，可选, 'userName'是可以选择的登录的用户名

- 字符串标量，可选, 'kerbTicketCachePath'，连接到hdfs时要使用的Kerberos路径。为hdfs集群配置项的 hadoop.security.kerberos.ticket.cache.path 的值。
    如果没有指定后三项，则该位置是已经生成的 ticket 的路径
    如果指定了后面的三项，则该路径是需要生成的 ticket 存储的路径

- 字符串标量，可选, 'keytabPath'是 kerberos 认证中，用于验证获得票据的 keytab 文件所在路径

- 字符串标量，可选, 'principal'是 kerberos 认证中指定的需要验证的 principal。

- 字符串标量，可选, 'lifeTime' 是生成的票据的生存期，D 代表天，H 代表小时，M 代表分钟，S 代表秒。"4h5m" 代表 4 小时 5 分钟，"1d2s" 表示 1 天 2 秒。默认生存期为 1 天

#### 详情

返回值是一个建立链接之后的句柄，如果建立连接失败则会抛出异常

### 3.2 disconnect

#### 语法

``` shell
disconnect(hdfsFS)
```

#### 参数

- ‘hdfsFS’是connect()函数返回的句柄

#### 详情

用以取消已经建立的连接

### 3.3 exists

#### 语法

``` shell
exists(hdfsFS, path )
```

#### 参数

- ‘hdfsFS’是connect()函数返回的句柄
- ‘path’是你想判断是否存在的路径

#### 详情

用来判断某一个指定的路径是否存在，如果不存在则报错，如果存在则没有返回值

### 3.4 copy

#### 语法

``` shell
copy(hdfsFS1, src, hdfsFS2, dst)
```

#### 参数

- ‘hdfsFS1’是connect()函数返回的句柄
- 'src'是源文件的路径
- ‘hdfsFS2’是connect()函数返回的句柄
- ‘dst’是目标文件的路径

#### 详情

用来将一个hdfs中某一路径的文件拷贝到另一hdfs的某一路径之中，如果未成功则报错，如果成功则没有返回值

### 3.5 move

#### 语法

``` shell
move(hdfsFS1,src,hdfsFS2,dst)
```

#### 参数

- ‘hdfsFS1’是connect()函数返回的句柄
- 'src'是源文件的路径
- ‘hdfsFS2’是connect()函数返回的句柄
- ‘dst’是目标文件的路径

#### 详情

用来将一个hdfs中某一路径的文件移动到另一hdfs的某一路径之中，如果未成功则报错，如果成功则没有返回值

### 3.6 delete

#### 语法

``` shell
delete(hdfsFS, path, recursive )
```

#### 参数

- ‘hdfsFS’是connect()函数返回的句柄
- 'path'是想要删除的文件的路径
- ‘recursive’表示是否递归删除想要删除的目录

#### 详情

用来删除某一个目录或文件，如果未成功则报错，如果成功则没有返回值

### 3.7 rename

#### 语法

``` shell
rename(hdfsFS, oldPath, newPath )
```

#### 参数

- ‘hdfsFS’是connect()函数返回的句柄
- 'oldPath'是重命名之前文件的路径
- ‘newPath’是重命名之后文件的路径。如果路径已经存在并且是一个目录，源文件将被移动到其中；如果路径存在并且是一个文件，或者缺少父级目录，则会报错。

#### 详情

用来将文件重命名，也可用于移动文件，如果未成功则报错，如果成功则没有返回值

### 3.8 createDirectory

#### 语法

``` shell
createDirectory(hdfsFS, path)
```

#### 参数

- ‘hdfsFS’是connect()函数返回的句柄
- ‘path’是想要创建的文件夹的路径

#### 详情

用来创建一个空文件夹，如果未成功则报错，如果成功则没有返回值

### 3.9 chmod

#### 语法

``` shell
chmod(hdfsFS, path, mode)
```

#### 参数

- ‘hdfsFS’是connect()函数返回的句柄
- ‘path’是想要修改权限的文件的路径
- ‘mode’是想要修改成为的权限值

#### 详情

用来修改某一文件或某一文件夹的使用权限，如果未成功则报错，如果成功则没有返回值

### 3.10 getListDirectory

#### 语法

``` shell
fileInfo=getListDirectory(hdfsFS, path)
```

#### 参数

- ‘hdfsFS’是connect()函数返回的句柄
- ‘path’是目标目录

#### 详情

返回一个包含目标目录所有信息的句柄，如果未成功则会抛出异常

### 3.11 listDirectory

#### 语法

``` shell
listDirectory(fileInfo)
```

#### 参数

- ‘fileInfo’是getListDirectory()函数返回的句柄

#### 详情

列出目标目录下所有文件的详细信息

### 3.12 freeFileInfo

#### 语法

``` shell
freeFileInfo(fileInfo)
```

#### 参数

- ‘fileInfo’是getListDirectory()函数返回的句柄

#### 详情

用来释放目录信息所占用的空间

### 3.13 readFile

#### 语法

``` shell
readFile(hdfsFS, path, handler)
```

#### 参数

- ‘hdfsFS’是connect()函数返回的句柄
- 'path'是想要读取的文件所在的路径
- 'handler'是只能够接受两个传入参数的用来处理文件字节流的函数

#### 详情

从hdfs的服务器中读取数据，调用handler函数将数据处理后存放在内存表中，返回值为该内存表。
handler可以理解为反序列化接口，将hdfs中的文件反序列化为dolphindb的table。参数有两个，一个是文件字节流的buf地址，一个是文件的长度。readFile函数从hdfs中读取文件之后将文件的内容保存到buf指向的buffer中，并且缓存内容的长度。
handler内部根据长度从buffer读取内容，进行反序列化，并保存到dolphidb table中。
目前已支持的函数有orc插件中的orc::loadORCHdfs, parquet插件中的parquet::loadParquetHdfs。如果在hdfs中保存了其他格式，则需要根据具体的格式再开发反序列化的接口。

### 3.14 writeFile

#### 语法

``` shell
writeFile(hdfsFS, path, tb, handler)
```

#### 参数

- ‘hdfsFS’是connect()函数返回的句柄
- 'path'是想要读取的文件所在的路径
- 'tb'要保存的内存表
- 'handler'是接受一个内存表作为参数，将内存表转换为数据流的函数

#### 详情

将内存表以特定格式存放在hdfs中。
这里的handler和readFile中的handle是对应的关系，是序列化的接口，将dolphindb的table序列化成字节流，并保存到文件中。'tb'是要保存的对象，handler是序列化的接口，将'tb'对象序列化到一个buffer中。handler的参数只有一个, 被序列化的table对象，返回值是一个vector，第一个元素是序列化后的buffer地址，第二个元素是buffer中内容的长度。在writeFile函数内部，会先调用handler，将tb进行序列化，并获取buffer地址和长度，将buffer中的内容写入hdfs中的buffer里。
目前支持的handler只有parquet插件中的parquet::saveParquetHdfs函数，如果需要新增其他格式的序列化接口，则需要定制开发。

## Appendix

``` shell
loadPlugin("/path/to/PluginHdfs.txt");
fs=hdfs::connect("default",9000);
hdfs::exists(fs,"/user/name");
hdfs::exists(fs,"/user/name1");
hdfs::copy(fs,"/tmp/testfile.txt",fs,"/tmp/testfile.txt.bk");
hdfs::copy(fs,"/tmp/testfile1.txt",fs,"/tmp/testfile.txt.bk");
hdfs::move(fs,"/tmp/testfile.txt.bk",fs,"/user/name/input/testfile.txt");
hdfs::move(fs,"/user/name/input/testfile.txt",fs,"/user/name1/testfile.txt");
hdfs::rename(fs,"/user/name1/testfile.txt","/user/name1/testfile.txt.rename");
hdfs::createDirectory(fs,"/user/namme");
hdfs::chmod(fs,"/user/namme",600);
hdfs::delete(fs,"/user/namme",1);
hdfs::disconnect(fs);

fileInfo=hdfs::getListDirectory(fs,"/user/name/input/");
hdfs::listDirectory(fileInfo);
hdfs::freeFileInfo(fileInfo);

loadPlugin("/path/to/PluginOrc.txt")
re=hdfs::readFile(conn,'/tmp/testfile.orc',orc::loadORCHdfs)

loadPlugin("/path/to/PluginParquet.txt")
hdfs::writeFile(conn,'/tmp/testfile.parquet',re,parquet::saveParquetHdfs)
```


# ReleaseNotes

## 故障修复

* 修复使用方法 hdf5::ls 执行特定类型的 hdf5文件后 server 宕机的问题。（**2.00.10**）
* 修复并行导入多个文件时 server 宕机的问题。（**2.00.10**）

# 功能优化

* 优化接口 hdf5::saveHDF5 的报错信息。（**2.00.10**）
