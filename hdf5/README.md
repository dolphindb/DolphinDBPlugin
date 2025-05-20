# DolphinDB HDF5 Plugin

DolphinDB hdf5 plugin imports HDF5 datasets into DolphinDB and supports data type conversions.
For the latest documentation, visit HDF5.

**Required server version**: DolphinDB 2.00.10 or higher

**Supported OS**: Windows and Linux x86-64

## Build from Source

### On Linux

(1) Install CMake

```
sudo apt install cmake
```

(2) Install c-blosc

```
#download source code at https://github.com/Blosc/c-blosc/releases/tag/v1.21.1
cd c-blosc-1.21.1
mkdir build
cd build
cmake -DCMAKE_C_FLAGS="-fPIC -std=c11" ..
make -j
cp blosc/src/blosc.h /path_to_hdf5_plugin/include/c-blosc
cp blosc/src/blosc-export.h /path_to_hdf5_plugin/include/c-blosc
cp blosc/libblosc.a /path_to_hdf5_plugin/lib
```

(3) Install HDF5 development kit

```
# download source code at https://portal.hdfgroup.org/display/support/HDF5+1.10.6#files
# DO NOT download other versions unless you are familar with the source code of plugin
tar -xvf hdf5-1.10.6.tar.gz
cd hdf5-1.10.6
export CFLAGS="-fPIC -std=c11"
export CXXFLAGS="-fPIC -std=c++11"
./configure --enable-cxx
make
make check
make install
make check-install
cp hdf5/include/* /path_to_hdf5_plugin/include/
cp hdf5/lib/libhdf5.a /path_to_hdf5_plugin/lib
cp hdf5/lib/libhdf5_cpp.a /path_to_hdf5_plugin/lib
cp hdf5/lib/libhdf5_hl.a /path_to_hdf5_plugin/lib
```

(4) Compile the plugin.

```
mkdir build
cd build
cp /path_to_dolphindb/libDolphinDB.so ./
cmake ..
make
```

**Note**: Make sure libDolphinDB.so is under the GCC search path before compilation. You can add the plugin path to the library search path LD_LIBRARY_PATH or copy it to the build directory.

### On Windows

(1) Install make

```
pacman -S make
```

(2) Open the MSYS2 terminal and navigate into the directory where hdf5-1.13.1 is decompressed

```
CFLAGS="-std=c11" CXXFLAGS="-std=c++11" ./configure --host=x86_64-w64-mingw32 --build=x86_64-w64-mingw32 --prefix=/d/hdf5_1.13.1 --enable-cxx --enable-tests=no --enable-tools=no with_pthread=no
```

(3) Open src/H5pubconf.h and add the following #define in the end of it

```
#ifndef H5_HAVE_WIN32_API
#define H5_HAVE_WIN32_API 1
#endif

#ifndef H5_HAVE_MINGW
#define H5_HAVE_MINGW 1
#endif
```

(4) Start compiling

```
make -j8
make install -j8
```

(5) Copy the compiled file to the HDF5 plugin directory

```
cp $HOME/hdf5/include/* /path_to_hdf5_plugin/include_win/hdf5
cp $HOME/hdf5/lib/libhdf5.a /path_to_hdf5_plugin/build
cp $HOME/hdf5/lib/libhdf5_cpp.a /path_to_hdf5_plugin/build
cp $HOME/hdf5/lib/libhdf5_hl.a /path_to_hdf5_plugin/build
cp $HOME/hdf5/lib/libhdf5_hl_cpp.a /path_to_hdf5_plugin/build
```

(6) Open Command Prompt Build c_blosc

```
cd c_blosc-1.21.1
mkdir build
cd build
cmake  ../ -G "MinGW Makefiles"
mingw32-make -j8
copy ./blosc/libblosc.a /path_to_hdf5_plugin/build/
```

(7) Copy libDolphinDB.dll to HDF5 plugin directory.

```
copy /path_to_dolphindb/libDolphinDB.dll /path_to_hdf5_plugin/build
```

(8) Compile the plugin.

```
cmake  ../ -G "MinGW Makefiles"
mingw32-make -j
```