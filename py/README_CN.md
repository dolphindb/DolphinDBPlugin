# DolphinDB Py Plugin

DolphinDB 的 Py 插件利用 [python C-API](https://docs.python.org/zh-cn/3.6/c-api/index.html) 协议，实现 DolphinDB 内调用 python 环境中的第三方库。本插件使用了 [pybind11](https://github.com/pybind/pybind11)  库。本文档仅介绍编译构建方法。通过 [文档中心 - py](https://docs.dolphindb.cn/zh/plugins/py/py.html) 查看接口介绍。


## 编译安装

可使用以下方法编译 Py 插件，编译成功后通过以上方法导入。

### 在 Linux 环境中编译安装

#### 使用 cmake 构建

安装 cmake：
```
sudo apt-get install cmake
```

构建插件内容：
```
mkdir build
cd build
cmake -DPYTHON:STRING=3.6 ../
make
```

**注意**：编译之前请确保 libDolphinDB.so 在 gcc 可搜索的路径中。可使用 LD_LIBRARY_PATH 指定其路径，或者直接将其拷贝到 build 目录下。不同 python 版本可用 - DPYTHON:STRING 指定，目前只支持 python3.6 版本。

编译后目录下会产生 libPluginPy.so 文件。
**注意**：开发过程中需要有完整的 python 运行环境，且插件运行时 sys.path 打印的路径要和安装的运行环境一致。
