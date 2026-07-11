# MySQL 插件

本插件用于连接 MySQL 或其他支持 MySQL 协议的数据库，主要用途为导入数据。

## SDK 选型

### MariaDB Connector/C vs MySQL Connector/C

选择 MariaDB 连接器。

- MySQL 的是 GPLv2 许可，商业不友好。
- MariaDB 官方支持 MySQL 没有兼容性问题。
- 目前没有使用 MariaDB 专有功能。

### Connector/C vs Connector/C++

选择 Connector/C。

- C++ 依赖 C
- 不需要 JDBC API
