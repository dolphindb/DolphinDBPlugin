# DolphinDB XGBoost 插件

该插件可以调用XGBoost库函数，对DolphinDB的表执行训练、预测、模型保存和加载。

## 安装构建

可以直接使用我们已经编译好的libPluginXgboost.dll或libPluginXgboost.so。如果你需要手动编译，请参考下面步骤：

### 编译XGBoost静态库

需要先编译XGBoost静态库。步骤如下：

1. 从GitHub上下载最新的XGBoost项目：

```
git clone --recursive https://github.com/dmlc/xgboost
```

2. 使用CMake编译为静态库：

cmake的最低版本要求为3.13。gcc的最低版本要求我5.0。

```
cd xgboost
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DBUILD_STATIC_LIB=ON
make
```


编译得到静态库分别位于xgboost/lib/xgboost.a, xgboost/build/rabit/librabit.a, xgboost/build/dmlc-core/libdmlc.a。

### 编译XGBoost插件

在本项目所在的路径下创建一个名为xgboost_static的目录，将相关头文件和上一步编译得到静态库库复制到其中。

```
cd path_to/DolphinDBPlugin/xgboost
mkdir xgboost_static
cp path_to/xgboost/lib/xgboost.a xgboost_static/lib
cp path_to/xgboost/build/rabit/librabit.a xgboost_static/lib
cp path_to/xgboost/build/dmlc-core/libdmlc.a xgboost_static/lib
cp -r path_to/xgboost/include xgboost_static
```

（注意需要将路径中的path_to改为你的环境中的实际路径）
#### 在Windows下编译

直接使用Makefile编译：

```
make
```
使用CMakeLists编译：
```
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DLIBDOLPHINDB=path_to_libDolphinDB
make
```
（注意需要将path_to_libDolphinDB改为你的环境中的实际路径，若libDolphinDB.dll在系统路径中则不需要指定LIBDOLPHINDB）

#### 在Linux下编译
使用CMakeLists编译：
```
mkdir build
cd build
cmake .. -DLIBDOLPHINDB=path_to_libDolphinDB
make
```
（注意需要将path_to_libDolphinDB改为你的环境中的实际路径，若libDolphinDB.so在g++搜索路径中则不需要指定LIBDOLPHINDB）

## 用户接口

### `xgboost::train`

#### 语法

`xgboost::train(Y, X, [params], [numBoostRound=10], [xgbModel])`

#### 参数

- `Y`: 是一个向量，表示因变量。
- `X`: 是一个矩阵或一个表，表示自变量。
- `params`: 一个字典，表示XGBoost训练所用的参数，详情参考[官方文档](https://xgboost.readthedocs.io/en/latest/parameter.html)。
- `numBoostRound`: 一个正整数，表示boosting的迭代次数。
- `xgbModel`: 一个XGBoost模型，允许继续训练。可以通过`xgboost::train`训练得到模型，或通过`xgboost::loadModel`加载已有模型。

#### 详情

对给定的表调用XGBoost库函数进行训练。返回值是训练得到的模型，可以用于继续训练或预测。

### `xgboost::predict`

#### 语法

`xgboost::predict(model, X, [outputMargin=false], [ntreeLimit=0], [predLeaf=false], [predContribs=false], [training=false])`

#### 参数

- `model`: 用于预测的XGBoost模型。可以通过`xgboost::train`或`xgboost::loadModel`函数得到模型。
- `X`: 是一个矩阵或一个表，表示用于预测的数据。
- `outputMargin`: 一个布尔值，表示是否输出原始的未经转换的边际值（raw untransformed margin value）。
- `ntreeLimit`: 一个非负整数，表示预测时使用的树的数量限制（默认值0表示使用所有树）。
- `predLeaf`: 一个布尔值。如果为true，将返回一个形状为(样本数, 树的个数)的矩阵，每一条记录表示每一个样本在每一棵树中的预测的叶节点的序号。
- `predContribs`: 一个布尔值。如果为true，将返回一个形状为(样本数, 特征数+1)的矩阵，每一条记录表示特征对预测的贡献（SHAP values）。所有特征贡献的总和等于未经转换的边际值（raw untransformed margin value）。
- `training`: 一个布尔值。表示预测值是否用于训练。

关于以上参数的具体用途说明，参见[官方文档](https://xgboost.readthedocs.io/en/latest/python/python_api.html#xgboost.Booster.predict)。

#### 详情

对给定的表调用XGBoost库函数进行预测。

### `xgboost::saveModel`

#### 语法

`xgboost::saveModel(model, fname)`

#### 参数

- `model`: 用于保存的XGBoost模型。
- `fname`: 一个字符串，表示保存的路径。

#### 详情

将训练得到的XGBoost模型保存到磁盘。

### `xgboost::loadModel`

#### 语法

`xgboost::loadModel(fname)`

#### 参数

- `fname`: 一个字符串，表示模型所在的路径。

#### 详情

从磁盘上加载XGBoost模型。

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
xgboost::saveModel(model, "001.model")

// 加载模型
model = xgboost::loadModel("001.model")

// 在已有模型的基础上继续训练
model = xgboost::train(label, t, params, 100, model)
```
