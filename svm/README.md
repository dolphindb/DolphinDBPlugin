# DolphinDB SVM Plugin

DolphinDB 提供了 SVM 插件，使用户可以在 DolphinDB 中对 DolphinDB 对象执行 SVM 模型的训练和预测。

SVM 插件基于 Libsvm [附录-参考文献](###参考文献) 进行实现，对常见的支持向量机算法进行了封装。

## 在插件市场安装插件

### 版本要求

DolphinDB Server: 2.00.10 及更高版本

注： 目前支持 x64 的 Linux 与 Windows 版本。

### 安装步骤

在DolphinDB 客户端中使用 listRemotePlugins 命令查看插件仓库中的插件信息。

``` Dolphin Script
login("admin", "123456")
listRemotePlugins()
```

使用 installPlugin 命令完成插件安装。

``` Dolphin Script
installPlugin("SVM")
```

返回：<path_to_SVM_plugin>/PluginSVM.txt

使用 loadPlugin 命令加载插件（即上一步返回的.txt文件）。

``` Dolphin Script
loadPlugin("SVM")
```


## 接口说明

### 1. svm::fit

根据给定的训练数据训练 SVM 模型

** 语法 **

```
svm::fit(Y, X, [para=None])
```

** 参数 **

- y: 目标值向量，元素类型统一为 int 类型或 double 类型。
- X: 输入的训练数据 (可以为矩阵、表、向量)，元素类型为 double。
  - 当 X 为矩阵的时候，每一列代表一个样本，列中的元素代表属性值。
  - 当 X 为表的时候，表中每列数据都必须是 double 类型，每一行表示一个样本。
  - 当 X 为向量的时候，fit 方法会根据 y 向量的长度将 X 均匀分成相应长度的样本。
- params: 一个 string-any 类型字典，表示 SVM 训练参数。它包括如下键值:
  * "type": 表示 SVM 类型。其值可以为 "NuSVC"、"NuSVR"、"OneClass"、"SVC"、"SVR"
  * "kernel": 表示核函数类型。其值可以为 "linear"、"poly"、"rbf"、"sigmoid"、"precomputed"
  * "degree": 表示核函数级数。其值为一个 int 值。
  * "gamma": 表示核函数的 gamma 参数。其值可以为 "scale" 或者 double 值。
  * "coef0": 表示核函数的 coef0 参数。其值为一个 double 值，默认为 0。
  * "C": 表示 C-SVC, epsilon-SVR, and nu-SVR 的 cost 参数。其值为一个 double 值，默认为 1。
  * "epsilon": 表示 epsilon-SVR 中的 epsilon 参数。其值为一个 double 值，默认为 0.1。
  * "shrinking": 表示是使用 shrinking heuristics。其值为一个布尔值。默认为 1。
  * "cache_size": 表示核函数缓存的大小。其值为一个 double 值，以 MB 为单位，默认为 100。
  * "verbose": 表示是否进行详细输出。其值为一个布尔值，默认为 True。
  * "nu": 表示边界误差的分数的上限）和支持向量的分数的下限。范围属于 (0,1]，其默认值为 0.5。
** 返回值 **

返回一个 SVM 对象。

### 2. svm::predict

根据 SVM 模型和测试数据进行分类或者回归

** 语法 **

```
svm::predict(SVMobject, X)
```

** 参数 **


- SVMobject: 一个 SVM 对象。
- X: 输入的测试数据，元素类型为 double。其类型可以为矩阵、表、向量。

** 返回值 **

返回一个向量，向量中的值为预测的样本标签值或回归值。

### 3. svm::score

根据给定的测试数据和标签值计算已有 SVM 的模型的准确性，并返回统计指标。其中 SVM 模型由一个 SVM 对象给出。

** 语法 **

```
svm::score(SVMobject, Y, X)
```

** 参数 **

- SVMobject: 一个 SVM 对象。
- Y: 真实目标值向量。
- X: 输入的测试数据矩阵。其类型可以为矩阵、表、向量。

** 返回值 **

如果是分类模型，返回预测的准确率。如果是回归模型，返回 MSE 和 R2。

### 4. svm::saveModel

将已经训练好的 SVM 模型保存。

** 语法 **

```
svm::saveModel(SVMobject, location)
```

** 参数 **

- SVMobject: 一个 SVM 对象。
- location: 一个字符串，表示文件路径。

** 返回值 **

一个布尔值，表示模型是否保存成功。

### 5. svm::loadModel

将文件形式的 SVM 模型导入到内存中。

** 语法 **

```
svm::loadModel(location)
```

** 参数 **

- location: 一个字符串，表示 SVM 模型文件路径。

** 返回值 **

一个 SVM 对象。

## 示例

### 例子 1 SVM 分类模型

训练模型：

```
path="/path/to/PluginSVM.txt";
modelPath="/path/to/mymodel"
loadPlugin(path)
X = matrix(-1.0 -1.0,-2.0 -1.0, 1.0 1.0, 2.0 1.0)
Y = 1.0 1.0 2.0 2.0
clf = svm::fit(Y, X)
```

用模型进行预测：

```
> svm::predict(clf, X)
[1,1,2,2]
```

评估模型：

```
> svm::score(clf, X);

1
```

将模型保存：

```
svm::saveModel(clf, modelPath)
```

### 例子 2 SVM 回归模型

训练模型：

```
path="/path/to/PluginSVM.txt";
modelPath="/path/to/mymodel";
loadPlugin(path);
X = table(1 3 5 7 11 16 23 as X)
Y = 0.1 4.2 5.6 8.8 22.1 35.6 77.2
regr = svm::fit(Y, X, {type: "SVR"})
```

评估模型：

```
> svm::score(regr, Y, X);

MSE->797.772
R2->0.582937
```

## 附录

### 自行构建

**Linux**

使用 GCC 4.8.5 进行编译。

``` shell
mkdir build
cd build
cmake ..
make
make install
```

**Windows**

使用 MinGW-W64 GCC-5.3.0 x86_64_win32-seh-rt 进行编译。

``` cmd
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
mingw32-make install
```

### 参考文献

1. Chih-Chung Chang and Chih-Jen Lin, LIBSVM : a library for support vector machines. ACM Transactions on Intelligent Systems and Technology, 2:27:1--27:27, 2011.  Software available at http://www.csie.ntu.edu.tw/~cjlin/libsvm

