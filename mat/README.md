# DolphinDB mat Plugin

DolphinDB 的 mat 插件支持读取 mat 文件的数据到 DolphinDB，或将 DolphinDB 变量写入 mat 文件，且在读取或写入时自动进行类型转换。mat 插件基于 MATLAB Runtime 运行库开发。本文档仅介绍编译构建方法。通过 [文档中心 - Mat](https://docs.dolphindb.cn/zh/plugins/mat/mat.html) 查看接口介绍；通过 [CHANGELOG.md](./CHANGELOG.md) 查看版本发布记录。

## 插件编译

需要 CMake 和对应编译器即可在本地编译 mat 插件。

### Linux

#### 使用 CMake 构建：

安装 CMake：

```
sudo apt-get install cmake
```

构建插件内容：

```
cd <PluginDir>/mat
mkdir build
cd build
cmake -DmatlabRoot=/home/Matlab/v901/../   // 指定 matlab 的安装位置
make
```

**注意**：编译之前请确保 libDolphinDB.so 在 gcc 可搜索的路径中。可使用 `LD_LIBRARY_PATH` 指定其路径，或者直接将其拷贝到 build 目录下。

编译后目录下会产生文件 libPluginMat.so 和 PluginMat.txt。

### Windows

#### 下载 Matlab R2026a 运行库
https://ssd.mathworks.cn/supportfiles/downloads/R2016a/deployment_files/R2016a/installers/win64/MCR_R2016a_win64_installer.exe

#### 使用 CMake 构建：

在编译开始之前，要将 libDolphinDB.dll 拷贝到 build 文件夹，并且拷贝部分 Matlab 的依赖库到 build 目录。

构建插件内容：

```
mkdir build                                                        # 新建 build 目录
cp <ServerDir>/libDolphinDB.dll build                 # 拷贝 libDolphinDB.dll 到 build 目录下
.\copyFile.bat <R2026a_DIR>\R2026\v901\bin\win64 build winLib.txt # R2026a 库路径、目标文件夹 build、所需依赖库列表
cd build
cmake  ../ -G "MinGW Makefiles"
mingw32-make -j4
```

编译后目录下会产生文件 libPluginMat.dll 和 PluginMat.txt