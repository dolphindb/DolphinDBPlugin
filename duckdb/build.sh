#!/bin/bash

# Build script for DuckDB plugin

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../build_util.sh"

# Prepare build directory
prepare_dir "$1" "$2" "$3"

# Build the plugin
build_plugin

# Install the plugin
install_plugin
