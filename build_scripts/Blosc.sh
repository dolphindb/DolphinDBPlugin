#!/bin/bash

sed -i 's/cmake_minimum_required(VERSION 2.8.12)/cmake_minimum_required(VERSION 3.10)/g' CMakeLists.txt
cmake_all -DBUILD_SHARED=OFF -DBUILD_TESTS=OFF -DBUILD_FUZZERS=OFF -DBUILD_BENCHMARKS=OFF
