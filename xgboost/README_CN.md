# DolphinDB XGBoost 插件

XGBoost（eXtreme Gradient Boosting）是一个用于建立梯度提升树（Gradient Boosting Decision Trees，GBDT）模型的开源机器学习库。xgboost 插件可以调用 XGBoost 库函数，对 DolphinDB 的表执行训练、预测、模型保存和加载。插件基于 [xgboost 开源库](https://github.com/dmlc/xgboost) 开发。

目前提供两个 xgboost 版本的支持，分别为 1.2 和 3.1，两个版本由于默认参数设置存在差别，计算结果也会有一定差别。

本文档仅介绍编译构建方法。通过 [文档中心-xgboost](https://docs.dolphindb.cn/zh/plugins/xgboost/xgboost.html) 查看接口介绍；通过 [CHANGELOG.md](./CHANGELOG.md) 查看版本发布记录。

## 编译构建

#### Linux 编译构建

编译 1.2 版本时，GCC 的最低版本要求为 5.0。编译 3.1 版本时，GCC 的最低版本为 8.1，linux 与 windows 编译要求相同。这里介绍 Linux 编译方法。

**编译 XGBoost 静态库**

需要先编译 XGBoost 静态库。步骤如下：

1. 从 GitHub 上下载 XGBoost 项目：

```
git clone --recursive https://github.com/dmlc/xgboost
```

2. 使用 CMake 编译为静态库：

目前支持 xgboost 的两个版本，一个是 1.2 版本，一个是 3.1 版本，需要切到对应 release tag 下进行编译。

编译 1.2 版本时，GCC 的最低版本要求为 5.0。编译 3.1 版本时，GCC 的最低版本为 8.1。

``` bash
cd xgboost
mkdir build && cd build
cmake .. -DBUILD_STATIC_LIB=ON -DCMAKE_INSTALL_PREFIX=<artifact_path>
cmake --build . -j --verbose
cmake --install .
```

编译完成后将 <artifact_path> 路径下的文件拷贝至 Plugin/thirdParty/ 对应的文件夹下。
3.1 版本 xgboost 如果要编译 linux 下插件，则拷贝至 3.1_linux。
1.2 版本 xgboost 如果要编译 linux 下插件，则拷贝至 1.2_linux, 编译 windows 下插件，则拷贝至 1.2_win。

**编译 XGBoost 插件**
``` bash
cd xgboost
export CMAKE_INSTALL_PREFIX="$(pwd)/output"
bash build.sh Release ./
```
则编译产物会出现在 xgboost/output 文件夹中，目录结构如下：

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
