# DolphinDB SVM Plugin

DolphinDB SVM plugin enables users to train and predict SVM models directly on DolphinDB objects.
For the latest documentation, visit SVM.

**Required server version**: DolphinDB 2.00.10 or higher

**Supported OS**: Windows x86-64 and Linux x86-64

## Build from Source

### On Linux

(1) Install CMake and compile the plugin with gcc 4.8.5.

```
sudo apt-get install cmake
```

(2) Compile the plugin.

```
mkdir build
cd build
cmake ..
make
```

**Note**: Make sure libDolphinDB.so is under the GCC search path before compilation. You can add the plugin path to the library search path LD_LIBRARY_PATH or copy it to the build directory.

### On Windows

Compile the plugin using MinGW-W64 GCC-5.3.0 x86_64_win32-seh-rt.

```
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
mingw32-make install
```