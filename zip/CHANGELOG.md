# Release Notes

## 2.00.11

### 新增功能

- 新增接口 `zip::zip`，支持压缩文件和文件夹。
- 接口 `zip::unzip` 新增参数 *password*，支持解压时的解密功能。

## 2.00.10

### 新增功能

- 新增对 Windows 版本的支持。
- 增加 zipEncode 参数，提供指定 zip 文件中文件名编码格式的功能。

### 功能优化

- 优化部分报错信息及终端输出。

### 故障修复

- 修复使用 `zip::unzip` 时若抛出异常时，已有 handle 未及时关闭的问题。
- 修复了路径为中文时的乱码问题。
