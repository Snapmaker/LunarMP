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

${CMAKE_DIR} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -G "${CodeBlocks}" -S ./ -B ${BUILD_DIR}
${CMAKE_DIR} --build ./${BUILD_DIR} --target ${INSTALL_TARGET} -- -j 4
