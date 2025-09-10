# 编译
需要cd到各个插件的目录执行
## 编译默认的Debug版本
./build.sh

## 编译其他版本
./build.sh <cmake build type>
e.g.
./build.sh Release

## 版本说明
Release：发布版本
Debug：调试版本
Test：带asan、覆盖率的版本

# 开发新插件
1. 本代码仓根目录下的build.sh用于定义公共函数。
2. 每个插件需要自己编写插件目录的build.sh
3. 统一使用cmake构建
