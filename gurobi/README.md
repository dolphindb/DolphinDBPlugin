# DolphinDB Gurobi 插件使用说明

Gurobi 是一款强大的数学优化求解器，主要用于求解线性规划、整数规划、混合整数线性规划、二次规划等一些优化问题。它广泛应用于运筹学、工程、金融和商业决策等领域，以帮助企业和研究人员在给定的约束条件下找到最优解或可行解。

## 在插件市场安装插件

### 版本要求

DolphinDB 2.00.12 及更高版本。

注：目前仅支持 x64 的 Linux 版本。

### 安装步骤

(1) 在 DolphinDB 客户端中使用 listRemotePlugins 命令查看插件仓库中的插件信息。
```DolphinDB
login("admin", "123456")
listRemotePlugins(, "http://plugins.dolphindb.cn/plugins/")
```

(2) 使用 installPlugin 命令完成插件安装。
```DolphinDB
installPlugin("gurobi")
```

(3) 使用 loadPlugin 命令加载插件。
```DolphinDB
loadPlugin("gurobi")
```

## 函数接口

### model

**语法**

`model([params])`

**详情**

建立一个 Gurobi 模型对象，可以通过 *params* 设置参数。

**参数**

- params: 可选参数，字典类型，其中键值的类型均为 STRING。参考 Gurobi 文档的 [Parameters](https://www.gurobi.com/documentation/10.0/refman/parameters.html#sec:Parameters)。

**示例**

```dolphindb
gurobi::model(dict(["NonConvex"], ["2"]))
```

**返回值**

返回创建的 Gurobi 模型对象。

### addVars

**语法**

`addVars(model, lb, ub, obj, type, varName)`

**详情**

为模型添加新的决策变量。

**参数**

- model：通过 `model` 接口创建的 Gurobi 模型对象。
- lb：DOUBLE 类型数组，数组中的每个元素表示对应变量的下界（lower bound）。
- ub：DOUBLE 类型数组，数组中的每个元素表示对应变量的上界（upper bound）。
- obj：DOUBLE 类型数组，数组中的每个元素表示对应变量在目标函数中的系数。参数 obj 可以为 NULL，此时所有变量的系数均为默认值 0.0。
- type：CHAR 类型数组，每个字符表示对应变量的类型。参数 type 可以为 NULL，此时所有变量的类型均为默认值 'C'：
  - 'C': 表示连续变量（Continuous）
  - 'B': 表示二元变量（Binary）
  - 'I': 表示整数变量（Integer）
  - 'S': 表示半连续变量（SEMICONT）
  - 'N': 表示半整数变量（SEMIINT）
- varName：STRING 类型数组，数组中的每个字符串用于命名对应的变量。

**返回值**

返回添加的变量的名数组。

### linExpr

**语法**

`linExpr(model, coefficient, varName)`

**详情**

创建线性表达式对象。

**参数**

- model：通过 `model` 接口创建的 Gurobi 模型对象。
- coefficient：DOUBLE 类型数组，数组中的每个元素表示变量的系数。
- varName：STRING 类型数组，对应 *model* 模型中的变量。

**返回值**

返回创建的线性表达式对象。

### quadExpr

**语法**

`quadExpr(model, quadMatrix, varNames, [linExpr])`

**详情**

创建二次表达式对象。

**参数**

- model：通过 `model` 接口创建的 Gurobi 模型对象。
- quadMatrix：DOUBLE 类型矩阵
  - 通过`累加系数(quadMatrix[r][c]) * 该行表示的变量(varName[r]) * 该列表示的变量(varName[c])`，得到二次表达式。
  - 注意虽然 DolphinDB 的矩阵是列存，理论上应该 transpose 转置，但因为 `quadMatrix[r][c]` 和 `quadMatrix[c][r]` 的作用是完全等价的（因为 `c * obj_r * obj_c = c * obj_c * obj_r`），所以实际使用中无需转置，当成行优先的矩阵直接使用即可。
- varName：STRING 类型数组，对应 model 中的变量。
- linExpr：可选参数，通过 linExpr 接口创建的线性表达式对象，用来初始化二次表达式对象。

**返回值**

返回创建的二次表达式对象。

### addConstr

**语法**

`addConstr(model, lhsExpr, sense, rhsVal)`

**详情**

为模型添加单个线性或二次约束。

**参数**

- model：通过 `model` 接口创建的 Gurobi 模型对象。
- lhsExpr：通过 linExpr 或 quadExpr 接口创建的表达式对象。
- sense：类型为 CHAR，表示约束类型：
  - '<': GRB_LESS_EQUAL
  - '>': GRB_GREATER_EQUAL
  - '=': GRB_EQUAL
- rhsVal：DOUBLE 类型标量。

### setObjective

**语法**

`setObjective(model, expr, sense)`

**详情**

定义模型的优化目标。

**参数**

- model：通过 `model` 接口创建的 Gurobi 模型对象。
- lhsExpr：通过 linExpr 或 quadExpr 接口创建的表达式对象。
- sense：类型为 INT，指定优化方向：
  - -1: GRB_MAXIMIZE
  - 1: GRB_MINIMIZE

### optimize

**语法**

`optimize(model)`

**详情**

根据设置的目标函数和约束来寻找最优解，获取模型求解后的状态。

**参数**

- model：通过 `model` 接口创建的 Gurobi 模型对象。

**返回值**

返回模型求解后的状态，类型为 INT，含义参考 Gurobi 文档的 [Optimization Status Codes](https://www.gurobi.com/documentation/10.0/refman/optimization_status_codes.html#sec:StatusCodes)。

### getResult

**语法**

`getResult(model)`

**详情**

获取优化后参数的取值。

**参数**

- model：通过 `model` 接口创建的 Gurobi 模型对象。

**返回值**

返回优化后的变量求解结果字典，其中包含了模型中所有变量的名称和对应的值。

### getObjective

**语法**

`getObjective(model)`

**详情**

获取优化后的目标值。

**参数**

- model：通过 `model` 接口创建的 Gurobi 模型对象。

**返回值**

获取优化模型求解后的目标函数值，类型为 DOUBLE。

## 使用示例

```bash
/// 初始化模型
model = gurobi::model()

/// 增加变量
lb = 0 0 0 0 0 0 0 0 0 0
ub = 1 1 1 1 1 1 1 1 1 1
stocks = ["A001", "A002", "A003", "A004", "A005", "A006", "A007", "A008", "A009", "A010"]
gurobi::addVars(model, lb, ub, , , stocks)

/// 增加线性约束
A = [1 1 1 0 0 0 0 0 0 0,
     0 0 0 1 1 1 0 0 0 0,
     0 0 0 0 0 0 1 1 1 1,
     -1 -1 -1 0 0 0 0 0 0 0,
     0 0 0 -1 -1 -1 0 0 0 0,
     0 0 0 0 0 0 -1 -1 -1 -1]
rhs = 0.38 0.48 0.38 -0.22 -0.32 -0.22

for (i in 0:6) {
	lhsExpr = gurobi::linExpr(model, A[i], stocks)
	gurobi::addConstr(model, lhsExpr, '<', rhs[i])
}

lhsExpr = gurobi::linExpr(model, 1 1 1 1 1 1 1 1 1 1, stocks)
gurobi::addConstr(model, lhsExpr, '=', 1)

/// 设定优化目标为二次表达式
coefficients = 0.1 0.02 0.01 0.05 0.17 0.01 0.07 0.08 0.09 0.10
linExpr = gurobi::linExpr(model, coefficients, stocks)

H = [-1 0 0 0 0 0 0 0 0 0,
     0 -1 0 0 0 0 0 0 0 0,
     0 0 -1 0 0 0 0 0 0 0,
     0 0 0 -1 0 0 0 0 0 0,
     0 0 0 0 -1 0 0 0 0 0,
     0 0 0 0 0 -1 0 0 0 0,
     0 0 0 0 0 0 -1 0 0 0,
     0 0 0 0 0 0 0 -1 0 0,
     0 0 0 0 0 0 0 0 -1 0,
     0 0 0 0 0 0 0 0 0 -1]
quadExpr = gurobi::quadExpr(model, matrix(H), stocks, linExpr)

gurobi::setObjective(model, quadExpr, -1)

/// 优化
status = gurobi::optimize(model)

/// 获取优化结果
result = gurobi::getResult(model)
```
