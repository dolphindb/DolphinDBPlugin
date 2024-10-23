# DolphinDB MySQL Plugin

DolphinDB的MySQL导入插件可将MySQL中的数据表或语句查询结果高速导入DolphinDB，并且支持数据类型转换。本插件的部分设计参考了来自Yandex.Clickhouse的mysqlxx组件。

本文档仅介绍编译构建方法。通过[文档中心 - MySQL](https://docs.dolphindb.cn/zh/plugins/mysql/mysql.html)查看使用介绍；通过 CHANGELOG.md 查看版本发布记录。

### 编译安装

### 在 Linux 下编译安装

##### 环境准备

安装 [git](https://git-scm.com/) 和 [CMake](https://cmake.org/)。

Ubuntu用户只需要在命令行输入以下命令即可：
```bash
$ sudo apt-get install git cmake
```

然后通过更新git子模块来下载[mariadb-connector-c](https://github.com/MariaDB/mariadb-connector-c)的源文件。
```
$ git submodule update --init --recursive
```


安装cmake：
```
sudo apt-get install cmake
```

##### cmake 编译

构建插件内容：
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../path/to/mysql_plugin/
make -j`nproc`
```

**注意:** 编译之前请确保libDolphinDB.so在gcc可搜索的路径中。可使用LD_LIBRARY_PATH指定其路径，或者直接将其拷贝到build目录下。

编译之后目录下会产生libPluginMySQL.so文件。

### 在 Windows 下编译安装

##### 在 Windows 环境中需要使用CMake和MinGW编译

* 下载安装[MinGW](http://www.mingw.org/)。确保将bin目录添加到系统环境变量Path中。 

* 下载安装[cmake](https://cmake.org/)。

##### cmake 编译

在编译开始之前，要将libDolphinDB.dll和包含curl头文件的文件夹拷贝到build文件夹内。

构建插件内容：
```
mkdir build                                                        # 新建build目录
cp path_to_libDolphinDB.dll/libDolphinDB.dll build                 # 拷贝 libDolphinDB.dll 到build目录下
cp -r curl build                                                   # 拷贝 curl 头文件到build目录下
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../path_to_mysql_plugin/ -G "MinGW Makefiles"
mingw32-make -j4
```
