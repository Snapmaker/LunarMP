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

# 创建补丁目录
mkdir -p "/Users/runner/work/LunarMP/LunarMP/deps/build/install/usr/local/include/CGAL/patches/"

# 创建正确的替代头文件 - 修复循环包含问题
cat > "/Users/runner/work/LunarMP/LunarMP/deps/build/install/usr/local/include/CGAL/patches/boost_prior.h" << 'EOF'
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
find "/Users/runner/work/LunarMP/LunarMP/deps/build/install/usr/local/include/CGAL" -type f -name "*.h" -exec grep -l "boost::prior" {} \; | while read file; do
    echo "修补文件: $file"
    # 在文件顶部添加include语句，而不是替换boost::prior
    if ! grep -q "#include.*boost_prior.h" "$file"; then
        sed -i.bak '1s/^/#include <CGAL\/patches\/boost_prior.h>\n/' "$file"
        echo "已修补: $file"
    fi
done

# 特别处理已知有问题的文件
echo "修补已知问题文件: /Users/runner/work/LunarMP/LunarMP/deps/build/install/usr/local/include/CGAL/Intersections_3/internal/Plane_3_Triangle_3_intersection.h"
sed -i.bak 's/boost::prior/::boost::prior/g' "/Users/runner/work/LunarMP/LunarMP/deps/build/install/usr/local/include/CGAL/Intersections_3/internal/Plane_3_Triangle_3_intersection.h"

echo "补丁应用完成" 