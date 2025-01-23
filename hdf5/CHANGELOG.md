# HDF5 插件 Changelog

## 2.00.12

### bug修复

- 修复 nullFlag 未设置的问题。

## 2.00.11

### 新功能

- 接口 `hdf5::loadPandasHDF5` 新增对表类型 series_table, frame_table 支持索引方式 multiIndex 。

## 2.00.10

### bug修复

- 修复使用方法 hdf5::ls 执行特定类型的 hdf5 文件后 server 宕机的问题。
- 修复并行导入多个文件时 server 宕机的问题。