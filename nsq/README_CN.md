# DolphinDB NSQ Plugin

为对接恒生 NSQ 极速行情服务软件，DolphinDB 开发了 NSQ 插件。通过该插件能够获取上海和深圳市场的行情。主要获得以下三种行情：

1. 主推-现货深度行情主推回调（OnRtnSecuDepthMarketData->snapshot）
2. 主推-现货逐笔成交行情主推回调（OnRtnSecuTransactionTradeData->trade）
3. 主推-现货逐笔委托行情主推回调（OnRtnSecuTransactionEntrustData->orders）

请注意，DolphinDB 仅提供对接 HSNsqApi 的 NSQ 插件。数据源和接入服务可咨询数据服务商或证券公司。

本文档仅介绍编译构建方法。通过 [文档中心-nsq](https://docs.dolphindb.cn/zh/plugins/nsq/nsq.html) 查看接口介绍。

## 编译构建

### Linux 编译构建

- 使用 CMake 编译构建

首先，在 NSQ 插件文件夹下创建一个 build 文件夹，作为编译工作区。因为编译插件时需要链接 libDolphinDB.so （libDolphinDB.so 是运行 dolphindb 所依赖的库，非插件特有）。编译开始之前，需要将 dolphindb server 同级或上级目录下的 libDolphinDB.so  拷贝至 build 文件夹。

注意：在 [lib](https://github.com/dolphindb/DolphinDBPlugin/tree/release200/nsq/lib) 下，有针对不同操作系统的 NSQ SDK 库，分别存放于不同的文件夹，需要根据当前构建系统的类型拷贝其中一个动态库至 lib 目录（默认是 linux64）。在插件运行的时候也需要加载此 so 库，因此还需要拷贝到能识别该 so 文件的路径下（例如：dolphindb server 下与 libDolphinDB.so 同级的目录）。

```
mkdir build
cd build
cmake ..
make
```

### Windows 编译构建

- 编译准备

    在 Windows 环境中需要使用 CMake 和 MinGW 进行编译，通过以下链接下载：

    - 下载安装 [MinGW](http://www.mingw-w64.org/)。确保将 bin 目录添加到系统环境变量 Path 中。
    - 下载安装 [CMake](https://cmake.org/)。

- 使用 CMake 构建

    首先，在 NSQ 插件文件夹下创建一个 build 文件夹，作为编译工作区。因为编译插件时需要链接 libDolphinDB.so （libDolphinDB.so 是运行 dolphindb 所依赖的库，非插件特有）。编译开始之前，需要将 dolphindb server 同级或上级目录下的 libDolphinDB.dll 拷贝至 build 文件夹。

    构建插件内容：

    ```
    cd <PluginDir>\nsq
    mkdir build                                             # 新建 build 目录
    COPY <ServerDir>/libDolphinDB.dll build                 # 拷贝 libDolphinDB.dll 到 build 目录下
    cd build
    cmake  ../ -G "MinGW Makefiles"
    mingw32-make -j
    ```

### 报错信息

插件正常运行的信息会打印在日志文件中（dolphindb.log），若运行中出现错误，则会抛出异常。具体异常信息及解决办法如下：

1. 重复连接异常。若当前已连接，则需要先通过 close 关闭连接，再 connect 重连。

  You are already connected. To reconnect, please execute close() and try again.

2. API 初始化错误，需要确认 connect 传入的配置文件路径和配置信息是否正确。

  Initialization failed. Please check the config file path and the configuration.

3. API 连接服务器失败，需要确认 connect 传入的配置文件路径和配置信息是否正确。

  Failed to connect to server. Please check the config file path and the configuration.

4. 登录错误，用户名，密码错误。

  login failed: iRet [iRet], error: [errorMsg]

5. API 未初始化错误，需要检查是否 connect() 成功。

  API is not initialized. Please check whether the connection is set up via connect().

6. subscribe 的 `streamTable` 参数错误，需要是一个 shared streamTable（共享流表）。

  The third parameter "streamTable" must be a shared stream table.

7. subscribe 的 `location` 参数错误，需要是 `sh` 或 `sz`。

  The second parameter "location" must be `sh` or `sz`.

8. subscribe 的 `type` 参数错误，应该是 `snapshot` or `trade` or `orders`。

  The first parameter "type" must be  `snapshot`, `trade` or `orders`.

9. subscribe `streamTable` 参数的 schema 错误，schema 需和 SDK 一致。

  Subscription failed. Please check if the schema of “streamTable” is correct.

10. 重复订阅错误，想要更换同一类订阅 (如 `snapshot`, `sh` 两个字段唯一标识一类订阅) 订阅的流表，需要先执行 unsubscribe，然后再更新订阅。

  Subscription already exists. To update subscription, call unsubscribe() and try again.

11. unsubscribe 时 API 未初始化错误。

   API is not initialized. Please check whether the connection is set up via connect().

12. close() 错误，在未初始化（未调用 connect）的 API 上进行了 close。

   Failed to close(). There is no connection to close.