# DolphinDB Feather Plugin

DolphinDB feather plugin supports efficient import and export of Feather files with automatic data type conversion.
For the latest documentation, visit Feather.

**Required server version**: DolphinDB 2.00.10 or higher

**Supported OS**: Windows x86-64 and Linux x86-64

## Build from Source - Linux

(1) Compile the Feather Development Kit.

```
git clone https://github.com/apache/arrow.git
cd arrow/cpp
mkdir build
cd build
cmake .. -DARROW_BUILD_STATIC=ON -DARROW_BUILD_SHARED=OFF -DARROW_DEPENDENCY_USE_SHARED=OFF -DARROW_WITH_ZLIB=ON -DARROW_WITH_ZSTD=ON -DARROW_WITH_LZ4=ON
make -j
```

(2) After compiling, copy the following files to the target directories.
| **Files**                                                    | **Target Directory** |
| ------------------------------------------------------------ | -------------------- |
| arrow/cpp/src/arrow                                          | ./include            |
| arrow/cpp/build/release/libarrow.a arrow/cpp/build/jemalloc_ep-prefix/src/jemalloc_ep/lib/libjemalloc_pic.a arrow/cpp/build/zstd_ep-install/lib64/libzstd.a arrow/cpp/build/zlib_ep/src/zlib_ep-install/lib/libz.a arrow/cpp/build/lz4_ep-prefix/src/lz4_ep/lib/liblz4.a | ./lib/linux          |

**Note**: If the files listed in the "Files" column do not exist during compilation, you can manually compile the following three libraries.

- If libz.a cannot be found, run the following command:

```
wget http://www.zlib.net/zlib-1.2.12.tar.gz
tar -zxf zlib-1.2.12.tar.gz
cd zlib-1.2.12
CFLAGS="-fPIC" ./configure
make
```
Find libz.a in the zlib-1.2.12 directory and put it to the ./lib/linux directory in the plugins folder.

- If liblz4.a cannot be found, run the following command:

```
wget https://github.com/lz4/lz4/archive/8f61d8eb7c6979769a484cde8df61ff7c4c77765.tar.gz
tar -xzvf 8f61d8eb7c6979769a484cde8df61ff7c4c77765.tar.gz
cd lz4-8f61d8eb7c6979769a484cde8df61ff7c4c77765/
make
```
Find libz.a in the lz4-8f61d8eb7c6979769a484cde8df61ff7c4c77765/lib directory and put it to the ./lib/linux directory in the plugins folder.

- If libzstd.a cannot be found, run the following command:

```
wget https://github.com/facebook/zstd/releases/download/v1.5.2/zstd-1.5.2.tar.gz
tar -zxvf zstd-1.5.2.tar.gz
cd zstd-1.5.2/
cd build/cmake/
mkdir build
cd build/
cmake ..
make -j
```

Find libz.a in the zstd-1.5.2/build/cmake/build/lib directory and put it to the ./lib/linux directory in the plugins folder.

(3) Compile the plugin.

```
cd /path/to/plugins/feather
mkdir build
cd build
cmake ..
make
```
**Note**: Make sure libDolphinDB.so is under the GCC search path before compilation. You can add the plugin path to the library search path LD_LIBRARY_PATH or copy it to the build directory.