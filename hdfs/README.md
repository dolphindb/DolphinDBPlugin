# DolphinDB HDFS Plugin

DolphinDB HDFS plugin can read files from Hadoop HDFS or load files from HDFS to DolphinDB.

The DolphinDB HDFS plugin has different branches, such as [release200](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/hdfs/README_EN.md) and [release130](https://github.com/dolphindb/DolphinDBPlugin/blob/release130/hdfs/README_EN.md). Each branch corresponds to a DolphinDB server version. Please make sure you are in the correct branch of the plugin documentation.

- [DolphinDB HDFS Plugin](#dolphindb-hdfs-plugin)
  - [1 Install Precompiled Plugin](#1-install-precompiled-plugin)
  - [2 Compile the Plugin](#2-compile-the-plugin)
    - [2.1 Environment Setup](#21-environment-setup)
    - [2.2 Compiling](#22-compiling)
  - [3 Methods](#3-methods)
    - [3.1 connect](#31-connect)
    - [3.2 disconnect](#32-disconnect)
    - [3.3 exists](#33-exists)
    - [3.4 copy](#34-copy)
    - [3.5 move](#35-move)
    - [3.6 delete](#36-delete)
    - [3.7 rename](#37-rename)
    - [3.8 createDirectory](#38-createdirectory)
    - [3.9 chmod](#39-chmod)
    - [3.10 getListDirectory](#310-getlistdirectory)
    - [3.11 listDirectory](#311-listdirectory)
    - [3.12 freeFileInfo](#312-freefileinfo)
    - [3.13 readFile](#313-readfile)
    - [3.14 writeFile](#314-writefile)
  - [Appendix](#appendix)


## 1 Install Precompiled Plugin

Pre-compiled plugin files are stored in the *DolphinDBPlugin/hdfs/bin/linux64*. Download it to */DolphinDB/server/plugins/hdfs*.

Specify the path to the dynamic libraries required for the plugin on Linux.

``` shell
export LD_LIBRARY_PATH=/path/to/plugins/hdfs:$LD_LIBRARY_PATH

 //Find the folder where the libjvm.so library is located.
export LD_LIBRARY_PATH=/path/to/libjvm.so/:$LD_LIBRARY_PATH
```

Start DolphinDB server and load the plugin

``` shell
cd DolphinDB/server //navigate into DolphinDB server directory
./dolphindb //start DolphinDB server
loadPlugin("/path/to/plugins/hdfs/PluginHdfs.txt");
```
## 2 Compile the Plugin

### 2.1 Environment Setup

``` shell 
# Download Hadoop
https://hadoop.apache.org
# for ubuntu users
sudo apt install cmake
# for Centos users
sudo yum install cmake
```

### 2.2 Compiling

``` shell
cd hdfs
mkdir build
cd build
cmake .. -DHADOOP_DIR=/path/to/your/hadoop/home
make
```

## 3 Methods

### 3.1 connect

**Syntax**

``` shell
conn=hdfs::connect(nameMode, port, [userName], [kerbTicketCachePath] )
```

**Parameters**

- nameMode: the IP address where the HDFS is located. If it is local, you can also use "localhost".
- port: the port number of HDFS. The local port is 9000.
- userName: the user name for login.
- kerbTicketCachePath: the path where Kerberos is located. It is an optional parameter.

**Details**

If the connection is built, return a handle. Otherwise, an exception will be thrown.

### 3.2 disconnect

**Syntax**

``` shell
disconnect(hdfsFS)
```

**Parameters**

- hdfsFS: the handle returned by the connect() function.

**Details**

Disconnect from the HDFS.

### 3.3 exists

**Syntax**

``` shell
exists(hdfsFS, path )
```

**Parameters**

- hdfsFS: the handle returned by the connect() function.
- path: an HDFS file path.

**Details**

Determine whether a specified path exists. If it does not exist, an error will be reported.

### 3.4 copy

**Syntax**

``` shell
copy(hdfsFS1, src, hdfsFS2, dst)
```

**Parameters**

- hdfsFS1: the handle returned by the connect() function.
- src: the path to the source file.
- hdfsFS2: the handle returned by the connect() function.
- dst: the destination path.

**Details**

Copy the files from one HDFS to another. If failed, an error will be reported.

### 3.5 move

**Syntax**

``` shell
move(hdfsFS1,src,hdfsFS2,dst)
```

**Parameters**

- hdfsFS1: the handle returned by the connect() function.
- src: the path to the source file.
- hdfsFS2: the handle returned by the connect() function.
- dst: the destination path.

**Details**

Move the files from one HDFS to another. If failed, an error will be reported.

### 3.6 delete

**Syntax**

``` shell
delete(hdfsFS, path, recursive )
```

**Parameters**

- hdfsFS: the handle returned by the connect() function.
- path: the path of the file to be deleted.
- recursive: indicates whether to delete files or folders recursively.

**Details**

Delete a directory or file. If failed, an error will be reported.

### 3.7 rename

**Syntax**

``` shell
rename(hdfsFS, oldPath, newPath )
```

**Parameters**

- hdfsFS: the handle returned by the connect() function.
- oldPath: the path of the file to be renamed.
- newPath: the path of the file after renaming. If a directory is specifed, the source file will be moved to it; If a file is specified, or the specified parent directory is missing, an error will be reported.

**Details**

Rename or move the files. If failed, an error will be reported.

### 3.8 createDirectory

**Syntax**

``` shell
createDirectory(hdfsFS, path)
```

**Parameters**

- hdfsFS: the handle returned by the connect() function.
- path: the path to the directory to be created.

**Details**

Create a new folder. If failed, an error will be reported.

### 3.9 chmod

**Syntax**

``` shell
chmod(hdfsFS, path, mode)
```

**Parameters**

- hdfsFS: the handle returned by the connect() function.
- path: the path to the file, of which access permissions you want to change.
- mode: the digits that represent different permissions.

**Details**

Control access to a file or a folder. If failed, an error will be reported.

### 3.10 getListDirectory

**Syntax**

``` shell
fileInfo=getListDirectory(hdfsFS, path)
```

**Parameters**

- hdfsFS: the handle returned by the connect() function.
- path: the path to the target directory.

**Details**

Return a handle containing all information about the target directory. If failed, an error will be reported.

### 3.11 listDirectory

**Syntax**

``` shell
listDirectory(fileInfo)
```

**Parameters**

- fileInfo: the handle returned by the getListDirectory() function.

**Details**

List all file information in the target directory.

### 3.12 freeFileInfo

**Syntax**

``` shell
freeFileInfo(fileInfo)
```

**Parameters**

- fileInfo: the handle returned by the getListDirectory() function.

**Details**

Free up space occupied by directory information.

### 3.13 readFile

**Syntax**

``` shell
readFile(hdfsFS, path, handler)
```

**Parameters**

- hdfsFS: the handle returned by the connect() function.
- path: the path of the file to be loaded.
- handler: the function for processing byte stream. It takes only 2 arguments.

**Details**

Read data from the HDFS server. Return an in-memory table that stores the data after it is processed with the handler function.

### 3.14 writeFile

**Syntax**

``` shell
readFile(hdfsFS, path, tb, handler)
```

**Parameters**

- hdfsFS: the handle returned by the connect() function.
- path: the path of the file to be loaded.
- tb: the in-memory table.
- handler: the function for converting the in-memory table to the data stream. It takes only one argument.

**Details**

Store in-memory tables in HDFS with a specific format.

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

