#!/bin/bash

# 如果检测到ARM64架构，则修复链接设置
if [[ $(uname -m) == "arm64" ]]; then
  echo "检测到ARM64架构，应用链接修复..."
  
  # 获取正确路径
  DEPS_INSTALL_PREFIX="$PWD/deps/build/install/usr/local"
  
  # 创建链接器配置文件
  cat > arm64-gmp-link.cmake << EOF
# 强制使用我们自己构建的ARM64库
set(GMP_INCLUDE_DIR "${DEPS_INSTALL_PREFIX}/include" CACHE PATH "GMP include directory" FORCE)
set(GMP_LIBRARIES "${DEPS_INSTALL_PREFIX}/lib/libgmp.a;${DEPS_INSTALL_PREFIX}/lib/libgmpxx.a" CACHE FILEPATH "GMP libraries" FORCE) 
set(MPFR_INCLUDE_DIR "${DEPS_INSTALL_PREFIX}/include" CACHE PATH "MPFR include directory" FORCE)
set(MPFR_LIBRARIES "${DEPS_INSTALL_PREFIX}/lib/libmpfr.a" CACHE FILEPATH "MPFR libraries" FORCE)
EOF

  # 使环境变量覆盖系统库路径
  export LIBRARY_PATH="$DEPS_INSTALL_PREFIX/lib:$LIBRARY_PATH"
  export CPATH="$DEPS_INSTALL_PREFIX/include:$CPATH"
  export CPLUS_INCLUDE_PATH="$DEPS_INSTALL_PREFIX/include:$CPLUS_INCLUDE_PATH"
  export C_INCLUDE_PATH="$DEPS_INSTALL_PREFIX/include:$C_INCLUDE_PATH"
  
  echo "ARM64链接修复应用完成"
fi 