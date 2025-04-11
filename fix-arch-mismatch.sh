#!/bin/bash

# 在ARM64 Mac上全面修复架构问题

if [[ $(uname -m) == "arm64" ]]; then
  echo "===== 检测到 ARM64 架构，正在应用全面架构兼容性修复 ====="
  
  # 更强力的架构强制设置
  export CFLAGS="-arch arm64 -mtune=native"
  export CXXFLAGS="-arch arm64 -mtune=native"
  export LDFLAGS="-arch arm64"
  export ABI="arm64-darwin"  # 关键：为GMP设置正确的ABI
  
  # 针对GMP/MPFR的特殊配置
  export GMP_HOST="arm64-apple-darwin"
  export MPFR_HOST="arm64-apple-darwin"
  
  # 保存环境变量
  echo "CFLAGS=$CFLAGS" >> $GITHUB_ENV
  echo "CXXFLAGS=$CXXFLAGS" >> $GITHUB_ENV
  echo "LDFLAGS=$LDFLAGS" >> $GITHUB_ENV
  echo "ABI=$ABI" >> $GITHUB_ENV
  echo "GMP_HOST=$GMP_HOST" >> $GITHUB_ENV
  echo "MPFR_HOST=$MPFR_HOST" >> $GITHUB_ENV
  
  # 创建临时目录用于自定义编译
  CUSTOM_BUILD_DIR="$PWD/arm64_custom_build"
  mkdir -p "$CUSTOM_BUILD_DIR"
  
  # 修改依赖构建脚本
  if [ -f "sh/cmake-deps.sh" ]; then
    echo "深度修改cmake-deps.sh脚本以确保ARM64兼容性..."
    
    # 备份原始脚本
    cp sh/cmake-deps.sh sh/cmake-deps.sh.orig
    
    # 修改GMP配置
    sed -i.bak1 's/\.\/configure/\.\/configure --build=arm64-apple-darwin --host=arm64-apple-darwin --enable-shared=no --enable-static=yes ABI=arm64-darwin/g' sh/cmake-deps.sh
    
    # 修改MPFR配置
    sed -i.bak2 's/\.\/configure --with-gmp.*/\.\/configure --with-gmp=\$\{DEPS_INSTALL_PREFIX\} --build=arm64-apple-darwin --host=arm64-apple-darwin --enable-shared=no --enable-static=yes/g' sh/cmake-deps.sh
    
    # 确保使用正确的链接器标志
    echo 'export LDFLAGS="-arch arm64"' >> sh/cmake-deps.sh
  fi
  
  # 清理现有构建
  if [ -d "./deps" ]; then
    echo "清理现有构建目录以使用正确的架构重新编译..."
    rm -rf ./deps ./build
  fi
  
  # 手动下载和预处理GMP（如果需要）
  mkdir -p deps/download
  if [ ! -f "deps/download/gmp-6.2.1.tar.bz2" ]; then
    echo "下载GMP源码..."
    curl -L -o deps/download/gmp-6.2.1.tar.bz2 https://ftp.gnu.org/gnu/gmp/gmp-6.2.1.tar.bz2
  fi
  
  echo "ARM64架构深度兼容性修复完成！"
else
  echo "非ARM64架构，不需要特殊处理。"
fi 