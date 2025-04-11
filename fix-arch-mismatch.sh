#!/bin/bash

# 架构检测和对应处理
if [[ $(uname -m) == "arm64" ]]; then
  echo "===== 检测到 ARM64 架构，使用 ARM64 专用编译路径 ====="
  
  # 清理任何现有构建
  if [ -d "./deps" ]; then
    echo "清理现有构建目录..."
    rm -rf ./deps ./build
  fi
  
  # ARM64专用环境变量
  export CFLAGS="-arch arm64"
  export CXXFLAGS="-arch arm64"
  export LDFLAGS="-arch arm64"
  
  # 设置cmake特定参数
  export CMAKE_ARGS="-DCMAKE_OSX_ARCHITECTURES=arm64"
  
  # 导出到环境中
  echo "CFLAGS=$CFLAGS" >> $GITHUB_ENV
  echo "CXXFLAGS=$CXXFLAGS" >> $GITHUB_ENV
  echo "LDFLAGS=$LDFLAGS" >> $GITHUB_ENV
  echo "CMAKE_ARGS=$CMAKE_ARGS" >> $GITHUB_ENV
  
  # 运行ARM64专用依赖构建脚本而不是标准脚本
  echo "使用ARM64专用依赖构建脚本..."
  chmod +x sh/arm64-deps.sh
  ./sh/arm64-deps.sh
  
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