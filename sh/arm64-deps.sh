#!/bin/bash

# 针对ARM64专用的依赖构建脚本
echo "===== 开始ARM64专用依赖构建 ====="

# 创建必要的目录
WORK_DIR="$PWD"
DEPS_DIR="$WORK_DIR/deps"
DOWNLOAD_DIR="$DEPS_DIR/download"
BUILD_DIR="$DEPS_DIR/build"
INSTALL_PREFIX="$BUILD_DIR/install/usr/local"

mkdir -p "$DOWNLOAD_DIR"
mkdir -p "$BUILD_DIR"
mkdir -p "$INSTALL_PREFIX/lib"
mkdir -p "$INSTALL_PREFIX/include"

# 设置ARM64专用编译标志
export CFLAGS="-arch arm64 -O2"
export CXXFLAGS="-arch arm64 -O2"
export LDFLAGS="-arch arm64"

# 下载和构建GMP（arm64版本）
cd "$DOWNLOAD_DIR"
if [ ! -f gmp-6.2.1.tar.bz2 ]; then
  echo "下载GMP源码..."
  curl -L -o gmp-6.2.1.tar.bz2 https://ftp.gnu.org/gnu/gmp/gmp-6.2.1.tar.bz2
fi

cd "$BUILD_DIR"
echo "解压并编译GMP（ARM64版本）..."
tar -xf "$DOWNLOAD_DIR/gmp-6.2.1.tar.bz2"
cd gmp-6.2.1
./configure --prefix="$INSTALL_PREFIX" --build=arm64-apple-darwin --host=arm64-apple-darwin --enable-static=yes --enable-shared=no ABI=arm64-darwin 
make -j$(sysctl -n hw.ncpu)
make install

# 下载和构建MPFR（arm64版本）
cd "$DOWNLOAD_DIR"
if [ ! -f mpfr-4.1.0.tar.bz2 ]; then
  echo "下载MPFR源码..."
  curl -L -o mpfr-4.1.0.tar.bz2 https://ftp.gnu.org/gnu/mpfr/mpfr-4.1.0.tar.bz2
fi

cd "$BUILD_DIR"
echo "解压并编译MPFR（ARM64版本）..."
tar -xf "$DOWNLOAD_DIR/mpfr-4.1.0.tar.bz2"
cd mpfr-4.1.0
./configure --prefix="$INSTALL_PREFIX" --with-gmp="$INSTALL_PREFIX" --build=arm64-apple-darwin --host=arm64-apple-darwin --enable-static=yes --enable-shared=no
make -j$(sysctl -n hw.ncpu)
make install

# 添加GMP C++接口的编译
cd "$BUILD_DIR/gmp-6.2.1"
echo "编译GMP C++接口（ARM64版本）..."
make -j$(sysctl -n hw.ncpu) CXX=c++
cp .libs/libgmpxx.a "$INSTALL_PREFIX/lib/"
cp -r c++ "$INSTALL_PREFIX/include/gmpxx"

# 创建特定的arm64配置文件，以确保CMake使用正确的库
cat > "$INSTALL_PREFIX/lib/cmake/gmp-arm64-config.cmake" << EOF
# GMP ARM64配置文件
set(GMP_INCLUDE_DIRS "${INSTALL_PREFIX}/include")
set(GMP_LIBRARIES "${INSTALL_PREFIX}/lib/libgmp.a")
set(GMPXX_LIBRARIES "${INSTALL_PREFIX}/lib/libgmpxx.a")
set(GMP_FOUND TRUE)
EOF

# 创建一个符号链接，确保CMake能找到我们的ARM64库
mkdir -p "$INSTALL_PREFIX/lib/cmake/gmp"
ln -sf "$INSTALL_PREFIX/lib/cmake/gmp-arm64-config.cmake" "$INSTALL_PREFIX/lib/cmake/gmp/gmp-config.cmake"

# 显示构建的库信息
echo "GMP库信息："
file "$INSTALL_PREFIX/lib/libgmp.a"
file "$INSTALL_PREFIX/lib/libgmpxx.a"
echo "MPFR库信息："
file "$INSTALL_PREFIX/lib/libmpfr.a"

echo "===== ARM64专用依赖构建完成 =====" 