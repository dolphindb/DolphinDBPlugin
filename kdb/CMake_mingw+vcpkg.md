# kdb+ Plugin for DolphinDB using CMake & MinGW

## Requirements

### 1. MinGW-w64

https://www.mingw-w64.org/downloads/#mingw-builds

- In order to be consistent with DolphinDB, it is recommended to use the `x86_64`/`win32`/`seh` build of MinGW-w64.

- The following version of MinGW-w64 was considered:
  
     - The original kdb+ plugin on Windows was built with `x86_64-5.3.0-win32-seh-rt_v4-rev0.zip`

### 2. CMake

https://cmake.org/download/

Alternatively, the build-in CMake within Visual Studio can also be used.

### 3. vcpkg

https://vcpkg.io/en/getting-started

A short version of how to install vcpkg:

```batch
git clone https://github.com/Microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat
SET VCPKG_ROOT=%CD%\vcpkg
%VCPKG_ROOT%\vcpkg integrate install
```

## Build Steps

The follow commands assume that you are under the `DolphinDBPlugin\kdb\` directory.

### 1. Install dependencies

```batch
%VCPKG_ROOT%\vcpkg install zlib:x64-windows
```

```batch
SET DOLPHINDB_ROOT=<path_to_DolphinDB>
```

### 2. Build using CMake

```batch
del /S /Q build

:: Release mode
cmake -S . -B build -G Ninja
:: Debug mode
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
:: RelWithDebInfo mode
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo

cmake --build build -j 4
cmake --install build
```
