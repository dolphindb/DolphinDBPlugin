# DolphinDB ORC Plugin

DolphinDB orc plugin supports importing and exporting ORC files, automatically converting data types in the process.
For the latest documentation, visit orc.

**Required server version**: DolphinDB 2.00.10 or higher

**Supported OS**: Linux x86-64 and Linux JIT

## Build from Source

### On Linux x86

(1) Install CMake.

```
sudo apt-get install cmake
```

(2) Install cyrus-sasl.

```
tar -xf cyrus-sasl-2.1.27.tar.gz
cd cyrus-sasl-2.1.27
./configure CFLAGS=-fPIC CXXFLAGS=-fPIC LDFLAGS=-static
cp ./lib/.libs/libsasl2.a path_to_orc_plugin/lib/linux
```

(3) Install openssl.

```
wget https://www.openssl.org/source/old/1.0.2/openssl-1.0.2u.tar.gz
tar -xzf openssl-1.0.2u.tar.gz
cd openssl-1.0.2u
./config shared --prefix=/tmp/ssl -fPIC
make
make install
cp /tmp/ssl/lib/libcrypto.a path_to_orc_plugin/lib/linux
```

(4) Install orc development kit.

```
tar -xf orc-1.6.7.tar.gz
cd orc-1.6.7
mkdir build
cd build
cmake .. -DBUILD_JAVA=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON
make package
tar -xf ORC-1.6.7-Linux.tar.gz
cp ORC-1.6.7-Linux/lib/* path_to_orc_plugin/lib/linux
```

(5) Recompile libprotobuf.a.

```
cd orc-1.6.7/build/protobuf_ep-prefix/src/
tar -xf v3.5.1.tar.gz
cd protobuf-3.5.1
./autogen.sh
cd cmake
mkdir build
cd build
cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON
make
cp libprotobuf.a path_to_orc_plugin/lib/linux
```

(6) Recompile libsnappy.a.

```
cd orc-1.6.7/build/snappy_ep-prefix/src/
tar -xf 1.1.7.tar.gz
cd snappy-1.17
mkdir build
cd build
cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON
make
cp libsnappy.a path_to_orc_plugin/lib/linux
```

(7) Recompile libz.a.

```
cd orc-1.6.7/build/zlib_ep-prefix/src
tar -xf zlib-1.2.11.tar.gz
cd zlib-1.2.11
mkdir build
cd build
cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON
make
cp libz.a path_to_orc_plugin/lib/linux
```

(8) Specify the path of libDolphinDB.so.

```
export LIBDOLPHINDB=path_to_libdolphindb/
```

(9) Compile the plugin.

```
mkdir build
cd build
cmake path_to_orc_plugin/
make
```

**Note**: Make sure libDolphinDB.so is under the GCC search path before compilation. You can add the plugin path to the library search path LD_LIBRARY_PATH or copy it to the build directory.

### On Linux ARM

(1) Install OpenSSL 1.0.1.

```
wget https://www.openssl.org/source/old/1.0.1/openssl-1.0.1u.tar.gz
tar -xzf openssl-1.0.1u.tar.gz
cd openssl-1.0.1u
./config shared --prefix=/tmp/ssl -fPIC
make
make install
```

(2) Install orc development kit.

```
tar -xf orc-1.6.7.tar.gz
cd orc-1.6.7
mkdir build
cd build
CMAKE_PREFIX_PATH=/tmp/ssl cmake .. -DBUILD_JAVA=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON//指定里前面安装openssl1.0.1u的路径。
make package
tar -xf ORC-1.6.7-Linux.tar.gz
cp ORC-1.6.7-Linux/lib/* path_to_orc_plugin/lib_arm/
```

(3) Recompile libprotobuf.a.

```
cd orc-1.6.7/build/protobuf_ep-prefix/src/
tar -xf v3.5.1.tar.gz
cd protobuf-3.5.1
./autogen.sh
cd cmake
mkdir build
cd build
cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON
make
cp libprotobuf.a path_to_orc_plugin/lib_arm/
```

(4) Recompile libsnappy.a.

```
cd orc-1.6.7/build/snappy_ep-prefix/src/
tar -xf 1.1.7.tar.gz
cd snappy-1.1.7
mkdir build
cd build
cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON
make
cp libsnappy.a path_to_orc_plugin/lib_arm/
```

(5) Recompile libz.a.

```
cd orc-1.6.7/build/zlib_ep-prefix/srcap
tar -xf zlib-1.2.11.tar.gz
cd zlib-1.2.11
mkdir build
cd build
cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON
make
cp libz.a path_to_orc_plugin/lib_arm/
```

(6) Copy the lib_arm folder from Linux ARM to a Linux x86-64 machine using a cross compiler and compile the plugin.

```
mkdir build
cd build
cp ../lib_arm/* .
CC=aarch64-linux-gnu-gcc CXX=aarch64-linux-gnu-g++ cmake .. -DBUILD_ARM=1
make
```