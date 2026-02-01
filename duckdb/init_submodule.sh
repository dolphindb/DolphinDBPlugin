#!/bin/bash

# Initialize DuckDB submodule

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "Initializing DuckDB submodule..."

# Clone DuckDB repository if not already present
if [ ! -d "contrib/duckdb" ]; then
    echo "Cloning DuckDB repository..."
    mkdir -p contrib
    cd contrib
    git clone --depth 1 --branch v1.1.3 https://github.com/duckdb/duckdb.git
    cd ..
else
    echo "DuckDB submodule already exists."
fi

echo "DuckDB submodule initialized successfully."
echo "Please run build.sh to build the plugin."
