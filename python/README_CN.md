# DolphinDB Py Plugin
利用[python C-API](https://docs.python.org/zh-cn/3.7/c-api/index.html)协议，实现DolphinDB内调用python环境中的第三方库。本插件使用了[pybind11](https://github.com/pybind/pybind11) 库。

Py插件目前支持版本：[relsease200](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/py/README_CN.md), [release130](https://github.com/dolphindb/DolphinDBPlugin/blob/release130/py/README_CN.md)。您当前查看的插件版本为release200，请使用DolphinDB 2.00.X版本server。若使用其它版本server，请切换至相应插件分支。

## 1. 准备工作

### 1.1 依赖库

- libDolphinDB.so（release130分支动态链接openblas.so动态库版本；Windows中为libDolphinDB.dll）

  请参考[插件开发教程](https://github.com/dolphindb/Tutorials_CN/blob/master/plugin_development_tutorial.md)进行配置。

- libOpenblas.so（Windows中为libOpenblas.dll）

  如果无法找到，需要自行安装[OpenBLAS](http://www.openblas.net/)。Linux中可直接用以下命令安装：

  ```
  git clone https://github.com/xianyi/OpenBLAS.git
  make
  make install PREFIX=your_installation_directory
  ```

  默认安装目录为/opt/OpenBLAS，请将该路径添加到环境变量中。

### 1.2 修改配置文件

需要修改配置文件dolphindb.cfg，添加libpython3.7m.so（python37.dll）动态库绝对路径。

```
globalDynamicLib=/path_to_libpython3.7m.so/libpython3.7m.so
```

如果python版本不同请修改成对应版本。 例如：

```
globalDynamicLib=/DolphinDB/server/plugins/py/libpython3.6m.so.1.0
```


**注意**：开发过程中需要有完整的python运行环境，且插件运行时sys.path打印的路径要和安装的运行环境一致。

## 2. 安装

### 2.1 编译安装

#### Linux

##### 使用cmake构建：

安装cmake：
```
sudo apt-get install cmake
```

构建插件内容：
```
mkdir build
cd build
cmake ../
make
```

**注意**：编译之前请确保libDolphinDB.so在gcc可搜索的路径中。可使用 LD_LIBRARY_PATH 指定其路径，或者直接将其拷贝到build目录下。不同python版本可用-DPYTHON:STRING指定，目前只支持python3.6、3.7和3.8版本。

编译后目录下会产生libPluginPy.so文件。

#### Windows

##### 在Windows环境中需要使用CMake和MinGW编译

- 下载安装[MinGW](http://www.mingw.org/)。确保将bin目录添加到系统环境变量Path中。
- 下载安装[cmake](https://cmake.org/)。

##### 使用cmake构建：

在编译开始之前，要将libDolphinDB.dll拷贝到build文件夹。不同python版本可用-DPYTHON:STRING指定，目前只支持python3.6、3.7和3.8版本。

构建插件内容：

```
mkdir build                                                        # 新建build目录
cp path_to_libDolphinDB.dll/libDolphinDB.dll build                 # 拷贝 libDolphinDB.dll 到build目录下
cd build
cmake -DPYTHON:STRING=3.7 ../ -G "MinGW Makefiles"
mingw32-make -j4
```

编译后目录下会产生libPluginPy.dll文件。

### 2.2 加载插件

```
loadPlugin("/path_to_PluginPy/PluginPy.txt");
```

**注意**：由于数据类型转换时需要用到numpy和pandas中的数据类型，python环境中需要安装numpy和pandas模块。Windows环境下请保证编译时的python版本与本地的python版本一致，否则加载插件时会出错。此外如果使用的是Anaconda，因为Anaconda自带的libstdc++.so.6动态库版本较高，而DolphinDB为了保证兼容性使用的是较低版本的libstdc++.so.6，两者冲突会导致加载插件失败，所以需要使用 pip uninstall pandas 卸载原来的pandas，然后使用 pip install pandas 重新安装pandas，不能使用 conda install 进行安装，否则会链接高版本的libstdc++.so.6。同理插件中要使用到的有链接libstdc++.so.6动态库的模块都需要使用 pip uninstall 和 pip intall 重新安装。除了上述重新安装的方法，还可用Anaconda里lib目录下的libstdc++.so.6替换DolphinDB目录下的libstdc++.so.6（为了防止意外，请先将原文件进行备份），这样就不用重新安装模块了。