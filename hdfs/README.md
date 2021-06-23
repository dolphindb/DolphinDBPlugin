# DolphinDB HDFS Plugin

DolphinDB的HDFS插件可以从Hadoop的hdfs之中读取文件的信息，或者是将hdfs上的文件传至本地。

## 1 预编译安装

``` shell
loadPlugin("/path/to/bin/linux64/PluginHdfs.txt");
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
conn=hdfs::connect(nameMode, port, [userName], [kerbTicketCachePath] )
```

#### 参数

- 'nameMode'是hdfs所在的IP地址，如果是本地的也可以使用“localhost”
- ‘port’是hdfs开放的端口号，如果是本地一般为9000
- 'userName'是可以选择的登录的用户名
- 'kerbTicketCachePath'可选，连接到hdfs时要使用的Kerberos路径

#### 详情

返回值是一个建立链接之后的句柄，如果建立连接失败则会抛出异常

### 3.2 disconnect

#### 语法

``` shell
disconnect(hdfsFS)
```

#### 参数

- ‘hdfsFs’是connect()函数返回的句柄

#### 详情

用以取消已经建立的连接

### 3.3 exists

#### 语法

``` shell
exists(hdfsFS, path )
```

#### 参数

- ‘hdfsFs’是connect()函数返回的句柄
- ‘path’是你想判断是否存在的路径

#### 详情

用来判断某一个指定的路径是否存在，如果不存在则报错，如果存在则没有返回值

### 3.4 copy

#### 语法

``` shell
copy(hdfsFS1, src, hdfsFS2, dst)
```

#### 参数

- ‘hdfsFs1’是connect()函数返回的句柄
- 'src'是源文件的路径
- ‘hdfsFs2’是connect()函数返回的句柄
- ‘dst’是目标文件的路径

#### 详情

用来将一个hdfs中某一路径的文件拷贝到另一hdfs的某一路径之中，如果未成功则报错，如果成功则没有返回值

### 3.5 move

#### 语法

``` shell
move(hdfsFS1,src,hdfsFS2,dst)
```

#### 参数

- ‘hdfsFs1’是connect()函数返回的句柄
- 'src'是源文件的路径
- ‘hdfsFs2’是connect()函数返回的句柄
- ‘dst’是目标文件的路径

#### 详情

用来将一个hdfs中某一路径的文件移动到另一hdfs的某一路径之中，如果未成功则报错，如果成功则没有返回值

### 3.6 delete

#### 语法

``` shell
delete(hdfsFS, path, recursive )
```

#### 参数

- ‘hdfsFs’是connect()函数返回的句柄
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

- ‘hdfsFs’是connect()函数返回的句柄
- 'oldPath'是重命名之前文件的路径
- ‘newPath’是重命名之后文件的路径

#### 详情

用来将文件重命名，也可用于移动文件，如果未成功则报错，如果成功则没有返回值

### 3.8 createDirectory

#### 语法

``` shell
createDirectory(hdfsFS, path)
```

#### 参数

- ‘hdfsFs’是connect()函数返回的句柄
- ‘path’是想要创建的文件夹的路径

#### 详情

用来创建一个空文件夹，如果未成功则报错，如果成功则没有返回值

### 3.9 chmod

#### 语法

``` shell
chmod(hdfsFS, path, mode)
```

#### 参数

- ‘hdfsFs’是connect()函数返回的句柄
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

- ‘hdfsFs’是connect()函数返回的句柄
- ‘path’是目标目录

#### 详情

返回一个包含目标目录所有信息的句柄，如果未成功则报错，如果成功则没有返回值

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

- ‘hdfsFs’是connect()函数返回的句柄
- 'path'是想要读取的文件所在的路径
- 'handler'是只能够接受两个传入参数的用来处理文件字节流的函数

#### 详情

从hdfs的服务器中读取数据，调用handler函数将数据处理后存放在内存表中，返回值为该内存表。

### 3.13 writeFile

#### 语法

``` shell
readFile(hdfsFS, path, tabble, handler)
```

#### 参数

- ‘hdfsFs’是connect()函数返回的句柄
- 'path'是想要读取的文件所在的路径
- 'tb'要保存的内存表
- 'handler'是接受一个内存表作为参数，将内存表转换为数据流的函数

#### 详情

将内存表以特定格式存放在hdfs中。

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

