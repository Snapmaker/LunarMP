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

# 创建临时的兼容性头文件
COMPAT_DIR="${CGAL_DIR}/patches"
mkdir -p "$COMPAT_DIR"

cat > "${COMPAT_DIR}/boost_prior.h" << EOF
// 自动生成的兼容性头文件
#ifndef CGAL_BOOST_PRIOR_COMPAT_H
#define CGAL_BOOST_PRIOR_COMPAT_H

#include <iterator>

// 强制定义boost::prior
#ifndef BOOST_PRIOR_DEFINED
#define BOOST_PRIOR_DEFINED
namespace boost {
  template <typename Iterator>
  Iterator prior(Iterator it) { 
    return std::prev(it); 
  }
  
  template <typename Iterator, typename Distance>
  Iterator prior(Iterator it, Distance n) { 
    return std::prev(it, n); 
  }
}
#endif // BOOST_PRIOR_DEFINED

#endif // CGAL_BOOST_PRIOR_COMPAT_H
EOF

# 查找问题文件并修补
echo "正在搜索包含boost::prior的文件..."
FILES_WITH_PRIOR=$(grep -r "boost::prior" "$CGAL_DIR" --include="*.h" --include="*.hpp" | cut -d: -f1 | sort | uniq)

for file in $FILES_WITH_PRIOR; do
  echo "修补文件: $file"
  
  # 备份原文件
  cp "$file" "$file.bak"
  
  # 在文件开头添加我们的兼容头文件
  sed -i.sedtmp "1i\\
#include \"${COMPAT_DIR}/boost_prior.h\"
" "$file"
  
  # 如果失败，使用直接替换
  sed -i.sedtmp2 's/boost::prior/std::prev/g' "$file"
  
  echo "已修补: $file"
done

# 特别处理已知的问题文件
KNOWN_PROBLEM="${CGAL_DIR}/Intersections_3/internal/Plane_3_Triangle_3_intersection.h"
if [ -f "$KNOWN_PROBLEM" ]; then
  echo "修补已知问题文件: $KNOWN_PROBLEM"
  cp "$KNOWN_PROBLEM" "$KNOWN_PROBLEM.bak"
  sed -i.sedknown 's/boost::prior/std::prev/g' "$KNOWN_PROBLEM"
fi

echo "补丁应用完成" 