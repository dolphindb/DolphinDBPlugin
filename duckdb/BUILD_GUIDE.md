# DuckDB 插件编译指南

## 概述

由于本地 macOS 编译的插件无法直接在 Linux Docker 容器中运行，我们需要在 Linux 环境下编译插件。

## 方法 1: 使用 Docker 编译（推荐）

### 步骤 1: 准备编译环境

```bash
# 创建编译目录
mkdir -p /tmp/duckdb_build
cd /tmp/duckdb_build

# 下载 DolphinDB Linux 版本
curl -L -o dolphindb_linux.zip https://cdn.dolphindb.cn/downloads/DolphinDB_Linux64_V2.00.10.zip
unzip -q dolphindb_linux.zip

# 复制插件源代码
cp -r /Users/qiucheng/work/jindata/DolphinDBPlugin/duckdb ./duckdb_plugin
```

### 步骤 2: 启动编译容器

```bash
# 启动 Ubuntu 容器并挂载源代码
docker run --rm -it \
  -v /tmp/duckdb_build/duckdb_plugin:/duckdb_plugin \
  -v /tmp/duckdb_build/server:/dolphindb \
  -e LD_LIBRARY_PATH=/dolphindb \
  ubuntu:22.04 \
  bash
```

### 步骤 3: 在容器中编译

```bash
# 1. 安装依赖
apt-get update && apt-get install -y git cmake build-essential g++ wget unzip

# 2. 下载 DuckDB 源码
cd /duckdb_plugin
mkdir -p contrib
cd contrib
wget -q https://github.com/duckdb/duckdb/archive/refs/tags/v1.1.3.tar.gz
tar -xzf v1.1.3.tar.gz
mv duckdb-1.1.3 duckdb
rm v1.1.3.tar.gz

# 3. 编译插件
cd /duckdb_plugin/build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

# 4. 验证编译结果
ls -la /duckdb_plugin/build/libPluginDuckDB.so
```

### 步骤 4: 复制编译好的插件

```bash
# 在宿主机上执行
cp /tmp/duckdb_build/duckdb_plugin/build/libPluginDuckDB.so ./
cp /tmp/duckdb_build/duckdb_plugin/PluginDuckDB.txt ./
```

## 方法 2: 使用自动化脚本

我们已经创建了自动化脚本，可以直接运行：

```bash
cd /Users/qiucheng/work/jindata/DolphinDBPlugin/duckdb
bash build_in_docker.sh
```

## 方法 3: 创建包含插件的 Docker 镜像

### 步骤 1: 编译插件

按照方法 1 或方法 2 编译插件。

### 步骤 2: 创建 Docker 镜像

```bash
# 创建 Dockerfile
cat > Dockerfile.dolphindb << 'EOF'
FROM ubuntu:22.04

# 安装运行时依赖
RUN apt-get update && apt-get install -y \
    libstdc++6 \
    curl \
    && rm -rf /var/lib/apt/lists/*

# 复制 DolphinDB
COPY server /opt/dolphindb/

# 复制插件
COPY libPluginDuckDB.so /opt/dolphindb/plugins/duckdb/
COPY PluginDuckDB.txt /opt/dolphindb/plugins/duckdb/

# 创建启动脚本
RUN echo '#!/bin/bash\n\
cd /opt/dolphindb\n\
export LD_LIBRARY_PATH=/opt/dolphindb:$LD_LIBRARY_PATH\n\
mkdir -p /opt/dolphindb/data\n\
./dolphindb -console 0 -mode single -home /opt/dolphindb/data -script dolphindb.dos -config dolphindb.cfg -logFile /opt/dolphindb/data/dolphindb.log\n\
' > /opt/dolphindb/start.sh && chmod +x /opt/dolphindb/start.sh

EXPOSE 8848

CMD ["/opt/dolphindb/start.sh"]
EOF

# 构建镜像
docker build -t dolphindb-duckdb:v2.00.10 -f Dockerfile.dolphindb .

# 运行容器
docker run -d --name dolphindb-duckdb -p 8848:8848 dolphindb-duckdb:v2.00.10
```

## 使用插件

### 1. 启动 DolphinDB 服务器

```bash
docker run -d --name dolphindb-duckdb -p 8848:8848 dolphindb-duckdb:v2.00.10
```

### 2. 连接到服务器

```python
import dolphindb as ddb

session = ddb.Session()
session.connect("localhost", 8848, "admin", "123456")
```

### 3. 加载插件

```python
# 加载 DuckDB 插件
session.run('loadPlugin("/opt/dolphindb/plugins/duckdb/PluginDuckDB.txt")')

# 测试插件
session.run('conn = duckdb::connect(":memory:")')
```

### 4. 执行 DOS 脚本导入数据

```python
# 读取 DOS 文件
with open('/Users/qiucheng/work/jindata/jindata/recorder/dos/update_from_duckdb.dos', 'r') as f:
    dos_script = f.read()

# 执行 DOS 脚本
session.run(dos_script)

# 调用导入函数
session.run('update_from_duckdb(2025.03.04, "/path/to/data.duckdb")')
```

## 性能测试

### CSV → DuckDB

```bash
cd /Users/qiucheng/work/jindata
python tests/test_csv_to_duckdb.py
```

预期性能：
- 128,712 行数据
- 导入时间：~0.08 秒
- 导入速度：~1,600,000 行/秒

### DuckDB → DolphinDB（使用插件）

```bash
python tests/test_perf_duckdb_plugin.py
```

预期性能：
- 导入时间：~0.5-1 秒
- 导入速度：~100,000-200,000 行/秒

## 常见问题

### 1. 编译错误：找不到 libDolphinDB.so

确保设置了正确的 `LD_LIBRARY_PATH`：
```bash
export LD_LIBRARY_PATH=/dolphindb:$LD_LIBRARY_PATH
```

### 2. 插件加载失败

检查插件文件是否存在：
```bash
docker exec dolphindb-duckdb ls -la /opt/dolphindb/plugins/duckdb/
```

### 3. 数据类型不匹配

确保 DolphinDB 表结构与 DuckDB 表结构兼容。

## 参考文档

- [DolphinDB 插件开发指南](https://docs.dolphindb.cn/zh/plugins/plg_dev_tutorial.html)
- [DolphinDB 插件使用文档](https://docs.dolphindb.cn/en/Plugins/ddb_plugin.html)
- [DuckDB 文档](https://duckdb.org/docs/)
