#!/bin/bash

CMAKE_DIR=cmake
BUILD_TYPE=Release
BUILD_DIR=build
CodeBlocks="CodeBlocks - MinGW Makefiles"

if [ "$(uname)" == "Darwin" ]; then
  CodeBlocks="CodeBlocks - Unix Makefiles"
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
  CodeBlocks="CodeBlocks - Unix Makefiles"
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
PROJECT_DIR="${SCRIPT_DIR}/.."

DEPS=${PROJECT_DIR}/deps/build/install/usr/local

if [ ! -d "${PROJECT_DIR}/${BUILD_DIR}" ]; then
    mkdir "${PROJECT_DIR}/${BUILD_DIR}"
fi

cd "${PROJECT_DIR}" || exit

# 在构建之前应用CGAL补丁
echo "应用CGAL补丁..."
chmod +x patch-cgal.sh
./patch-cgal.sh

# 检查是否有ARM64专用的构建设置
if [[ $(uname -m) == "arm64" ]]; then
  echo "使用ARM64专用构建设置..."
  # 确保使用正确路径的依赖
  DEPS_INSTALL_PREFIX="$PWD/deps/build/install/usr/local"
  
  # 设置ARM64特定的CMake参数，明确指定要使用的库文件和包含目录
  cmake -S . -B build \
    -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_FIND_ROOT_PATH="$DEPS_INSTALL_PREFIX" \
    -DCMAKE_IGNORE_PATH="/usr/local/lib;/usr/lib" \
    -DCMAKE_FIND_FRAMEWORK=NEVER \
    -DCMAKE_LIBRARY_PATH="$PWD/arm64-libs" \
    -DCMAKE_PREFIX_PATH="$DEPS_INSTALL_PREFIX" \
    -DCGAL_DIR="/usr/local/lib/cmake/CGAL" \
    -DGMP_INCLUDE_DIR="$DEPS_INSTALL_PREFIX/include" \
    -DGMP_LIBRARIES="$PWD/arm64-libs/libgmp.a;$PWD/arm64-libs/libgmpxx.a" \
    -DMPFR_INCLUDE_DIR="$DEPS_INSTALL_PREFIX/include" \
    -DMPFR_LIBRARIES="$PWD/arm64-libs/libmpfr.a" \
    -DCMAKE_EXE_LINKER_FLAGS="-Wl,-search_paths_first -L$PWD/arm64-libs" \
    -DCMAKE_CXX_FLAGS="-I$DEPS_INSTALL_PREFIX/include -L$DEPS_INSTALL_PREFIX/lib"
else
  # 原有的构建命令
  cmake -S . -B build
fi

# 编译
cmake --build build -j $(nproc 2>/dev/null || sysctl -n hw.ncpu)