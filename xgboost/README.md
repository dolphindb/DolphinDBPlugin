# XGBoost Plugin for DolphinDB

- [XGBoost Plugin for DolphinDB](#xgboost-plugin-for-dolphindb)
  - [1. Install the Plugin](#1-install-the-plugin)
    - [1.1 Compile on Linux](#11-compile-on-linux)
    - [1.2 Compile on Windows](#12-compile-on-windows)
  - [2. Interfaces](#2-interfaces)
    - [2.1 xgboost::train](#21-xgboosttrain)
    - [2.2 xgboost::predict](#22-xgboostpredict)
    - [2.3 xgboost::saveModel](#23-xgboostsavemodel)
    - [2.4 xgboost::loadModel](#24-xgboostloadmodel)
  - [3. Examples](#3-examples)


DolphinDB XGBoost plugin offers methods for model training and prediction with given DolphinDB tables. You can also use the methods to save or load the trained models.

## 1. Install the Plugin

You can download precompiled binaries *libPluginXgboost.dll* or *libPluginXgboost.so* to install the DolphinDB XGBoost Plugin directly. To manually compile a XGBoost plugin, please follow the instructions:

### 1.1 Compile on Linux 

- Build a static library

The following steps show how to compile a static library

(1) Download XGBoost from GitHub:

```
git clone --recursive https://github.com/dmlc/xgboost
```

(2) Use CMake to build a static library:

```
cd xgboost
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DBUILD_STATIC_LIB=ON
make
```

The static libraries are stored in *xgboost/lib/xgboost.a*, *xgboost/build/rabit/librabit.a*, *xgboost/build/dmlc-core/libdmlc.a*, respectively.

Create a directory of xgboost_static under the working directory of the project, and copy the header files as well as built libraries to the folder.

```
cd path_to/DolphinDBPlugin/xgboost
mkdir xgboost_static
cp path_to/xgboost/lib/xgboost.a xgboost_static/lib
cp path_to/xgboost/build/rabit/librabit.a xgboost_static/lib
cp path_to/xgboost/build/dmlc-core/libdmlc.a xgboost_static/lib
cp -r path_to/xgboost/include xgboost_static
```

Note: please replace the “path_to” in the above script with the plugin directory.

- Compile XGBoost plugin

Compile with CMakeLists

```
mkdir build
cd build
cmake .. -DLIBDOLPHINDB=path_to_libDolphinDB
make
```

Note: please replace the “path_to_libDolphinDB” in the above script with the path of DolphinDB server. If the file libDolphinDB.so is included in the g++ search path, then LIBDOLPHINDB is not required.

### 1.2 Compile on Windows

> Note: Currently the XGBoost plugin for Windows only supports DolphinDB server (JIT). It is recommended to compile with 8.1.0-posix version of MinGW-w64.

- Build a static library

(1) Download XGBoost 1.2.0 from GitHub:

```
git clone -b release_1.2.0 https://github.com/dmlc/xgboost.git
git submodule update --init --recursive
```

(2) Build dynamic and static libraries with CMake

```
cd xgboost
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
make
```

The libraries are stored in xgboost/lib/xgboost.dll, xgboost/build/rabit/libr abit.a, xgboost/build/dmlc-core/libdmlc.a, respectively.

Please note that the static files for Linux system are stored under the directory of xgboost_static provided by DolphinDB. For Windows users, please first delete the files under the folder, and then copy the relevant header files and static libraries built above to the folder. See the following code: 

```
cd path_to/DolphinDBPlugin/xgboost
mkdir xgboost_static
cp path_to/xgboost/lib/xgboost.dll xgboost_static/lib
cp path_to/xgboost/build/rabit/librabit.a xgboost_static/lib
cp path_to/xgboost/build/dmlc-core/libdmlc.a xgboost_static/lib
cp -r path_to/xgboost/include xgboost_static
```

Note: please replace the “path_to” in the above script with the plugin directory.

 

- Compile XGBoost Plugin

Compile with CMakeLists:

```
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DLIBDOLPHINDB=path_to_libDolphinDB
make
```

Note: please replace the “path_to_libDolphinDB” in the above script with the server directory. If the file libDolphinDB.dll is in the library search path, it is not necessary to specify LIBDOLPHINDB.

After compiling the plugin, please copy the following dependencies to the sibling directory of libPluginXgboost.dll.

Assuming it is under the build directory:

```
cp path_to/xgboost/lib/xgboost.dll ./
cp path_to/mingw64/bin/libgomp-1.dll ./
```

 

## 2. Interfaces

### 2.1 xgboost::train

**Syntax**

`xgboost::train(Y, X, [params], [numBoostRound=10], [xgbModel])`

**Parameters**

- **Y:** a vector indicating the dependent variables.
- **X:** a matrix or table indicating the independent variables.
- **params:** a dictionary representing the parameters used for XGBoost training. See [XGBoost Docs](https://xgboost.readthedocs.io/en/latest/parameter.html).
- **numBoostRound:** a positive integer indicating the number of boosting iterations.
- **model:** an XGBoost model (allows training continuation). You can obtain a model with `xgboost::train`, or load an existing model with `xgboost::loadModel`.

**Details**

Train the given table or matrix. Return the trained model which can be used for further training or prediction.

 

### 2.2 xgboost::predict

**Syntax**

`xgboost::predict(model, X, [outputMargin=false], [ntreeLimit=0], [predLeaf=false], [predContribs=false], [training=false])`

**Parameters**

- **model:** an XGBoost model used for prediction. You can obtain a model with `xgboost::train`, or load an existing model with `xgboost::loadModel`.
- **X:** a matrix or table for prediction
- **outputMargin:** A Boolean value indicating whether to output the raw untransformed margin value.
- **ntreeLimit:** a non-negative interger indicating which layer of trees are used in prediction. The default value is 0, indicating all trees are used.
- **predLeaf:** a Boolean value. When this option is on, the output will be a matrix of (nsample, ntrees) with each record indicating the predicted leaf index of each sample in each tree.
- **predContribs:** a Boolean value. When this is True the output will be a matrix of size (nsample, nfeats + 1) with each record indicating the feature contributions (SHAP values) for that prediction. The sum of all feature contributions is equal to the raw untransformed margin value of the prediction.
- **training:** a Boolean value indicating whether the prediction value is used for training.

**Details**

Predict with the given matrix or table.

 

### 2.3 xgboost::saveModel

**Syntax**

`xgboost::saveModel(model, path)`

**Parameters**

- **model:** an XGBoost model to be saved.
- **path:** a string indicating where the model is saved.

**Details**

Save the trained model to disk.

### 2.4 xgboost::loadModel

**Syntax**

`xgboost::loadModel(path)`

**Parameter**

- **path:** a string indicating where the model is saved.

**Details**

Load the model from disk.



## 3. Examples

```
loadPlugin("path_to/PluginXgboost.txt")

// Create a table for training
t = table(1..5 as c1, 1..5 * 2 as c2, 1..5 * 3 as c3)
label = 1 2 9 28 65

// Set params
params = {objective: "reg:linear", max_depth: 5, eta: 0.1, min_child_weight: 1, subsample: 0.5, colsample_bytree: 1, num_parallel_tree: 1}

// Train the model
model = xgboost::train(label, t, params, 100)

// Predict with the model
xgboost::predict(model, t)

// Save the model
xgboost::saveModel(model, WORK_DIR + "/xgboost001.model")

// Load the model
model = xgboost::loadModel(WORK_DIR + "/xgboost001.model")

// Continue training on the model
model = xgboost::train(label, t, params, 100, model)
```

 
