#!/bin/bash

# 特别针对CI环境的路径查找
if [ -n "$GITHUB_WORKSPACE" ]; then
  # 在GitHub Actions环境中
  CGAL_DIR="$GITHUB_WORKSPACE/deps/build/install/usr/local/include/CGAL"
else
  # 查找已安装的CGAL目录
  CGAL_DIR=$(find ./deps/build/install -name CGAL -type d 2>/dev/null | grep -v CMakeFiles | head -1)
  if [ -z "$CGAL_DIR" ]; then
    CGAL_DIR=$(find /usr -name CGAL -type d 2>/dev/null | grep -v CMakeFiles | head -1)
    if [ -z "$CGAL_DIR" ]; then
      CGAL_DIR=$(find /opt -name CGAL -type d 2>/dev/null | grep -v CMakeFiles | head -1)
    fi
  fi
fi

if [ -z "$CGAL_DIR" ]; then
  echo "无法找到CGAL目录"
  exit 1
fi

echo "CGAL目录: $CGAL_DIR"

# 检查CGAL版本
CGAL_VERSION_FILE="$CGAL_DIR/version.h"
if [ -f "$CGAL_VERSION_FILE" ]; then
  CGAL_VERSION=$(grep -o "CGAL_VERSION_NR.*" "$CGAL_VERSION_FILE" | head -1)
  echo "检测到CGAL版本: $CGAL_VERSION"
  
  # 如果是较新版本的CGAL (5.0+)，则减少补丁操作
  if [[ "$CGAL_VERSION" =~ 1050 ]] || [[ "$CGAL_VERSION" =~ 1060 ]]; then
    echo "检测到较新版本的CGAL，减少补丁操作"
    exit 0
  fi
fi

# 创建补丁目录
mkdir -p "$CGAL_DIR/patches/"

# 创建boost_prior.h补丁文件
cat > "$CGAL_DIR/patches/boost_prior.h" << 'EOF'
#ifndef CGAL_BOOST_PRIOR_PATCH_H
#define CGAL_BOOST_PRIOR_PATCH_H

#include <iterator>

namespace boost {
    template <class T>
    inline T prior(T x) { return --x; }

    template <class T, class Distance>
    inline T prior(T x, Distance n) {
        std::advance(x, -n);
        return x;
    }

    template <class T>
    inline T next(T x) { return ++x; }

    template <class T, class Distance>
    inline T next(T x, Distance n) {
        std::advance(x, n);
        return x;
    }
}

#endif // CGAL_BOOST_PRIOR_PATCH_H
EOF

echo "正在搜索包含boost::prior的文件..."

# 寻找和修补使用boost::prior的CGAL文件
find "$CGAL_DIR" -type f -name "*.h" -exec grep -l "boost::prior" {} \; | while read file; do
    echo "修补文件: $file"
    # 在文件顶部添加include语句，而不是替换boost::prior
    if ! grep -q "#include.*boost_prior.h" "$file"; then
        sed -i.bak '1s/^/#include <CGAL\/patches\/boost_prior.h>\n/' "$file"
        echo "已修补: $file"
    fi
done

# 特别处理已知有问题的文件
PLANE_TRIANGLE_FILE="$CGAL_DIR/Intersections_3/internal/Plane_3_Triangle_3_intersection.h"
if [ -f "$PLANE_TRIANGLE_FILE" ]; then
    echo "修补已知问题文件: $PLANE_TRIANGLE_FILE"
    sed -i.bak 's/boost::prior/::boost::prior/g' "$PLANE_TRIANGLE_FILE"
fi

echo "补丁应用完成" 