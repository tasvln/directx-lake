#!/bin/bash

mkdir -p build
cd build || exit

# This is the KEY line — set Debug build type here
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_TOOLCHAIN_FILE=C:/Users/tasvl/vcpkg/scripts/buildsystems/vcpkg.cmake ..

# Just build normally (no --config for Ninja or Makefiles)
cmake --build .
