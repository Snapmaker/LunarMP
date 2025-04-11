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

# 手动下载和安装GMP
GMP_URL="https://ftp.gnu.org/gnu/gmp/gmp-6.2.1.tar.bz2"
GMP_FILE="gmp-6.2.1.tar.bz2"
GMP_DIR="gmp-6.2.1"
DOWNLOAD_DIR="${DEPS_DIR}/${BUILD_DIR}/down/GMP"

# 创建下载目录
mkdir -p "${DOWNLOAD_DIR}"

if [ ! -f "${DOWNLOAD_DIR}/${GMP_FILE}" ]; then
  echo "直接下载GMP..."
  # 尝试多个下载地址
  if ! curl -L ${GMP_URL} -o "${DOWNLOAD_DIR}/${GMP_FILE}"; then
    echo "从GNU源下载失败，尝试备用源..."
    # 备用下载地址
    BACKUP_URL="https://mirrors.kernel.org/gnu/gmp/${GMP_FILE}"
    if ! curl -L ${BACKUP_URL} -o "${DOWNLOAD_DIR}/${GMP_FILE}"; then
      echo "GMP下载失败，请检查网络连接或手动下载"
      exit 1
    fi
  fi
fi

# 如果下载成功，解压并准备源码
if [ -f "${DOWNLOAD_DIR}/${GMP_FILE}" ]; then
  echo "GMP下载成功，正在解压..."
  # 创建源码目录
  mkdir -p "${DOWNLOAD_DIR}/src"
  tar -xjf "${DOWNLOAD_DIR}/${GMP_FILE}" -C "${DOWNLOAD_DIR}/src"
  # 创建符号链接让CMake可以找到源码
  if [ ! -L "${DOWNLOAD_DIR}/src/gmp" ]; then
    ln -sf "${DOWNLOAD_DIR}/src/${GMP_DIR}" "${DOWNLOAD_DIR}/src/gmp"
  fi
  # 创建完成标记文件，告诉CMake下载已完成
  touch "${DOWNLOAD_DIR}/download-gmp-prefix/src/download-gmp-stamp/download-gmp-download"
fi

# 下载并更新 RapidJSON 库
if [ ! -d "${DEPS_DIR}/rapidjson" ]; then
    git clone https://github.com/Tencent/rapidjson.git "${DEPS_DIR}/rapidjson"
else
    cd "${DEPS_DIR}/rapidjson" && git pull
fi

# 检测处理器架构
ARCH=$(uname -m)
if [ "$ARCH" == "arm64" ] || [ "$ARCH" == "aarch64" ]; then
  echo "检测到ARM64架构"
  CMAKE_EXTRA_ARGS="-DCMAKE_SYSTEM_PROCESSOR=arm64"
else
  CMAKE_EXTRA_ARGS=""
fi

# 运行CMake构建
${CMAKE_DIR} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ${CMAKE_EXTRA_ARGS} -G "${CodeBlocks}" -S ./ -B ${BUILD_DIR}
${CMAKE_DIR} --build ./${BUILD_DIR} --target ${INSTALL_TARGET} -- -j 4

# 在CGAL安装完成后添加
echo "应用CGAL补丁..."
chmod +x patch-cgal.sh
./patch-cgal.sh

# 为CGAL添加兼容性补丁，解决boost::prior的问题
CGAL_INCLUDE_DIR="${DEPS_DIR}/${BUILD_DIR}/install/usr/local/include/CGAL"
if [ -d "${CGAL_INCLUDE_DIR}" ]; then
  echo "修补CGAL以支持新版Boost..."
  # 创建兼容性头文件
  mkdir -p "${CGAL_INCLUDE_DIR}/boost/compat"
  cat > "${CGAL_INCLUDE_DIR}/boost/compat/prior_next.hpp" << EOF
#ifndef CGAL_BOOST_COMPAT_PRIOR_NEXT_HPP
#define CGAL_BOOST_COMPAT_PRIOR_NEXT_HPP

#include <iterator>

namespace boost {
  template <typename Iterator>
  Iterator prior(Iterator it) { return std::prev(it); }
  
  template <typename Iterator>
  Iterator next(Iterator it) { return std::next(it); }
}

#endif // CGAL_BOOST_COMPAT_PRIOR_NEXT_HPP
EOF

  # 修复受影响的文件
  INTERSECTION_FILE="${CGAL_INCLUDE_DIR}/Intersections_3/internal/Plane_3_Triangle_3_intersection.h"
  if [ -f "${INTERSECTION_FILE}" ]; then
    # 确保备份
    cp "${INTERSECTION_FILE}" "${INTERSECTION_FILE}.bak"
    # 添加我们的兼容头文件
    sed -i '1s/^/#include <CGAL\/boost\/compat\/prior_next.hpp>\n/' "${INTERSECTION_FILE}"
    echo "已修补${INTERSECTION_FILE}"
  fi
fi
