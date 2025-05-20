# XGBoost Plugin for DolphinDB

DolphinDB XGBoost plugin offers methods for model training and prediction with given DolphinDB tables. You can also use the methods to save or load the trained models.
For the latest documentation, visit xgboost.

**Required server version**: DolphinDB 2.00.10 or higher

**Supported OS**:

- XGBoost 1.2: Linux x86-64, Windows x86-64 JIT.

- XGBoost 2.0: Linux x86-64 ABI=1.

## Build from Source

### On Linux

(1) Download XGBoost.

```
git clone --recursive https://github.com/dmlc/xgboost
```

(2) Use CMake to build a static library.

```
cd xgboost
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DBUILD_STATIC_LIB=ON
make
```

(3) Create a directory of xgboost_static under the working directory of the project, and copy the header files as well as built libraries to the folder.

```
cd path_to/DolphinDBPlugin/xgboost
mkdir xgboost_static
cp path_to/xgboost/lib/xgboost.a xgboost_static/lib
cp path_to/xgboost/build/rabit/librabit.a xgboost_static/lib
cp path_to/xgboost/build/dmlc-core/libdmlc.a xgboost_static/lib
cp -r path_to/xgboost/include xgboost_static
```

(4) Compile the plugin.

```
mkdir build
cd build
cmake .. -DLIBDOLPHINDB=path_to_libDolphinDB
make
```

### On Windows

(1) Download XGBoost 1.2.0.

```
git clone -b release_1.2.0 https://github.com/dmlc/xgboost.git
git submodule update --init --recursive
```

(2) Build dynamic and static libraries with CMake.

```
cd xgboost
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
make
```

(3) Delete the static files for Linux system under the directory of xgboost_static and copy the relevant header files and static libraries built above to the folder.

```
cd path_to/DolphinDBPlugin/xgboost
mkdir xgboost_static
cp path_to/xgboost/lib/xgboost.dll xgboost_static/lib
cp path_to/xgboost/build/rabit/librabit.a xgboost_static/lib
cp path_to/xgboost/build/dmlc-core/libdmlc.a xgboost_static/lib
cp -r path_to/xgboost/include xgboost_static
```

(4) Compile the plugin.

```
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DLIBDOLPHINDB=path_to_libDolphinDB
make
```

(5) Copy the following dependencies to the sibling directory of libPluginXgboost.dll.

```
cp path_to/xgboost/lib/xgboost.dll ./
cp path_to/mingw64/bin/libgomp-1.dll ./
```