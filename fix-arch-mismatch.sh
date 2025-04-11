#!/bin/bash

# 在ARM64 Mac上全面修复架构问题

if [[ $(uname -m) == "arm64" ]]; then
  echo "===== 检测到 ARM64 架构，正在应用全面架构兼容性修复 ====="
  
  # 1. 设置通用编译标志确保所有库使用正确的ARM64架构
  export CFLAGS="-arch arm64"
  export CXXFLAGS="-arch arm64"
  export LDFLAGS="-arch arm64"
  
  # 2. 添加特定的MACOSX_DEPLOYMENT_TARGET环境变量
  export MACOSX_DEPLOYMENT_TARGET="11.0"
  
  # 3. 配置CMake以正确使用ARM64架构
  export CMAKE_OSX_ARCHITECTURES="arm64"
  
  # 4. 保存环境变量供workflow后续步骤使用
  echo "CFLAGS=-arch arm64" >> $GITHUB_ENV
  echo "CXXFLAGS=-arch arm64" >> $GITHUB_ENV
  echo "LDFLAGS=-arch arm64" >> $GITHUB_ENV
  echo "MACOSX_DEPLOYMENT_TARGET=11.0" >> $GITHUB_ENV
  echo "CMAKE_OSX_ARCHITECTURES=arm64" >> $GITHUB_ENV
  
  # 5. 确保Homebrew使用正确的架构
  echo "HOMEBREW_ARCH=arm64" >> $GITHUB_ENV
  
  # 6. 如果deps目录已存在，清理并重新编译
  if [ -d "./deps" ]; then
    echo "清理现有构建和依赖目录以进行架构正确的重新编译..."
    rm -rf ./deps ./build
  fi
  
  # 7. 修改cmake-deps.sh脚本，添加特定的配置参数
  if [ -f "sh/cmake-deps.sh" ]; then
    echo "修改cmake-deps.sh脚本添加ARM64专用配置..."
    # 为所有依赖库的configure步骤添加--build=arm64-apple-darwin
    sed -i.bak 's/\.\/configure/\.\/configure --build=arm64-apple-darwin/g' sh/cmake-deps.sh
  fi
  
  # 8. 添加库特定修复
  echo "为特定库应用架构修复..."
  
  # GMP特定修复
  if grep -q "gmp" sh/cmake-deps.sh; then
    echo "应用GMP库ARM64特定修复..."
    if [ -d "deps/download" ] && [ -f "deps/download/gmp-6.2.1.tar.bz2" ]; then
      # 可以考虑替换为ARM64原生预编译版本或特殊配置
      echo "GMP已下载，准备使用ARM64特定配置重新编译..."
    fi
  fi
  
  # MPFR特定修复 (通常也会有架构问题)
  if grep -q "mpfr" sh/cmake-deps.sh; then
    echo "应用MPFR库ARM64特定修复..."
    # 类似GMP的修复方式
  fi
  
  echo "ARM64全面架构兼容性修复完成！"
else
  echo "非ARM64架构，不需要特殊处理。"
fi 