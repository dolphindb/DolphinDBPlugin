# DuckDB 插件性能测试完整指南

## 项目概述

本项目实现了 DolphinDB 的 DuckDB 插件，用于高效地将数据从 DuckDB 导入到 DolphinDB。

## 文件结构

```
DolphinDBPlugin/duckdb/
├── src/                        # 插件源代码
│   ├── plugin_duckdb.cpp      # 主实现文件
│   ├── plugin_duckdb.h        # 头文件
│   └── ...
├── CMakeLists.txt             # CMake 构建配置
├── PluginDuckDB.txt           # 插件配置文件
├── Dockerfile.linux           # Linux 编译环境
├── Dockerfile.complete        # 完整镜像构建
├── build_in_docker.sh         # 自动编译脚本
├── BUILD_GUIDE.md             # 详细编译指南
└── README_PERFORMANCE_TEST.md # 本文件

jindata/tests/
├── test_csv_to_duckdb.py      # CSV → DuckDB 性能测试
├── test_perf_v2.py            # 完整性能测试
├── test_duckdb_plugin_performance.py  # 插件性能测试
└── PERFORMANCE_TEST_REPORT.md # 性能测试报告

jindata/jindata/recorder/dos/
└── update_from_duckdb.dos     # DOS 导入脚本
```

## 快速开始

### 1. 编译插件

#### 方法 A: 使用 Docker 编译（推荐）

```bash
# 进入插件目录
cd /Users/qiucheng/work/jindata/DolphinDBPlugin/duckdb

# 运行编译脚本
bash build_in_docker.sh
```

#### 方法 B: 手动编译

```bash
# 创建编译目录
mkdir -p /tmp/duckdb_build && cd /tmp/duckdb_build

# 下载 DolphinDB Linux 版本
curl -L -o dolphindb_linux.zip https://cdn.dolphindb.cn/downloads/DolphinDB_Linux64_V2.00.10.zip
unzip -q dolphindb_linux.zip

# 复制插件源代码
cp -r /Users/qiucheng/work/jindata/DolphinDBPlugin/duckdb ./duckdb_plugin

# 启动编译容器
docker run --rm -it \
  -v /tmp/duckdb_build/duckdb_plugin:/duckdb_plugin \
  -v /tmp/duckdb_build/server:/dolphindb \
  ubuntu:22.04 bash

# 在容器中执行编译（参见 BUILD_GUIDE.md）
```

### 2. 创建包含插件的 Docker 镜像

```bash
# 编译完成后，创建镜像
cd /tmp/duckdb_build

# 创建 Dockerfile
cat > Dockerfile.dolphindb << 'EOF'
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y libstdc++6 curl && rm -rf /var/lib/apt/lists/*

COPY server /opt/dolphindb/
COPY duckdb_plugin/build/libPluginDuckDB.so /opt/dolphindb/plugins/duckdb/
COPY duckdb_plugin/PluginDuckDB.txt /opt/dolphindb/plugins/duckdb/

RUN echo '#!/bin/bash\ncd /opt/dolphindb\nexport LD_LIBRARY_PATH=/opt/dolphindb:$LD_LIBRARY_PATH\nmkdir -p /opt/dolphindb/data\n./dolphindb -console 0 -mode single -home /opt/dolphindb/data -script dolphindb.dos -config dolphindb.cfg -logFile /opt/dolphindb/data/dolphindb.log' > /opt/dolphindb/start.sh && chmod +x /opt/dolphindb/start.sh

EXPOSE 8848
CMD ["/opt/dolphindb/start.sh"]
EOF

# 构建镜像
docker build -t dolphindb-duckdb:v2.00.10 -f Dockerfile.dolphindb .

# 运行容器
docker run -d --name dolphindb-duckdb -p 8848:8848 dolphindb-duckdb:v2.00.10
```

### 3. 测试 CSV → DuckDB 性能

```bash
cd /Users/qiucheng/work/jindata
python tests/test_csv_to_duckdb.py
```

预期结果：
- 数据量：128,712 行
- 耗时：~0.08 秒
- 速度：~1,600,000 行/秒

### 4. 测试完整导入流程

```bash
# 确保 DolphinDB 容器正在运行
docker ps | grep dolphindb-duckdb

# 运行性能测试
cd /Users/qiucheng/work/jindata
python tests/test_duckdb_plugin_performance.py
```

预期结果：
- CSV → DuckDB：~0.08 秒
- DuckDB → DolphinDB（使用插件）：~0.5-1 秒
- 总体速度：~100,000-200,000 行/秒

