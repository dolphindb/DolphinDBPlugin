# Kafka Plugin Changelog

## 2.00.11.3.1

### 新功能

新增接口 kafka::getSubJobConsumer 获取指定订阅中的消费者句柄。

### 故障修复

修复了会话结束后 Kafka 连接未释放的问题。

### 功能优化

改进操作异常时的错误信息提示。

## 2.00.11

### 新增功能

- 新增支持 SASL2 认证与数据加密。

### 故障修复

- 修复 OOM 时插件宕机的问题。

## 2.00.10

### 优化

- 函数 eventGetParts , getOffsetPosition , getOffsetCommitted 增加了返回值。

### bug修复

- 修复了接口 kafka::pollByteStream 不能接收非 JSON 格式数据的问题。
- 修复了多线程操作导致的server 宕机问题。