# DolphinDB OPCUA Plugin

OPC 是自动化行业与其他行业用于数据安全交换的互操作性标准。OPC DA 只可用于 Windows 操作系统，OPC UA 则可以独立于平台。本插件实现了 DolphinDB 与 OPC UA 服务器之间的数据传输，可以连接、查看以及读取写入信息。

本文档仅介绍编译构建方法。通过 [文档中心-opcua](https://docs.dolphindb.cn/zh/plugins/opcua/opcua.html) 查看接口介绍；通过 [CHANGELOG.md](./CHANGELOG.md) 查看版本发布记录。

## 编译构建

### 环境准备

需要先编译 mbedtls 静态库和 open62541 动态库。步骤如下：

**Windows**

安装 mbedtls

* 从 GitHub 上下载最新的 mbedtls 项目：
    ```
    git clone https://github.com/ARMmbed/mbedtls.git
    ```

* 使用 CMake 编译为静态库：
    ```
    cd mbedtls
    mkdir build
    cd build
    cmake .. -G "MinGW Makefiles" -DENABLE_PROGRAMS=OFF
    make
    ```
/libmbedcrypto.a $(LIB_DIR)/libmbedx509.a $(LIB_DIR)/libmbedtls.a
编译得到静态库位于 mbedtls/build/library 下，分别是 libmbedcrypto.a、libmbedx509.a、libmbedtls.a。

安装 open62541

* 从 GitHub 上下载 1.0 版本的 open62541 项目：
```
git clone https://github.com/open62541/open62541.git
git submodule update --init --recursive
cd open62541
git checkout 1.0
```

* 使用 CMake 编译为动态库：
```
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DUA_ENABLE_SUBSCRIPTIONS=ON -DBUILD_SHARED_LIBS=ON -DUA_ENABLE_ENCRYPTION=ON -DMBEDTLS_INCLUDE_DIRS="path_to_mbedtls/include" -DMBEDTLS_LIBRARIES="path_to_mbedtls/build/library" -DMBEDTLS_FOLDER_INCLUDE="path_to_mbedtls/include" -DMBEDTLS_FOLDER_LIBRARY="path_to_mbedtls/build/library"
make
```
**请注意**：用户需要根据实际情况替换路径 -DMBEDTLS_INCLUDE_DIRS 和 -DMBEDTLS_LIBRARIES。

**Linux Ubuntu**

安装 mbedtls
```
sudo apt-get install libmbedtls-dev
```

安装 open62541 方法与 Windows 一致，可不指定 -DMBEDTLS_INCLUDE_DIRS 和 -DMBEDTLS_LIBRARIES。

**Linux Centos**

安装 mbedtls
```
yum install mbedtls-devel
```

安装 open62541 方法与 Windows 一致，可不指定 -DMBEDTLS_INCLUDE_DIRS 和 -DMBEDTLS_LIBRARIES。

### 使用 cmake 构建

* 复制 mbedtls/build/library 目录下的 libmbedcrypto.a、libmbedx509.a、libmbedtls.a 到./lib 目录下；复制 mbedtls/include 目录下的 mbedtls 和 psa 文件夹到./include 目录下（linux 系统可跳过这一步）。

* 复制 open62541/build/bin 目录下的.dll 文件，或者所有.so 文件到./lib 目录下；复制 open62541/build/src_generated/、open62541/include/、open62541/plugins/include/ 下的文件夹到./include 目录下，open62541/arch/ 下的文件夹到./include/open62541 目录下。

* 使用 cmake 构建 libPluginOPCUA，linux 不需要指定 - G。
```
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DLIBDOLPHINDB="path_to_libdolphindb"
make
```
**请注意**：用户需要根据实际情况替换路径 -DLIBDOLPHINDB。

* 将 libopen62541.dll 或 libopen62541.so 复制到 build 目录下。
