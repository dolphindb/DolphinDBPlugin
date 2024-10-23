# 如何 contribute DolphinDBPlugin 项目

DolphinDBPlugin 是 DolphinDB 的插件项目，为 DolphinDB 提供了许多拓展的系统功能，DolphinDB 欢迎任何社区贡献者参与这个项目。

本文档概述了有关贡献代码须知，开发工作流等事项。贡献代码时请遵循这些规则，以便 DolphinDB 更加方便地接受您的代码贡献。

## 环境配置

Linux版 server 的插件需要使用 `GCC-4.8.5` 编译器进行编译，Windows 版本 server 的插件需要使用 `MinGW-W64 GCC-5.3.0 x86_64-win32-seh` 编译器进行编译。每个插件的编译流程可以见各个插件文件夹下的 README。

具体编写案例见 [readme linux](README_CN.md)，[readme windows](README_WIN_CN.md)。

## 贡献代码须知

`修改内容限制`

`include/` 文件夹下的头文件（不包括 `include/ddbplugin`）为对应 DolphinDB server 的部分头文件。因此，为避免因为头文件不匹配而带来的宕机等问题，这部分代码社区贡献者不可改动。

`插件组织`

为了便于组织管理，DolphinDBPlugin 项目中同一个插件只放在一个文件夹中。由于 DolphinDBPlugin 项目中内容较多，请尽量一次 git 提交 只涉及一个插件，并使用 git fetch git rebase / get pull --rebase 等方式，避免无谓的 merge。

`代码规范`

1. 使用驼峰命名法

2. 类的首字母大写，变量和函数的首字母小写

3. 成员变量最后加下划线

4. 函数的返回值必须写在最前面

其他没有硬性要求，做到代码整洁，易读性高即可。

`文档规范`

开发新插件或者增加新功能后，请提供相应的文档，包括：

1. 简述：简要概述插件的内容

2. 编译方法：如何编译这个插件

3. 接口说明：

    3.1 语法：函数在 DolphinDB 脚本中的语法，使用 [param] 区分是否是可选参数，例如：hdf5::loadHDF5(fileName,datasetName,[schema],[startRow=0],[rowNum=0])

    3.2 参数：说明参数的类型、含义、是否可选以及是否有默认值

    3.3 返回值：说明返回值的类型

    3.4 函数详情：说明该函数的具体功能和用法

    3.5 使用案例：给出调用该函数的一个 DolphinDB 脚本案例

整体文档编写要求简洁、符合 markdown 文档的规范, 详情请见 [开源项目文档规范](./开源项目文档规范.md)。

`开发分支`

DolphinDBPlugin 项目的开发分支为 release200 和 release130，请将代码提交到这两个分支。

DolphinDB 公司不使用 GitHub 作为内部的代码仓库，最新的开发内容会定期同步到 GitHub 上。因此外部贡献者看到的可能非最新代码。DolphinDB 会将被接受的 pull request 代码合入内、外的代码仓库中。

## 开发工作流

要为 DolphinDBPlugin 代码库做出贡献，请遵循本节中定义的工作流。

1. 编写代码，修改 bug 或者增加新 feature。

2. 运行该插件文件夹下的回归测试并确保通过。

    在每个插件中有 test 文件夹，里面是该插件的精简版测试用例，在各自插件的 README 中会介绍测试文件和测试方法。

    DolphinDBPlugin 要求贡献者在 提交 代码时能够跑通 test 目录下的精简过的测试用例，然后再进行下一步的 code review

3. 将代码改动以 pull request 形式推送到存储库分支中的开发分支。

    注意 pull request 的提交信息格式：`插件名: 进行的改动`。

    以 odbc 为例：`odbc: add bulk insert support for Clickhouse.`


感谢您的贡献！

## 进行 code review

由于测试脚本和文件数量较多，在项目 test 文件夹下的只是精简版测试用例。在 code review 前 DolphinDB 会先进行完整回归测试，如有问题会给社区贡献者反馈具体的问题场景。

提交的 pull request 在通过完整的回归测试后，会至少被分配给两位审阅者进行审阅。审阅者将针对正确性、错误、改进机会、文档和代码风格进行将进行彻底的代码审查。

如果需要进一步的沟通请通过邮件联系 support@dolphindb.com。


