# DolphinDB Demo 插件使用说明

通过 DolphinDB 的 Demo 插件可以执行 minmax 和 echo 两个函数。

## 在插件市场安装插件

### 版本要求

DolphinDB Server Linux 版本。

### 安装步骤

使用 installPlugin 命令完成插件安装。

``` Dolphin Script
installPlugin("demo");
```

使用 loadPlugin 命令加载插件。

``` Dolphin Script
loadPlugin("demo");
```

## 函数接口

### minmax

**语法**

``` Dolphin Script
demo::minmax(X)
```

**参数**

**X** 任意类型的标量或者向量

**详情**

如果传入了一个标量，则会返回一个长度为 2 的向量，值都是这个传入的标量。

如果传入了一个向量，则会返回一个长度为 2 的向量，第一个值为入参向量所有元素中的最小值，第二个值为最大值。

**示例**
``` Dolphin Script
demo::minmax([12,3,4]);
// output: [3,12]
```

### echo

**语法**

``` Dolphin Script
demo::echo(X)
```

**参数**

**X** 任意类型

**详情**

直接返回输入的参数

**示例**

``` Dolphin Script
demo::echo("foo");
//output: foo

demo::echo(1);
// output: 1
```

## 编译构建

### Linux 编译构建

进入插件目录，执行 `make` 命令，即可在插件目录下生成 libPluginDemo.so
