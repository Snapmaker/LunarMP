#!/bin/bash

# 如果检测到ARM64架构，则强制创建正确的库链接
if [[ $(uname -m) == "arm64" ]]; then
  echo "创建ARM64强制链接..."
  
  # 获取正确路径
  DEPS_INSTALL_PREFIX="$PWD/deps/build/install/usr/local"
  ARM64_LIB_DIR="$DEPS_INSTALL_PREFIX/lib"
  
  # 创建临时链接目录
  mkdir -p arm64-libs
  
  # 为GMP和MPFR创建硬链接
  ln -f "$ARM64_LIB_DIR/libgmp.a" arm64-libs/libgmp.a
  ln -f "$ARM64_LIB_DIR/libgmpxx.a" arm64-libs/libgmpxx.a
  ln -f "$ARM64_LIB_DIR/libmpfr.a" arm64-libs/libmpfr.a
  
  # 设置环境变量，确保链接器首先查找这个目录
  export LDFLAGS="-L$PWD/arm64-libs -Wl,-search_paths_first $LDFLAGS"
  export LIBRARY_PATH="$PWD/arm64-libs:$LIBRARY_PATH"
  
  echo "强制链接环境设置完成"
fi 