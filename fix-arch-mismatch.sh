#!/bin/bash

# 架构检测和对应处理
if [[ $(uname -m) == "arm64" ]]; then
  echo "===== 检测到 ARM64 架构，使用 ARM64 专用编译路径 ====="
  
  # 纯粹的ARM64环境变量
  export CFLAGS="-arch arm64"
  export CXXFLAGS="-arch arm64"
  export LDFLAGS="-arch arm64"
  
  # 导出到环境中
  echo "CFLAGS=$CFLAGS" >> $GITHUB_ENV
  echo "CXXFLAGS=$CXXFLAGS" >> $GITHUB_ENV
  echo "LDFLAGS=$LDFLAGS" >> $GITHUB_ENV
  
  # 清理任何现有构建
  if [ -d "./deps" ]; then
    echo "清理现有构建目录..."
    rm -rf ./deps ./build
  fi
  
  # 确保使用正确的GMP下载
  mkdir -p deps/download
  echo "下载正确架构的源码包..."
  curl -L -o deps/download/gmp-6.2.1.tar.bz2 https://ftp.gnu.org/gnu/gmp/gmp-6.2.1.tar.bz2
  
else
  echo "===== 检测到 x86_64 架构，使用 x86_64 专用编译路径 ====="
  
  # x86_64环境变量
  export CFLAGS="-arch x86_64"
  export CXXFLAGS="-arch x86_64" 
  export LDFLAGS="-arch x86_64"
  
  # 导出到环境中
  echo "CFLAGS=$CFLAGS" >> $GITHUB_ENV
  echo "CXXFLAGS=$CXXFLAGS" >> $GITHUB_ENV
  echo "LDFLAGS=$LDFLAGS" >> $GITHUB_ENV
fi 