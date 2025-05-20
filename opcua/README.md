# DolphinDB OPCUA Plugin

DolphinDB opcua plugin enables data transfer between DolphinDB and OPC UA servers.
For the latest documentation, visit [opcua](https://docs.dolphindb.com/en/Plugins/opcua.html).

**Required server version**: DolphinDB 2.00.10 or higher

**Supported OS**: Windows x86-64 and Linux x86-64

## Build from Source

### On Linux
(1) Install mbedtls.

For Linux Ubuntu:

```
sudo apt-get install libmbedtls-dev
```

For Linux Centos:

```
yum install mbedtls-devel
```

(2) Install open62541 same as on Windows, no need to specify -DMBEDTLS_INCLUDE_DIRS and -DMBEDTLS_LIBRARIES.

(3) Copy .dll files or all .so files from open62541/build/bin to ./lib directory. Copy folders under open62541/build/src_generated/, open62541/include/, open62541/plugins/include/ to ./include directory, and folders under open62541/arch/ to ./include/open62541 directory.

(4) Compile the plugin.

```
mkdir build
cd build
cmake ../
make
```
**Note**: Make sure libDolphinDB.so is under the GCC search path before compilation. You can add the plugin path to the library search path LD_LIBRARY_PATH or copy it to the build directory.

### On Windows
(1) Install mbedtls.

```
git clone https://github.com/ARMmbed/mbedtls.git
```

(2) Compile to static library using CMake:

```
cd mbedtls
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DENABLE_PROGRAMS=OFF
make
```
The compiled static libraries are under mbedtls/build/library, named libmbedcrypto.a, libmbedx509.a, and libmbedtls.a.

(3) Install open62541.

```
git clone https://github.com/open62541/open62541.git
git submodule update --init --recursive
cd open62541
git checkout 1.0
```

(4) Compile to dynamic library using CMake:

```
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DUA_ENABLE_SUBSCRIPTIONS=ON -DBUILD_SHARED_LIBS=ON -DUA_ENABLE_ENCRYPTION=ON -DMBEDTLS_INCLUDE_DIRS="path_to_mbedtls/include" -DMBEDTLS_LIBRARIES="path_to_mbedtls/build/library" -DMBEDTLS_FOLDER_INCLUDE="path_to_mbedtls/include" -DMBEDTLS_FOLDER_LIBRARY="path_to_mbedtls/build/library"
make
```

**Note**: The paths for -DMBEDTLS_INCLUDE_DIRS and -DMBEDTLS_LIBRARIES must be replaced with actual path.

(5) Copy libmbedcrypto.a, libmbedx509.a, libmbedtls.a from mbedtls/build/library to ./lib directory. Copy mbedtls and psa folders under mbedtls/include to ./include directory.

(6) Copy .dll files or all .so files from open62541/build/bin to ./lib directory. Copy folders under open62541/build/src_generated/, open62541/include/, open62541/plugins/include/ to ./include directory, and folders under open62541/arch/ to ./include/open62541 directory.

(7) Compile the plugin.

```
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DLIBDOLPHINDB="path_to_libdolphindb"
make
```
