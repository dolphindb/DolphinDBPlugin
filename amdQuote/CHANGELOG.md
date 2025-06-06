
# AmdQuote Plugin Changelog

## 2.00.12.4 & 3.00.0.1

### 新功能

- 连接时支持指定 dataVersion，增加对 dataVersion 为 4.0.1 的新增数据字段的支持。
- subscribe 新增 seqCheckMode 参数，可以在订阅 orderExecution 类型时指定连续性检测的模式。

## 2.00.12 & 3.00.0

### 新功能

- amdQuote398 插件支持 Linux X86-64 ABI=1 DolphinDB server。

## 2.00.11.3.1

### 功能改进：

- 获取代码表时支持指定市场参数。

## 2.00.11.1

### 新功能

- 支持接收期货数据。
- 在 log 中增加输出华锐 SDK 的 log。

## 2.00.11

### 新功能

- 支持接收 ETF 期权与 IOPV 数据（支持华锐 SDK 3.9.8 及以后版本）
- 新增 getHandle 接口，获取已连接的句柄。
- 接口 getStatus 新增返回内容 processedMsgCount，显示已经处理的消息数。
- 新增支持华锐 SDK 4.3.0 版本。

### 故障修复

- 规避了低内存情况下，连接华锐行情源可能导致 crash 的问题。
- 修复了接入异步持久化流表会导致 crash 的问题。

## 2.00.10

### 新功能

- 新增支持接收委托表（order）和成交表（trade ）按交易所原始频道代码（ChannelNo）多线程异步写入 server 中的目标表。同一个原始频道代码的委托表和成交表将写入同一张表，且保证其写入顺序。
- 接口 amdQuote::connect 新增支持参数 outputElapsed，用于统计插件内部的时延。
- 新增支持订阅输出至非流数据表。

### 故障修复

- 修复在订阅一个错误市场后，无法使用 amdQuote::unsubscribe 取消该订阅的问题。
- 修复了在长时间订阅后，行情数据偶现无法正确写入流表的问题。
