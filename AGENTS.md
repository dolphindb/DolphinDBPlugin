# Project overview
这个代码仓是 DolphinDB 插件集合

目录结构：
include: DolphinDB 和插件公共头文件
src: 插件公共代码
third_party: 公共第三方库
build_scripts: 开源第三方库编译脚本
dockerfiles: 已废弃
其余文件夹是各个插件

# Rules
- 使用 C++17
- 不要自行引入新的依赖
- 使用 DolphinDB 接口时，将 DolphinDBEverything.h 作为第一个头文件以解决编译告警
- 新插件目录结构参考 UniqueID
  - include: 插件与 DolphinDB 的接口
  - src: 插件实现代码与内部头文件
  - build.sh: 用于 Jenkins CI/CD 的构建脚本
  - Plugin*.txt: 插件接口定义
  - CMakeLists.txt: 构建系统入口，利用公共的 template.cmake 实现统一的构建配置

# Build & Test Environment
- 使用 cmake 构建系统，依赖库在 $HOME/install e.g. cd UniqueID && cmake -B build -DCMAKE_PREFIX_PATH=$HOME/install
