# DolphinDB HDF5 Plugin

HDF5 (Hierarchical Data Format) 是一种常见的跨平台数据储存文件，可以表示非常复杂、异构的数据对象。DolphinDB 提供了 HDF5 插件，可以查看 HDF5 文件元数据，读写 HDF5 文件并自动转换数据类型。

本文档仅介绍编译构建方法。通过 [文档中心-hdf5](https://docs.dolphindb.cn/zh/plugins/hdf5/hdf5.html) 查看接口介绍；通过 [CHANGELOG.md](./CHANGELOG.md) 查看版本发布记录。

## 编译构建

#### Linux 编译构建

安装 cmake：
```bash
sudo apt install cmake
```
编译 c-blosc
```bash
#在 https://github.com/Blosc/c-blosc/releases/tag/v1.21.1 下载源码
cd c-blosc-1.21.1
mkdir build
cd build
cmake -DCMAKE_C_FLAGS="-fPIC -std=c11" ..
make -j
cp blosc/src/blosc.h /path_to_hdf5_plugin/include/c-blosc
cp blosc/src/blosc-export.h /path_to_hdf5_plugin/include/c-blosc
cp blosc/libblosc.a /path_to_hdf5_plugin/lib
```

编译 HDF5 1.13.1：
```bash
# git clone https://github.com/HDFGroup/hdf5/ -b hdf5-1_13_1
# 若您不熟悉插件源代码，请不要下载其他版本，hdf5 版本兼容性差，可能导致安装失败
cd hdf5-1.13.1
export CC=gcc-4.8
export CXX=g++-4.8
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
编译整个项目：
```bash
mkdir build
cd build
cp /path_to_dolphindb/libDolphinDB.so ./
cmake ..
make
```

### Windows 编译构建

在 msys2 环境下，使用 mingw 编译 hdf5 1.13.1.zip
+ 使用 mingw-w64-x86_64 编译器
```
export PATH=/PATH_to mingw-w64/x86_64-5.3.0-win32-seh-rt_v4-rev0/mingw64/bin/:$PATH
```
+ 安装 make
```
pacman -S make
```

+ 打开 msys2 终端，进入已经解压好的 hdf5-1.13.1 所在目录
```
 CFLAGS="-std=c11" CXXFLAGS="-std=c++11" ./configure --host=x86_64-w64-mingw32 --build=x86_64-w64-mingw32 --prefix=/d/hdf5_1.13.1 --enable-cxx --enable-tests=no --enable-tools=no with_pthread=no
```
+ 打开 src/H5pubconf.h,在末尾添加以下宏定义
    ```
    #ifndef H5_HAVE_WIN32_API
    #define H5_HAVE_WIN32_API 1
    #endif

    #ifndef H5_HAVE_MINGW
    #define H5_HAVE_MINGW 1
    #endif
    ```
+ 开始编译
```
make -j8
make install -j8
```

+ 拷贝编译文件到 hdf5 插件文件目录
```
cp $HOME/hdf5/include/* /path_to_hdf5_plugin/include_win/hdf5
cp $HOME/hdf5/lib/libhdf5.a /path_to_hdf5_plugin/build
cp $HOME/hdf5/lib/libhdf5_cpp.a /path_to_hdf5_plugin/build
cp $HOME/hdf5/lib/libhdf5_hl.a /path_to_hdf5_plugin/build
cp $HOME/hdf5/lib/libhdf5_hl_cpp.a /path_to_hdf5_plugin/build
```

+ 打开 Windows 的 CMD 命令行终端

+ 编译 c_blosc
```
cd c_blosc-1.21.1
mkdir build
cd build
cmake  ../ -G "MinGW Makefiles"
mingw32-make -j8
copy .\blosc\libblosc.a /path_to_hdf5_plugin\build\
```
+ 复制 libDolphinDB.dll 到 hdf5 编译目录
```
copy /path_to_dolphindb/libDolphinDB.dll /path_to_hdf5_plugin/build
```

+ hdf5 插件编译
```
cmake  ../ -G "MinGW Makefiles"
mingw32-make -j
```