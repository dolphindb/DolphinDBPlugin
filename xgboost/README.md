# DolphinDB XGBoost 插件

该插件可以调用 XGBoost 库函数，对 DolphinDB 的表执行训练、预测、模型保存和加载。

## 安装构建

可以直接使用我们已经编译好的位于 bin 目录下的 libPluginXgboost.dll 或 libPluginXgboost.so。如果你需要手动编译，请参考下面步骤：

### linux 环境下编译

#### 编译 XGBoost 静态库

首先，编译 XGBoost 静态库。步骤如下：

1. 从 GitHub 上下载最新的 XGBoost 项目：

```
git clone --recursive https://github.com/dmlc/xgboost
```

2. 使用 CMake 编译为静态库：

```
cd xgboost
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DBUILD_STATIC_LIB=ON
make
```

编译得到的静态库分别位于 xgboost/lib/xgboost.a, xgboost/build/rabit/librabit.a, xgboost/build/dmlc-core/libdmlc.a。

在本项目所在的路径下创建一个名为 xgboost_static 的目录，将相关头文件和上一步编译得到的静态库库复制到其中。

```
cd path_to/DolphinDBPlugin/xgboost
mkdir xgboost_static
cp path_to/xgboost/lib/xgboost.a xgboost_static/lib
cp path_to/xgboost/build/rabit/librabit.a xgboost_static/lib
cp path_to/xgboost/build/dmlc-core/libdmlc.a xgboost_static/lib
cp -r path_to/xgboost/include xgboost_static
```

注意：请修改 path_to 为插件所在的实际路径。

#### 编译 XGBoost 插件
使用 CMakeLists 编译：
```
mkdir build
cd build
cmake .. -DLIBDOLPHINDB=path_to_libDolphinDB
make
```
注意：请修改 path_to_libDolphinDB 为DolphinDB server的实际路径。若 libDolphinDB.so 在 g++ 搜索路径中，则不需要指定 LIBDOLPHINDB。

### Windows 环境下编译

**目前 xgboost 插件 windows 下仅支持 DolphinDB JIT 版本**
**推荐使用 8.1.0-posix 版本的 mingw64 进行编译**

#### 编译 XGBoost 静态库

需要先编译 XGBoost 静态库。步骤如下：

1. 从 GitHub 上下载 1.2.0 版本的 XGBoost 项目：

```
git clone -b release_1.2.0 https://github.com/dmlc/xgboost.git
git submodule update --init --recursive
```

2. 使用 CMake 编译为动态库与静态库：

```
cd xgboost
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
make
```

编译得到的动态库与静态库分别位于 xgboost/lib/xgboost.dll, xgboost/build/rabit/librabit.a, xgboost/build/dmlc-core/libdmlc.a。

请注意，因为 DolphinDB 提供的原始 xgboost_static 目录下存放了 Linux 系统的静态文件，所以对于 Windows 系统，需要先删除 xgboost_static 下的文件，然后将相关头文件和上一步编译得到的静态库库复制到其中，见下方代码：

```
cd path_to/DolphinDBPlugin/xgboost
mkdir xgboost_static
cp path_to/xgboost/lib/xgboost.dll xgboost_static/lib
cp path_to/xgboost/build/rabit/librabit.a xgboost_static/lib
cp path_to/xgboost/build/dmlc-core/libdmlc.a xgboost_static/lib
cp -r path_to/xgboost/include xgboost_static
```

注意：请修改 path_to 为插件所在的实际路径。

#### 编译 XGBoost 插件

使用 CMakeLists 编译：
```
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DLIBDOLPHINDB=path_to_libDolphinDB
make
```

注意：请修改 path_to_libDolphinDB 为你的环境中的实际路径，若 libDolphinDB.dll 在系统路径中则不需要指定 LIBDOLPHINDB。

在编译完成后需要将以下依赖库拷贝到 libPluginXgboost.dll 同级目录下，假设当前仍在 build 目录下。

```
cp path_to/xgboost/lib/xgboost.dll ./
cp path_to/mingw64/bin/libgomp-1.dll ./
```

## 用户接口

### `xgboost::train`

#### 语法

`xgboost::train(Y, X, [params], [numBoostRound=10], [model])`

#### 参数

- `Y`: 是一个向量，表示因变量。
- `X`: 是一个矩阵或一个表，表示自变量。
- `params`: 一个字典，表示 XGBoost 训练所用的参数，详情参考 [官方文档](https://xgboost.readthedocs.io/en/latest/parameter.html)。
- `numBoostRound`: 一个正整数，表示 boosting 的迭代次数。
- `model`: 一个 XGBoost 模型，用于继续训练。可以通过 `xgboost::train` 或 `xgboost::loadModel` 函数得到模型。

#### 详情

对给定的表调用 XGBoost 库函数进行训练。返回值是训练得到的模型，可用于继续训练或预测。

### `xgboost::predict`

#### 语法

`xgboost::predict(model, X, [outputMargin=false], [ntreeLimit=0], [predLeaf=false], [predContribs=false], [training=false])`

#### 参数

- `model`: 用于预测的 XGBoost 模型。可以通过 `xgboost::train` 或 `xgboost::loadModel` 函数得到模型。
- `X`: 是一个矩阵或一个表，表示用于预测的数据。
- `outputMargin`: 一个布尔值，表示是否输出原始的未经转换的边际值（raw untransformed margin value）。
- `ntreeLimit`: 一个非负整数，表示预测时使用的树的数量限制（默认值 0 表示使用所有树）。
- `predLeaf`: 一个布尔值。如果为 true，将返回一个形状为 (样本数, 树的个数) 的矩阵，每一条记录表示每一个样本在每一棵树中的预测的叶节点的序号。
- `predContribs`: 一个布尔值。如果为 true，将返回一个形状为 (样本数, 特征数 + 1) 的矩阵，每一条记录表示特征对预测的贡献（SHAP values）。所有特征贡献的总和等于未经转换的边际值（raw untransformed margin value）。
- `training`: 一个布尔值。表示预测值是否用于训练。

关于以上参数的具体用途说明，参见 [官方文档](https://xgboost.readthedocs.io/en/latest/python/python_api.html#xgboost.Booster.predict)。

#### 详情

对给定的矩阵或表调用 XGBoost 库函数进行预测。

### `xgboost::saveModel`

#### 语法

`xgboost::saveModel(model, path)`

#### 参数

- `model`: 用于保存的 XGBoost 模型。
- `path`: 一个字符串，表示保存的路径。

#### 详情

将训练得到的 XGBoost 模型保存到磁盘。

### `xgboost::loadModel`

#### 语法

`xgboost::loadModel(path)`

#### 参数

- `path`: 一个字符串，表示模型所在的路径。

#### 详情

从磁盘上加载 XGBoost 模型。

## 综合使用范例

```
loadPlugin("path_to/PluginXgboost.txt")

// 创建训练表
t = table(1..5 as c1, 1..5 * 2 as c2, 1..5 * 3 as c3)
label = 1 2 9 28 65

// 设置模型参数
params = {objective: "reg:linear", max_depth: 5, eta: 0.1, min_child_weight: 1, subsample: 0.5, colsample_bytree: 1, num_parallel_tree: 1}

// 训练模型
model = xgboost::train(label, t, params, 100)

// 用模型预测
xgboost::predict(model, t)

// 保存模型
xgboost::saveModel(model, WORK_DIR + "/xgboost001.model")

// 加载模型
model = xgboost::loadModel(WORK_DIR + "/xgboost001.model")

// 在已有模型的基础上继续训练
model = xgboost::train(label, t, params, 100, model)
```