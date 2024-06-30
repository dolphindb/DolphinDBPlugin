# DolphinDB Zlib Plugin

DolphinDB 的 zlib 插件，支持文件到文件的 zlib 压缩与解压缩。本文档仅介绍编译构建方法。通过 [文档中心 - Zlib](https://docs.dolphindb.cn/zh/plugins/zlib/zlib.html) 查看接口介绍；通过 [CHANGELOG.md](./CHANGELOG.md) 查看版本发布记录。

## 编译构建

可使用以下方法编译 Zlib 插件，编译成功后通过以上方法导入。

### 在 Linux 环境中编译安装

#### cmake 编译

```
mkdir build
cd build
cmake ..
make
```

**注意:** 编译之前请确保 libDolphinDB.so 在 gcc 可搜索的路径中。可使用 LD_LIBRARY_PATH 指定其路径，或者直接将其拷贝到 build 目录下。

编译之后目录下会产生 libPluginZlib.so 文件
