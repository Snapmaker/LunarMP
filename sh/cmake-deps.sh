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

# 使用更可靠的GMP下载URL并确保传递给CMake
GMP_URL="https://ftp.gnu.org/gnu/gmp/gmp-6.2.1.tar.bz2"

# 创建或修改CMake缓存初始化文件，强制使用我们的URL
cat > ${BUILD_DIR}/gmp_download_url.cmake << EOF
set(GMP_URL "${GMP_URL}" CACHE STRING "URL for downloading GMP" FORCE)
EOF

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

# 在CMAKE调用中使用-C选项加载我们的初始缓存文件
${CMAKE_DIR} -C ${BUILD_DIR}/gmp_download_url.cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ${CMAKE_EXTRA_ARGS} -G "${CodeBlocks}" -S ./ -B ${BUILD_DIR}
${CMAKE_DIR} --build ./${BUILD_DIR} --target ${INSTALL_TARGET} -- -j 4

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
