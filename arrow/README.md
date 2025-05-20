# Arrow
DolphinDB Arrow plugin enables users to use the Arrow format to interact with the DolphinDB server through Python API with automatic data type conversion.

For the latest documentation, visit [Arrow](https://docs.dolphindb.cn/en/Plugins/formatArrow.html).

**Required server version**: DolphinDB 2.00.12 or higher

**Supported OS**: Windows x86-64 and Linux x86-64

## Build from Source
### On Linux
(1) Install CMake and g++ 4.8.5.

(2) Compile Arrow plugin development kit.


```bash
git clone <https://github.com/apache/arrow.git>
cd arrow/cpp
mkdir build
cd build
cmake .. 
make -j
```

(3) Save arrow/cpp/src/arrow to the include directory and arrow/cpp/build/release/libarrow.so.900 to the build directory.

(4) Compile the plugin.


```bash
cd /path/to/plugins/formatArrow
mkdir build
cd build
cmake ..
make
```
**Note:** Make sure libDolphinDB.so and libarrow.so.900 is under the GCC search path before compilation. You can add the plugin path to the library search path LD_LIBRARY_PATH or copy it to the build directory.

## On Windows
(1) Install [CMake](https://cmake.org/) and [MinGW](https://www.mingw.org/).

(2) Add the bin directories of MinGW and CMake to your PATH on Windows.

(3) Copy the file libDolphinDB.dll to the build directory and compile the plugin.


```bash
cd DolphinDBPlugin/Arrow
mkdir build
cp /path_to_dolphindb/libDolphinDB.dll ./
cp /path_to_dolphindb/libarrow.dll ./
cmake -G "MinGW Makefiles" ..
make -j
```