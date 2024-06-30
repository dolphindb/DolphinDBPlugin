# Insight Plugin Changelog

## 2.00.12.4 & 3.00.0.1

### 新功能

- 新增支持数据品类：债券快照（BondTick）、基金快照（FundTick）、期权快照（OptionTick）、融券通（SecurityLending）。
- 新增支持市场种类：北交所（XBSE），大商所（XDCE），上期所（XSGE），新三板（NEEQ），郑商所（XZCE），港股通（HKSC），H 股全流通（HGHQ），国证指数（CNI）。
- 新增支持多地址配置。

### 功能改进

- 增加 dataVersion 参数，支持指定华泰 INSIGHT 数据字典的版本号。目前支持填写 '3.2.8'，'3.2.11'，对应华泰 INSIGHT 3.2.8 和 3.2.11 版本。
  兼容性：脚本兼容，字段不兼容，即无法直接将数据落库到以前的数据库中。
- insight::connect 的 ignoreApplSeq 参数名更改为 seqCheckMode 参数，用于控制 OrderTransaction 类型数据出现序列号不连续时的行为。

## 2.00.11.3.1

### 新功能

- 支持用户指定 cert 文件路径。

## 2.00.11

### 新功能

- 新增 `insight::getHandle` 接口，用于获取已有连接句柄。
- `insight::connect` 接口新增参数 *options*，表示扩展参数。
- `insight::connect` 接口新增参数 *ignoreApplSeq*，用于决定当 `OrderTransaction` 数据中出现数据丢失时是否停止接收数据。
- 新增对 OrderTransaction 合并数据类型和基金、债券 投资品类型的支持。
- 新增时延统计功能。（`insight::connect` *options*）
- 支持同时接收 order 和 trade 数据按 ChannelNo 多线程异步写入 DolphinDB 目标表。

### 功能优化

- 优化了数据解析过程，降低了时延。

## 2.00.10

### 故障修复

- 修复了在断网时取消订阅失败的问题。
- 修复了在执行 insight::close 后，再次执行 insight::getStatus 时 server 宕机的问题。
- 修复了当首次连接时输入错误密码，后续连接一直报错的问题。
