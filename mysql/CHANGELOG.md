# Release Notes

## 2.00.11

### 新增功能

- 新增支持 Decimal 数据类型。
- 新增接口函数 `mysql::close`，用于断开连接、关闭 MySQL 句柄。

### 功能优化

- 优化接口 `mysql::load`、`mysql::loadEx` 可传入字符串数据的上限。

## 2.00.10

### 故障修复

- 增加对传入连接有效性的校检。
