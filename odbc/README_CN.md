# DolphinDB ODBC 插件
ODBC 为异构数据库访问提供统一接口，允许应用程序以 SQL 为数据存取标准。通过 DolphinDB 的 ODBC 插件，可以连接其它数据库，将数据导入到 DolphinDB 的内存表或分布式表中；或者将 DolphinDB 内存表导出到其它数据库。ODBC 插件基于 nanodbc 开发。本文档仅介绍编译构建方法。通过 [文档中心 - ODBC](https://docs.dolphindb.cn/zh/plugins/odbc/odbc.html) 查看接口介绍；通过 [CHANGELOG.md](./CHANGELOG.md) 查看版本发布记录。

## Linux 编译构建

### 编译 unixODBC-2.3.11

推荐编译 2.3.11 版本的 unixODBC 库。

```
wget https://src.fedoraproject.org/repo/pkgs/unixODBC/unixODBC-2.3.11.tar.gz/sha512/dddc32f90a7962e6988e1130a8093c6fb8b9ff532cad270d572250324aecbc739f45f9d8021d217313910bab25b08e69009b4f87456575535e93be1f46f5f13d/unixODBC-2.3.11.tar.gz
tar -zxvf unixODBC-2.3.11.tar.gz
LDFLAGS="-lrt" CFLAGS="-fPIC"  ./configure --prefix=/hdd1/gitlab/DolphinDBPlugin/unixodbc2.3.11Lib --enable-static=yes --enable-shared=no --sysconfdir=/etc/ --with-included-ltdl=yes
make -j
make install
```

### 编译 ODBC 插件

运行以下命令编译并生成 ODBC 插件 `libPluginODBC.so`：

```
cd <plugin_odbc_dir>
mkdir build
cd build
cmake ..  -DUNIXODBCDIR=/hdd1/gitlab/DolphinDBPlugin/unixodbc2.3.11Lib
make -j
```

### 编译 freetds odbc

如需连接 SQLServer 数据源，则需要编译 freetds odbc：

```
wget -c http://ibiblio.org/pub/Linux/ALPHA/freetds/stable/freetds-stable.tgz
tar -zxvf freetds-stable.tgz
cd freetds
./configure --prefix=/usr/local/freetds --with-tdsver=8.0 --enable-msdblib
make -j
make install
```

若插件运行机器与编译机器不是同一个，则需要将编译好的 freetds 拷贝至运行机器上，即：

* 将 `/usr/local/freetds/lib` 下的 freetds.conf, locales.conf, pool.conf 拷贝至到目标机器上的 `/usr/local/freetds/lib` 目录
* 将 `/usr/local/freetds/lib/ibtdsodbc.so.0.0.0` 拷贝至目标机器的 `/usr/local/freetds/lib` 目录。

> :bulb:**注意**：
>> 若使用 Docker 容器（即使用 Alpine Linux 操作系统），直接使用 `apk add freetds` 安装的新版本 freetds odbc 可能会与 DolphinDB ODBC 插件产生冲突而无法正常使用，因此推荐用户按照本小节给出的步骤下载并手动编译 freetds odbc。

在 Alpine Linux 环境中编译 freetds odbc 前，需要先添加某些库以提供编译环境：

```
wget -c http://ibiblio.org/pub/Linux/ALPHA/freetds/stable/freetds-stable.tgz
tar -zxvf freetds-stable.tgz

# 添加依赖库
apk add gcc
apk add g++
apk add make
apk add linux-headers
```
然后运行以下命令进行编译：

```
# 编译
./configure --prefix=/usr/local/freetds --with-tdsver=8.0 --enable-msdblib --disable-libiconv --disable-apps
make -j
make install
```
