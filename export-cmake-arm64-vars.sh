#!/bin/bash

if [[ $(uname -m) == "arm64" ]]; then
  echo "导出ARM64 CMake变量..."
  
  # 获取正确路径
  DEPS_INSTALL_PREFIX="$PWD/deps/build/install/usr/local"
  
  # 创建CMake缓存文件
  cat > arm64-cache.cmake << EOF
set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architecture" FORCE)
set(GMP_INCLUDE_DIR "${DEPS_INSTALL_PREFIX}/include" CACHE PATH "GMP include directory" FORCE)
set(GMP_LIBRARIES "${PWD}/arm64-libs/libgmp.a;${PWD}/arm64-libs/libgmpxx.a" CACHE STRING "GMP libraries" FORCE)
set(MPFR_INCLUDE_DIR "${DEPS_INSTALL_PREFIX}/include" CACHE PATH "MPFR include directory" FORCE)
set(MPFR_LIBRARIES "${PWD}/arm64-libs/libmpfr.a" CACHE STRING "MPFR libraries" FORCE)
set(CMAKE_IGNORE_PATH "/usr/local/lib;/usr/local/include" CACHE STRING "Ignored paths" FORCE)
EOF

  # 使用该缓存文件
  export CMAKE_VARS="-C $PWD/arm64-cache.cmake"
  
  echo "CMake变量导出完成"
fi 