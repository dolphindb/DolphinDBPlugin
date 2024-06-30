# DolphinDB INSIGHT Plugin

INSIGHT 是华泰证券提供的极速金融数据服务，为对接华泰 INSIGHT 行情系统，DolphinDB 开发了 INSIGHT 插件，获取实时的行情数据。Insight 插件基于华泰 Insight SDK TCP 版本开发，通过实现行情的回调接口获取数据，目前支持接入多家证券市场与期货市场的实时行情，数据品类包括逐笔、股票、基金、期权、期货快照、融券通等。

本文档仅介绍编译构建方法。通过 [文档中心-insight](https://docs.dolphindb.cn/zh/plugins/insight/insight_2.html) 查看接口介绍；通过 [CHANGELOG.md](CHANGELOG.md) 查看版本发布记录。

## 编译构建

### Linux 编译构建

``` bash
cd <path_to_insight>/insight
mkdir build
cd build
cmake ..
make -j
```