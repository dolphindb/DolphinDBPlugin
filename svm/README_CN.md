# DolphinDB SVM Plugin

支持向量机（Support Vector Machine, SVM）是一种监督学习算法，常用于分类和回归问题。DolphinDB 提供了 SVM 插件，使用户可以在 DolphinDB 中对 DolphinDB 对象执行 SVM 模型的训练和预测。插件基于 [libsvm](https://github.com/cjlin1/libsvm) 进行实现，对常见的支持向量机算法进行了封装。

本文档仅介绍编译构建方法。通过 [文档中心-svm](https://docs.dolphindb.cn/zh/plugins/svm/svm.html) 查看接口介绍；通过 [CHANGELOG.md](./CHANGELOG.md) 查看版本发布记录。

## 编译构建

### Linux 编译构建

使用 GCC 4.8.5 进行编译。

``` shell
mkdir build
cd build
cmake ..
make
make install
```

### Windows 编译构建

使用 MinGW-W64 GCC-5.3.0 x86_64_win32-seh-rt 进行编译。

``` cmd
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
mingw32-make install
```
