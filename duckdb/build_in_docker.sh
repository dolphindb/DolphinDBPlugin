#!/bin/bash
# 在 Docker 中编译 DuckDB 插件的脚本

set -e

echo "=== 开始编译 DuckDB 插件 ==="

# 创建工作目录
WORK_DIR="/tmp/duckdb_build_$(date +%s)"
mkdir -p "$WORK_DIR"
cd "$WORK_DIR"

echo "1. 下载 DolphinDB Linux 版本..."
curl -L -o dolphindb_linux.zip https://cdn.dolphindb.cn/downloads/DolphinDB_Linux64_V2.00.10.zip
unzip -q dolphindb_linux.zip

echo "2. 复制插件源代码..."
cp -r /Users/qiucheng/work/jindata/DolphinDBPlugin/duckdb ./duckdb_plugin

echo "3. 下载 DuckDB 源码..."
cd duckdb_plugin
mkdir -p contrib
cd contrib
wget -q https://github.com/duckdb/duckdb/archive/refs/tags/v1.1.3.tar.gz
tar -xzf v1.1.3.tar.gz
mv duckdb-1.1.3 duckdb
rm v1.1.3.tar.gz

echo "4. 编译插件..."
cd /build/duckdb_plugin/build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

echo "5. 验证编译结果..."
ls -la /build/duckdb_plugin/build/libPluginDuckDB.so

echo "6. 复制编译好的插件..."
mkdir -p /output
cp /build/duckdb_plugin/build/libPluginDuckDB.so /output/
cp /build/duckdb_plugin/PluginDuckDB.txt /output/

echo "=== 编译完成 ==="
echo "插件文件:"
ls -la /output/
