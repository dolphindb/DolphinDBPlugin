# DolphinDB XGBoost 插件

XGBoost（eXtreme Gradient Boosting）是一个用于建立梯度提升树（Gradient Boosting Decision Trees，GBDT）模型的开源机器学习库。xgboost 插件可以调用 XGBoost 库函数，对 DolphinDB 的表执行训练、预测、模型保存和加载。插件基于 [xgboost 开源库](https://github.com/dmlc/xgboost) 开发。

目前提供两个 xgboost 版本的支持，分别为 1.2 和 2.0，两个版本由于默认参数设置存在差别，计算结果也会有一定差别。

本文档仅介绍编译构建方法。通过 [文档中心-xgboost](https://docs.dolphindb.cn/zh/plugins/xgboost/xgboost.html) 查看接口介绍；通过 [CHANGELOG.md](./CHANGELOG.md) 查看版本发布记录。

## 编译构建

#### Linux 编译构建

可以直接使用我们已经编译好的 libPluginXgboost.dll 或 libPluginXgboost.so。如果需要手动编译，请参考下面步骤。

编译 1.2 版本时，GCC 的最低版本要求为 5.0。编译 2.0 版本时，GCC 的最低版本为 8.1，linux 与 windows 编译要求相同。这里介绍 Linux 编译方法。

**编译 XGBoost 静态库**

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

**编译 XGBoost 插件**

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


**在 Linux 下编译**

使用 CMakeLists 编译：
``` bash
mkdir build
cd build
cmake .. -DXGBOOST_VERSION=1.2 # 指定要编译的 xgboost 版本，可以为 1.2 或 2.0
make
```
