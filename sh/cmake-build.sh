#!/bin/bash

CMAKE_DIR=cmake
BUILD_TYPE=Release
BUILD_DIR=build
CodeBlocks="CodeBlocks - MinGW Makefiles"

if [ "$(uname)" == "Darwin" ]; then
  CodeBlocks="CodeBlocks - Unix Makefiles"
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
  CodeBlocks="CodeBlocks - Unix Makefiles"
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
PROJECT_DIR="${SCRIPT_DIR}/.."

DEPS=${PROJECT_DIR}/deps/build/install/usr/local

if [ ! -d "${PROJECT_DIR}/${BUILD_DIR}" ]; then
    mkdir "${PROJECT_DIR}/${BUILD_DIR}"
fi

cd "${PROJECT_DIR}" || exit

# 在构建之前应用CGAL补丁
echo "应用CGAL补丁..."
chmod +x patch-cgal.sh
./patch-cgal.sh

echo "${CMAKE_DIR} -DCMAKE_PREFIX_PATH="${DEPS}" -DPKG_CONFIG_PATH="${DEPS}" -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -G "${CodeBlocks}" -S ./ -B ${BUILD_DIR}"
${CMAKE_DIR} -DCMAKE_PREFIX_PATH="${DEPS}" -DPKG_CONFIG_PATH="${DEPS}" -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -G "${CodeBlocks}" -S ./ -B ${BUILD_DIR}
${CMAKE_DIR} --build ./${BUILD_DIR} --target LunarMP -- -j 4