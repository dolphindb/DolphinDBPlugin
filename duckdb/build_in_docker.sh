#!/bin/bash
# 在 Docker 容器中编译 DuckDB 插件

set -e

echo "=================================================="
echo "在 Docker 容器中编译 DuckDB 插件"
echo "=================================================="

# 检查 Docker 是否运行
if ! docker ps &>/dev/null; then
    echo "❌ Docker 未运行，请先启动 Docker"
    exit 1
fi

# 创建一个编译用的容器
echo ""
echo "1. 创建编译容器..."
docker run -d \
    --name duckdb-plugin-builder \
    -v "$(pwd)/..:/DolphinDBPlugin" \
    ubuntu:22.04 \
    tail -f /dev/null

# 安装编译依赖
echo ""
echo "2. 安装编译依赖..."
docker exec duckdb-plugin-builder bash -c "
    apt-get update && \
    apt-get install -y build-essential cmake git && \
    rm -rf /var/lib/apt/lists/*
"

# 初始化子模块
echo ""
echo "3. 初始化 DuckDB 子模块..."
docker exec duckdb-plugin-builder bash -c "
    cd /DolphinDBPlugin/duckdb && \
    if [ -d contrib/duckdb/.git ]; then
        echo '子模块已存在'
    else
        rm -rf contrib/duckdb && \
        git clone --depth 1 --branch v1.1.3 https://github.com/duckdb/duckdb.git contrib/duckdb
    fi
"

# 创建构建目录
echo ""
echo "4. 创建构建目录..."
docker exec duckdb-plugin-builder bash -c "
    cd /DolphinDBPlugin/duckdb && \
    rm -rf build && \
    mkdir -p build
"

# 编译
echo ""
echo "5. 开始编译..."
docker exec duckdb-plugin-builder bash -c "
    cd /DolphinDBPlugin/duckdb/build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j\$(nproc)
"

# 复制编译结果
echo ""
echo "6. 复制编译结果..."
docker exec duckdb-plugin-builder bash -c "
    cp /DolphinDBPlugin/duckdb/build/libPluginDuckDB.so /DolphinDBPlugin/duckdb/
"

# 停止并删除容器
echo ""
echo "7. 清理容器..."
docker stop duckdb-plugin-builder &>/dev/null || true
docker rm duckdb-plugin-builder &>/dev/null || true

echo ""
echo "=================================================="
echo "✅ 编译完成！"
echo "=================================================="
echo ""
echo "编译结果:"
ls -lh "$(pwd)/libPluginDuckDB.so" 2>/dev/null || echo "   libPluginDuckDB.so 未找到"
echo ""
echo "现在可以将插件加载到 DolphinDB 中使用了"
