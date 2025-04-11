#!/bin/bash

CMAKE_DIR=cmake
BUILD_TYPE=Release
BUILD_DIR=build
CodeBlocks="CodeBlocks - MinGW Makefiles"
INSTALL_TARGET="CGAL"

if [ "$(uname)" == "Darwin" ]; then
  CodeBlocks="CodeBlocks - Unix Makefiles"
  INSTALL_TARGET="GMP CGAL"
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
  CodeBlocks="CodeBlocks - Unix Makefiles"
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
DEPS_DIR="${SCRIPT_DIR}/../deps"

if [ ! -d "${DEPS_DIR}/${BUILD_DIR}" ]; then
    mkdir "${DEPS_DIR}/${BUILD_DIR}"
fi

cd ${DEPS_DIR} || exit

# 在脚本中直接修改GMP下载URL
# 找到类似这样的行：
# GMP_URL="https://gmplib.org/download/gmp/gmp-6.2.1.tar.bz2"
# 将其改为：
GMP_URL="https://ftp.gnu.org/gnu/gmp/gmp-6.2.1.tar.bz2"

# 在适当位置添加
# 下载并更新 RapidJSON 库
if [ ! -d "${DEPS_DIR}/rapidjson" ]; then
    git clone https://github.com/Tencent/rapidjson.git "${DEPS_DIR}/rapidjson"
else
    cd "${DEPS_DIR}/rapidjson" && git pull
fi

# 在文件中添加处理器架构检测
ARCH=$(uname -m)
if [ "$ARCH" == "arm64" ] || [ "$ARCH" == "aarch64" ]; then
  echo "检测到ARM64架构"
  CMAKE_EXTRA_ARGS="-DCMAKE_SYSTEM_PROCESSOR=arm64"
else
  CMAKE_EXTRA_ARGS=""
fi

# 然后在CMAKE调用中使用这些额外参数
${CMAKE_DIR} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ${CMAKE_EXTRA_ARGS} -G "${CodeBlocks}" -S ./ -B ${BUILD_DIR}
${CMAKE_DIR} --build ./${BUILD_DIR} --target ${INSTALL_TARGET} -- -j 4
