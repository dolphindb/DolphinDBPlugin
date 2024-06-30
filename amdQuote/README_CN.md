# DolphinDB amdQuote Plugin

华锐高速行情平台（Archforce Market Data）简称 AMD，是华锐提供的超高可用、超低时延的优质行情服务。DolphinDB 提供了 amdQuote 系列插件用于接收华锐 AMD 的极速行情数据，可以将实时行情数据便捷地接入 DolphinDB。amdQuote 系列插件基于多个版本的华锐 AMD SDK 进行开发，通过实现行情的回调接口获取数据。目前支持实时获取逐笔成交委托、股票债券基金以及期货期权快照等多种品类的数据。目前基于的不同的 AMD SDK 版本有 amdQuote396，amdQuote398，amdQuote401，amdQuote420，amdQuote430 五种插件。

本文档仅介绍编译构建方法。通过 [文档中心-amdQuote](https://docs.dolphindb.cn/zh/plugins/amdquote/amdquote_2.html) 查看接口介绍；通过 [CHANGELOG.md](./CHANGELOG.md) 查看版本发布记录。

## 编译构建

### Linux 编译构建

编译支持 AMD SDK 3.9.6，3.9.8，4.0.1，4.2.0，4.3.0 版本的插件，需要在 cmake 时指定 AMD_VERSION 为 对应的 AMD SDK 版本。
以 3.9.6 为例：
``` bash
mkdir build
cd build
cmake .. -DAMDAPIDIR=<amd_ami_dir> -DAMD_VERSION=3.9.6
make -j
make install
```

编译支持 ABI=1 华锐 AMD SDK 3.9.8 版本的插件，需要在 cmake 时指定 AMD_VERSION 为 3.9.8, AMD_USE_ABI=1
``` bash
mkdir build
cd build
cmake .. -DAMDAPIDIR=<amd_ami_dir> -DAMD_VERSION=3.9.8 -DAMD_USE_ABI=1
make -j
make install
```

编译后文件 libPluginAmdQuote.so, PluginAmdQuote.txt 和所依赖的一系列动态库会被放在 bin/linux 目录下。
