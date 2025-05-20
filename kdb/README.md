# DolphinDB kdb+ Plugin

DolphinDB kdb+ plugin can be used to load the kdb+ tables on your disk into DolphinDB as in-memory tables.
For the latest documentation, visit Kdb+.

**Required server version**: DolphinDB 2.00.10 or higher

**Supported OS**: Windows x86-64, Linux x86-64, and Linux JIT

## Build from Source - Linux

**build zlib**

```
wget http://www.zlib.net/zlib-1.2.11.tar.gz
tar -zxf zlib-1.2.11.tar.gz
cd zlib-1.2.11
CFLAGS="-fPIC" ./configure  --prefix=/tmp/zlib
make
make install
```

copy /tmp/zlib/lib/libz.a to <path_to_kdb>/lib/linux

**build snappy**

download [snappy-release](https://github.com/google/snappy/tree/1.1.10)

```
mkdir build
cd build
cmake .. -DCMAKE_CXX_FLAGS="-fPIC"
make -j10
```
copy ./build/libsnappy.a to <path_to_kdb>/lib/linux

**build lz4**
```
wget https://github.com/lz4/lz4/releases/download/v1.9.4/lz4-1.9.4.tar.gz
tar -xzvf lz4-1.9.4.tar.gz
cd lz4-1.9.4
make
```
copy ./lib/liblz4.a to <path_to_kdb>/lib/linux

**build plugin**
```
cd /path/to/plugins/kdb
mkdir build
cd build
cmake ..
make
```

**Note**: Make sure libDolphinDB.so is under the GCC search path before compilation. You can add the plugin path to the library search path LD_LIBRARY_PATH or copy it to the build directory.