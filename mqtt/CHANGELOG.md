# Release Note

## 2.00.11

### 新增功能

- 接口 `mqtt::connect` 新增参数 *sendbufSize*，用于指定发送缓冲区的大小。
- 接口 `mqtt::subscribe` 新增参数 *recvbufSize*，用于指定接收缓冲区的大小。

## 2.00.10

### 功能优化

- 优化了部分场景下的报错信息。
- 优化了 connect 函数中关于参数 batchsize 默认值的报错信息。

### 故障修复

- 当 MQTT 服务器关闭时，通过 mqtt::connect 进行连接将收到错误提示。
- 修复了当 mqtt::publish, mqtt::createCsvFormatter 参数的输入值为 NULL 时可能出现的宕机或卡住的问题。
- 修复了若发布消息中包含空值，订阅端无法接收到数据的问题。
- 修复了若发布数据中包含类型为 CHAR  , MONTH 数据时，订阅端会接收到错误类型数据的问题。
