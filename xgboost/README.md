# XGBoost Plugin for DolphinDB

DolphinDB XGBoost plugin offers methods for model training and prediction with given DolphinDB tables. You can also use the methods to save or load the trained models.
For the latest documentation, visit xgboost.

**Required server version**: DolphinDB 2.00.10 or higher

**Supported OS**:

- XGBoost 1.2: Linux x86-64, Windows x86-64 JIT.

- XGBoost 3.1: Linux x86-64 ABI=1.

## Build from Source

### On Linux

**Build XGBoost Static Library**

You need to build the XGBoost static library first. The steps are as follows:

1. Download the XGBoost project from GitHub:

```
git clone --recursive https://github.com/dmlc/xgboost
```

2. Use CMake to build the static library:

Currently, two versions of XGBoost are supported: version 1.2 and version 3.1. You need to switch to the corresponding release tag before building.

- For version 1.2, the minimum GCC version required is 5.0.
- For version 3.1, the minimum GCC version required is 8.1.

```bash
cd xgboost
mkdir build && cd build
cmake .. -DBUILD_STATIC_LIB=ON -DCMAKE_INSTALL_PREFIX=<artifact_path>
cmake --build . -j --verbose
cmake --install .
```

After the build is complete, copy the files from `<artifact_path>` to the corresponding folder under `Plugin/thirdParty/`.  
For version 3.1 on Linux, copy the files to `3.1_linux`.  
For version 1.2, copy the files to `1.2_linux` for Linux builds or `1.2_win` for Windows builds.

**Build the XGBoost Plugin**

```bash
cd xgboost
export CMAKE_INSTALL_PREFIX="$(pwd)/output"
bash build.sh Release ./
```

The build artifacts will be located in the output folder with the following structure:

```
.
└── xgboost
    ├── 1.2
    │   ├── libgomp.so.1
    │   ├── libPluginXgboost.so
    │   └── PluginXgboost.txt
    └── 3.1
        ├── libgomp.so.1
        ├── libPluginXgboost.so
        └── PluginXgboost.txt
```