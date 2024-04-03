# DolphinDB XGBoost 插件

该插件可以调用 XGBoost 库函数，对 DolphinDB 的表执行训练、预测、模型保存和加载。

XGBoost 插件目前支持版本：[relsease200](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/xgboost/README_CN.md), [release130](https://github.com/dolphindb/DolphinDBPlugin/blob/release130/xgboost/README_CN.md), [release120](https://github.com/dolphindb/DolphinDBPlugin/blob/release120/xgboost/README_CN.md)。您当前查看的插件版本为 release200，请使用 DolphinDB 2.00.X 版本 server。若使用其它版本 server，请切换至相应插件分支。

目前提供两个 xgboost 版本的支持，分别为 1.2 和 2.0，两个版本由于默认参数设置存在差别，计算结果也会有一定差别。

注意 1.2 版本的插件只能在 JIT 版本 server 上使用，2.0 版本的插件只能在 ABI=1 版本 server 上使用。

## 1. 安装构建

可以直接使用我们已经编译好的 libPluginXgboost.dll 或 libPluginXgboost.so。如果需要手动编译，请参考下面步骤：

### 1.1 编译 XGBoost 静态库

需要先编译 XGBoost 静态库。步骤如下：

1. 从 GitHub 上下载 XGBoost 项目：

```
git clone --recursive https://github.com/dmlc/xgboost
```

2. 使用 CMake 编译为静态库：

目前支持 xgboost 的两个版本，一个是 1.2 版本，一个是 2.0 版本，需要切到对应 release tag 下进行编译，cmake 的最低版本要求为 3.13 。

编译 1.2 版本时，GCC 的最低版本要求为 5.0。编译 2.0 版本时，GCC 的最低版本为 8.1。

```
cd xgboost
mkdir build
cd build
cmake .. -DBUILD_STATIC_LIB=ON
make
```

xgboost 1.2 版本编译得到静态库分别位于 xgboost/lib/xgboost.a, xgboost/build/rabit/librabit.a, xgboost/build/dmlc-core/libdmlc.a。

xgboost 2.0 版本编译得到静态库分别位于 xgboost/lib/xgboost.a, xgboost/build/dmlc-core/libdmlc.a。

### 1.2 编译 XGBoost 插件

在本项目所在的路径下创建一个名为 xgboost_static 的目录，将相关头文件和上一步编译得到静态库库复制到其中。

``` bash
cd path_to/DolphinDBPlugin/xgboost
cp path_to/xgboost/lib/libxgboost.a ./lib/1.2/
cp path_to/xgboost/build/rabit/librabit.a ./lib/1.2/
cp path_to/xgboost/build/dmlc-core/libdmlc.a ./lib/1.2/
cp -r path_to/xgboost/include ./include/1.2
```

``` bash
cd path_to/DolphinDBPlugin/xgboost
cp path_to/xgboost/lib/libxgboost.a ./lib/2.0/
cp path_to/xgboost/build/dmlc-core/libdmlc.a ./lib/2.0/
cp -r path_to/xgboost/include ./include/2.0/
```

（注意需要将路径中的 path_to 改为你的环境中的实际路径）

#### 1.2.1 在 Windows 下编译

编译 1.2 版本时，GCC 的最低版本要求为 5.0。编译 2.0 版本时，GCC 的最低版本为 8.1，linux 与 windows 编译要求相同。

其中 1.2 版本的插件只能在 JIT 版本 server 上使用，2.0 版本的插件只能在 ABI=1 版本 server 上使用。

使用 CMakeLists 编译：
``` bash
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DLIBDOLPHINDB=path_to_libDolphinDB
mingw32-make
```
（注意需要将 path_to_libDolphinDB 改为你的环境中的实际路径，若 libDolphinDB.dll 在系统路径中则不需要指定 LIBDOLPHINDB）

#### 1.2.2 在 Linux 下编译

使用 CMakeLists 编译：
``` bash
mkdir build
cd build
cmake .. -DLIBDOLPHINDB=path_to_libDolphinDB
make
```
（注意需要将 path_to_libDolphinDB 改为你的环境中的实际路径，若 libDolphinDB.so 在 g++ 搜索路径中则不需要指定 LIBDOLPHINDB）

## 2. 用户接口

### 2.1 xgboost::train

#### 语法

xgboost::train(Y, X, [params], [numBoostRound=10], [xgbModel])

#### 参数

- Y: 一个向量，表示因变量。
- X: 一个矩阵或一个表，表示自变量。
- params: 一个字典，表示 XGBoost 训练所用的参数，详情参考 [官方文档](https://xgboost.readthedocs.io/en/latest/parameter.html)。
- numBoostRound: 一个正整数，表示 boosting 的迭代次数。
- xgbModel: 一个 XGBoost 模型，允许继续训练。可以通过 xgboost::train 训练得到模型，或通过 xgboost::loadModel 加载已有模型。

#### 详情

对给定的表调用 XGBoost 库函数进行训练。返回值是训练得到的模型，可以用于继续训练或预测。

### 2.2.1 xgboost::predict（xgboost 1.2 版本）

#### 语法

xgboost::predict(model, X, [outputMargin=false], [ntreeLimit=0], [predLeaf=false], [predContribs=false], [training=false])

#### 参数

- model: 用于预测的 XGBoost 模型。可以通过 xgboost::train 或 xgboost::loadModel 函数得到模型。
- X: 一个矩阵或一个表，表示用于预测的数据。
- outputMargin: 一个布尔值，表示是否输出原始的未经转换的边际值（raw untransformed margin value）。
- ntreeLimit: 一个非负整数，表示预测时使用的树的数量限制（默认值 0 表示使用所有树）。
- predLeaf: 一个布尔值。如果为 true，将返回一个形状为 (样本数，树的个数) 的矩阵，每一条记录表示每一个样本在每一棵树中的预测的叶节点的序号。
- predContribs: 一个布尔值。如果为 true，将返回一个形状为 (样本数，特征数 + 1) 的矩阵，每一条记录表示特征对预测的贡献（SHAP values）。所有特征贡献的总和等于未经转换的边际值（raw untransformed margin value）。
- training: 一个布尔值。表示预测值是否用于训练。

关于以上参数的具体用途说明，参见 [官方文档](https://xgboost.readthedocs.io/en/latest/python/python_api.html#xgboost.Booster.predict)。

#### 详情

对给定的表调用 XGBoost 库函数进行预测。

### 2.2.2 xgboost::predict（xgboost 2.0 版本）

#### 语法

xgboost::predict(model, X, [type=0], [iterationPair], [strictShape=false], [training=false])

#### 参数

- model: 用于预测的 XGBoost 模型。可以通过 xgboost::train 或 xgboost::loadModel 函数得到模型。
- X: 一个矩阵或一个表，表示用于预测的数据。
- type: 一个整型，可以为 0-6。具体含义为 0：normal prediction，1：output margin，2：predict contribution，3：predict approximated contribution，4：predict feature interaction，5：predict approximated feature interaction，6：predict leaf "training"。
- iterationPair: 一个整型数据对，第一个数字为开始的迭代次数，第二个数字为结束的迭代次数。
- strictShape: 一个布尔值，指示是否要以严格规则输出结果。
- training: 一个布尔值。表示预测值是否用于训练。

关于以上参数的具体用途说明，参见 [官方文档](https://xgboost.readthedocs.io/en/latest/c.html)。

#### 详情

对给定的表调用 XGBoost 库函数进行预测。

###  2.3 xgboost::saveModel

#### 语法

xgboost::saveModel(model, fname)

#### 参数

- model: 用于保存的 XGBoost 模型。
- fname: 一个字符串，表示保存的路径。

#### 详情

将训练得到的 XGBoost 模型保存到磁盘。

### 2.4 xgboost::loadModel

#### 语法

xgboost::loadModel(fname)

#### 参数

- fname: 一个字符串，表示模型所在的路径。

#### 详情

从磁盘上加载 XGBoost 模型。

## 3. 使用范例

注意，若使用 Windows 插件，加载时必须指定绝对路径，且路径中使用 "\\\\" 或 "/" 代替 "\\"。
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
