# DolphinDB SVM Plugin

DolphinDB的SVM插件是基于libsvm<sup>[1](#参考文献)</sup>的插件，对常见的支持向量机算法进行了封装。用户可以在DolphinDB中对DolphinDB对象执行SVM模型的训练和预测。



## 构建

该插件使用CMake编译。

```
mkdir build
cd build
cmake ..
make
```


## 插件加载

编译生成 libPluginSVM.so 之后，通过以下脚本加载插件：

```
loadPlugin("/path/to/PluginSVM.txt");
```

# API
## 1. svm::fit
根据给定的训练数据训练SVM模型
### 语法

```
svm::fit(Y, X, [para=None])
```

### 参数
- y: 目标值向量，元素类型统一为int类型或double类型。
- X: 输入的训练数据(可以为矩阵、表、向量)，元素类型为double。
  - 当X为矩阵的时候，每一列代表一个样本，列中的元素代表属性值。
  - 当X为表的时候，表中每列数据都必须是double类型,每一行表示一个样本。
  - 当X为向量的时候，fit方法会根据y向量的长度将X均匀分成相应长度的样本。
- params: 一个string-any类型字典，表示SVM训练参数。它包括如下键值:
  * "type":表示SVM类型。其值可以为"NuSVC"、"NuSVR"、"OneClass"、"SVC"、"SVR"
  * "kernel":表示核函数类型。其值可以为"linear"、"poly"、"rbf"、"sigmoid"、"precomputed"
  * "degree":表示核函数级数。其值为一个int值。
  * "gamma":表示核函数的gamma参数。其值可以为"scale"或者double值。
  * "coef0":表示核函数的coef0参数。其值为一个double值，默认为0。
  * "C":表示C-SVC, epsilon-SVR, and nu-SVR的cost参数。其值为一个double值，默认为1。
  * "epsilon":表示epsilon-SVR中的epsilon参数。其值为一个double值，默认为0.1。
  * "shrinking":表示是使用shrinking heuristics。其值为一个布尔值。默认为1。
  * "cache_size":表示核函数缓存的大小。其值为一个double值，以MB为单位，默认为100。
  * "verbose":表示是否进行详细输出。其值为一个布尔值，默认为True。
  * "nu":表示边界误差的分数的上限）和支持向量的分数的下限。范围属于(0,1]，其默认值为0.5。
### 返回值

返回一个SVM对象。

## 2. svm::predict
根据SVM模型和测试数据进行分类或者回归

### 语法

```
svm::predict(SVMobject, X)
```

### 参数

- SVMobject: 一个SVM对象。
- X: 输入的测试数据，元素类型为double。其类型可以为矩阵、表、向量。

### 返回值

返回一个向量，向量中的值为预测的样本标签值或回归值。

## 3. svm::score
根据给定的测试数据和标签值计算已有SVM的模型的准确性，并返回统计指标。其中SVM模型由一个SVM对象给出。

### 语法

```
svm::score(SVMobject, Y, X)
```

### 参数

- SVMobject: 一个SVM对象。
- Y: 真实目标值向量。
- X: 输入的测试数据矩阵。其类型可以为矩阵、表、向量。

### 返回值

如果是分类模型，返回预测的准确率。如果是回归模型，返回MSE和R2。

## 4. svm::saveModel
将已经训练好的SVM模型保存。
### 语法

```
svm::saveModel(SVMobject, location)
```

### 参数

- SVMobject: 一个SVM对象。
- location: 一个字符串，表示文件路径。

### 返回值

一个布尔值，表示模型是否保存成功。

## 5. svm::loadModel
将文件形式的SVM模型导入到内存中。
### 语法

```
svm::loadModel(location)
```

### 参数

- location: 一个字符串，表示SVM模型文件路径。

### 返回值

一个SVM对象。

# 示例

## 例子1 SVM分类模型

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

## 例子2 SVM回归模型

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

##### 参考文献：

1. Chih-Chung Chang and Chih-Jen Lin, LIBSVM : a library for support vector machines. ACM Transactions on Intelligent Systems and Technology, 2:27:1--27:27, 2011.  Software available at http://www.csie.ntu.edu.tw/~cjlin/libsvm

# ReleaseNotes:

## 故障修复

* 修复了多线程执行 svm::fit 导致 server 宕机的问题。（**1.30.22**）

# 功能优化

* 接口 svm::fit 新增对参数 *params* 中键值"nu"输入值范围的检查。（**1.30.22**）
