# 开源项目文档规范

## 文档要求

开源项目应提供README用于说明插件的使用方法、接口说明和用例，以及版本说明（Release Notes）用于说明版本更迭细节。有关RELEASE NOTES的写法，参考：[版本说明模板](#版本说明模板)。以下详述README的写作规范。

## 结构

1. 简述：介绍当前版本插件用途。
2. 用法：该插件如何使用（预编译、编译、加载、使用）。
3. 接口说明：包含语法、接口说明简述、参数（值域、数据类型、默认值、约束条件）、返回值
4. 用例：尽可能提供面向每个接口的使用用例。如果接口很多，提供典型的插件用例。

### 简述



描述插件当前版本、用途、适用场景。

### 用法

以分节的方式描述插件的从环境准备到实际使用的使用方法。这包括：环境准备、预编译、编译、加载、使用。

### 接口说明

接口说明能帮助用户了解某一个具体接口的语法、参数设置、返回值、约束条件等信息。以signal插件某接口为例，其模板如下：

#### signal::dwt

对离散信号作一维离散小波变换，返回由变换序列组成的 table。

**语法**

```
signal::dwt(X)
```

**参数**

- X：输入的离散信号序列，一个 int 或 double 类型的 vector。

**返回值**

返回变换序列组成的 table，包含两个字段：cA，cD。cA 对应变换后的近似部分序列，cD 对应变换后的细节部分序列，两个序列等长。

### 用例

通过举例说明的方式展示一个插件或一个插件的接口如何具体使用。以aws插件用例为例：

```
//创建账号
account=dict(string,string);
account['id']='XXXXXXXXXXXXXXX';
account['key']='XXXXXXXXXX';
account['region']='us-east';
//加载S3对象
db = database(directory="dfs://rangedb", partitionType=RANGE, partitionScheme=0 51 101)
aws::loadS3Object(account, 'dolphindb-test-bucket', 't2.zip', 4, db, `pt, `ID);
```

## 描述方式

- 主动语态。陈述句如DolphinDB提供了XXX，实现了XXX；祈使句如添加端口号xxxx至yyyy.cfg文件。
- 人称选择。使用第三人称：他、它、她、他们、她们、它们不要用。以实际名称作为描述主题，例如：使用 `zip`插件解压缩.zip类型文件。
- 避免口语化。“不妨这么想”、“好比”、“比方说”、“我们”等不要用。
- 避免情绪化、感情色彩。
- 避免过分客套。“请”字不要频繁出现。
- 一句话一件事一个对象。一个段落讲述一个意群。当描述的对象与前一个描述对象不同时，分段。
- 言之有物。
  - 内容充实。介绍一个概念时，不仅要陈述，更要简短阐述其机制原理，存在外部资料引用时，提供参考用户易于访问的链接。
  - 解释详实。在使用代码例子或片段对概念、步骤进行诠释时，不要直接把代码丢到标题下就草草了事。需要对该代码的作用，解决什么问题，关键行的作用，返回结果在正文中或用例的代码段中以注释方式进行说明。
- 避免废话。
  - 内部复用。一句话应当尽可能保持唯一，每一句话对应着一个独特的意思。重复的话，车轱辘话不要说。如果在某些操作方法的介绍中存在与前文相似/相同的步骤或说明，那么就在第二次出现时简述当前操作目的，并添加内部参考到前文。
  - 外部复用。如果一个函数、一个原理、一个参数解释、一个算法等在别处有更详尽的解释，那么除非当前教程有必要重述，否则尽可能在简述后使用参考链接到别处。

## 用例选择

选择和设计案例的时候，需要充分地考虑以下几点：

- 与客户实际场景的相似性。或许不需要十分庞大的数据集，但也应当尽可能地贴近现实。
- 代码与案例的关联紧密性。例子代码的提供是为了让用户能以最小的时间开销，通过运行代码的方式，了解某特性是如何解决现实中的问题的。
- 常用性。例子的选择应尽可能选择用户常用的、或容易出现使用偏差的例子。

## 格式

整体上，插件README或RELEASE NOTES可以使用MARKDOWN作为编辑语言，并遵守MARKDOWN语法。发布在GITHUB的文档应遵守GITHUB风格的MARKDOWN语法。参考：https://github.github.com/gfm/

- 标点符号。
  - 顿号。中文写作中，顿号应使用 、
  - 句号。中文句号：。
  - 逗号。中文逗号：，。例外情况：在函数内或代码内，使用英文逗号。
  - 感叹号。除非为函数名或其他代码部分例如表达式包含的，否则不要用。
  - 冒号。使用中文冒号：
  - 括号。在正文描述中使用中文括号；在特定语法格式中，例如Markdown中的链接或图片中的方框与圆括号，使用英文格式。
  - 大于号。在正文中，如果需要表达注意事项，不要使用>或>>。这样会造成在某些输出格式中被转译为为双引号。注意事项就写 **注意** 即可
  - 空格。中文中夹杂英文名称时，可加可不加。无论加或不加，一定要保持上下文一致。
- 函数、变量、路径、参数。
  - 函数的格式：使用一对反引号。例如：`loadModule`
  - 参数的格式：斜体。例如：*parameter*
  - 变量：反引号。例如：`x`
  - 路径：斜体。例如：/DolphinDB/server/
- Markdown文件中步骤编号与步骤内容。在markdown中，有序列表的常用写法有两种：1.1.1.1. 和 1.2.3.4.. 使用1.1.1.1.写法时，务必确保换行后的内容有一个tab键的缩进，否则会造成实际的HTML页面中或PDF中的序号失效。
- 截图。在截图时，确保只截取与上下文或当前步骤存在直接相关的界面内容或代码输出内容。例如，在以下截图中，如果描述的对象只是搜索栏的位置，那么第一个截图是浪费页面空间的，第二个截图足以说明其所在。如果描述对象只是搜索栏的按钮，那么第三张图就足够了。
- MD格式下的标题层级。虽然Gitlab, Github, Gitee能够兼容一个MD文件下存在多个一级标题（# 标题名称），但这样的写法在生成官网使用的HTML页面时无法通过语法校验，导致第一个一级标题以外的一级标题虽然在正文中有相应的格式和内容，但无法写入目录。因此，强烈建议在设置内容层级时，只用一个一级标题。可以理解为一个ROOT。

## 版本说明模板

### 插件名

#### 版本号 (例如，1.30.1)

#### 新功能

- feature 1: xxxxx
- feature 2: xxxxx
- feature 3: xxxxx

#### 功能改进

- Improved xxxx to ensure data integrity...
- Modified xxxx to reduce the xxx
- Prolonged the timeout interval to avoid unexpected incidents such as connection lost... 

#### 缺陷修复

- Fixed the issue that xxx inferface malfunctions when xxx occurs...
- Fixed the issue that ...