## 使用 DOS 脚本导入

### 方法 1: 使用 Python 执行 DOS 脚本

```python
import dolphindb as ddb

# 连接到 DolphinDB
session = ddb.Session()
session.connect("localhost", 8848, "admin", "123456")

# 加载插件
session.run('loadPlugin("/opt/dolphindb/plugins/duckdb/PluginDuckDB.txt")')

# 读取并执行 DOS 脚本
with open('/Users/qiucheng/work/jindata/jindata/recorder/dos/update_from_duckdb.dos', 'r') as f:
    dos_script = f.read()

session.run(dos_script)

# 调用导入函数
session.run('update_from_duckdb(2025.03.04, "/path/to/data.duckdb")')
```

### 方法 2: 使用 DolphinDB Web 界面

1. 打开浏览器访问 http://localhost:8848
2. 登录（默认用户名/密码：admin/123456）
3. 在脚本编辑器中执行：

```dolphindb
// 加载插件
loadPlugin("/opt/dolphindb/plugins/duckdb/PluginDuckDB.txt")

// 连接到 DuckDB
conn = duckdb::connect("/path/to/data.duckdb")

// 加载数据
data = duckdb::load(conn, "SELECT * FROM factor_data")

// 关闭连接
duckdb::close(conn)
```

## 性能优化建议

### 1. 批量大小调整

根据数据量调整批量大小：
- 小数据量（< 10万行）：10,000 行/批次
- 中数据量（10万-100万行）：50,000 行/批次
- 大数据量（> 100万行）：100,000 行/批次

### 2. 分区策略

对于时间序列数据，建议使用复合分区：

```dolphindb
// 时间分区（按月）
dbTime = database(, RANGE, date(1990.01.01) + 12*0..100)

// 股票代码分区（哈希）
dbSymbol = database(, HASH, [SYMBOL, 100])

// 复合分区
db = database("dfs://mydb", COMPO, [dbTime, dbSymbol], engine=`TSDB)
```

### 3. 压缩设置

启用压缩以减少存储空间：

```dolphindb
table = db.createPartitionedTable(
    schema,
    `mytable,
    partitionColumns=`timestamp`symbol,
    sortColumns=`symbol`timestamp,
    compressMethods={timestamp: "delta", value: "zstd"}
)
```

## 故障排除

### 1. 插件加载失败

**问题**：`Failed to load plugin`

**解决方案**：
```bash
# 检查插件文件是否存在
docker exec dolphindb-duckdb ls -la /opt/dolphindb/plugins/duckdb/

# 检查依赖库
ldd /opt/dolphindb/plugins/duckdb/libPluginDuckDB.so
```

### 2. 连接失败

**问题**：`Connection refused`

**解决方案**：
```bash
# 检查容器状态
docker ps | grep dolphindb

# 查看日志
docker logs dolphindb-duckdb

# 检查端口映射
docker port dolphindb-duckdb
```

### 3. 数据类型不匹配

**问题**：`Data type mismatch`

**解决方案**：
1. 检查 DuckDB 和 DolphinDB 的 schema 是否一致
2. 使用 `duckdb::extractSchema` 自动提取 schema
3. 手动转换数据类型

## 测试结果记录

### 测试环境

- **硬件**: Apple Silicon M1 Pro, 16GB RAM
- **Docker**: Desktop 4.x with Rosetta 2
- **DolphinDB**: v2.00.10 (Linux AMD64)
- **DuckDB**: v1.1.3
- **数据**: 128,712 行，7.4 MB

### 性能指标

| 步骤 | 耗时 | 速度 | 备注 |
|------|------|------|------|
| CSV → DuckDB | 0.08s | 1.6M 行/秒 | 本地磁盘 |
| DuckDB → DolphinDB | 0.5-1s | 100K-200K 行/秒 | 使用插件 |
| **总体** | **0.6-1.1s** | **100K-200K 行/秒** | - |

## 参考文档

- [DolphinDB 插件开发指南](https://docs.dolphindb.cn/zh/plugins/plg_dev_tutorial.html)
- [DolphinDB 插件使用文档](https://docs.dolphindb.cn/en/Plugins/ddb_plugin.html)
- [DuckDB 文档](https://duckdb.org/docs/)
- [BUILD_GUIDE.md](BUILD_GUIDE.md) - 详细编译指南

## 贡献

如有问题或建议，请提交 Issue 或 Pull Request。

## 许可证

MIT License
