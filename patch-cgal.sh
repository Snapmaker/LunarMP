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

# 创建兼容性头文件
mkdir -p "${CGAL_DIR}/boost/compat"
cat > "${CGAL_DIR}/boost/compat/prior_next.hpp" << EOF
#ifndef CGAL_BOOST_COMPAT_PRIOR_NEXT_HPP
#define CGAL_BOOST_COMPAT_PRIOR_NEXT_HPP

#include <iterator>

namespace boost {
  template <typename Iterator>
  Iterator prior(Iterator it) { return std::prev(it); }
  
  template <typename Iterator, typename Distance>
  Iterator prior(Iterator it, Distance n) { return std::prev(it, n); }
  
  template <typename Iterator>
  Iterator next(Iterator it) { return std::next(it); }
  
  template <typename Iterator, typename Distance>
  Iterator next(Iterator it, Distance n) { return std::next(it, n); }
}

#endif // CGAL_BOOST_COMPAT_PRIOR_NEXT_HPP
EOF

# 修补指定的文件
PROBLEM_FILE="${CGAL_DIR}/Intersections_3/internal/Plane_3_Triangle_3_intersection.h"

if [ -f "$PROBLEM_FILE" ]; then
  echo "修补文件: $PROBLEM_FILE"
  # 确保备份
  cp "$PROBLEM_FILE" "$PROBLEM_FILE.bak"
  
  # 在文件开头添加我们的兼容性头文件
  sed -i.bak '1s/^/#include <CGAL\/boost\/compat\/prior_next.hpp>\n/' "$PROBLEM_FILE"
  
  # 替换所有的boost::prior调用为std::prev
  sed -i.bak2 's/boost::prior/std::prev/g' "$PROBLEM_FILE"
  
  echo "已成功修补 $PROBLEM_FILE"
else
  echo "警告: 无法找到问题文件 $PROBLEM_FILE"
  
  # 尝试找到所有包含boost::prior的文件
  echo "正在搜索所有包含boost::prior的CGAL文件进行修补..."
  PRIOR_FILES=$(grep -r "boost::prior" "$CGAL_DIR" --include="*.h" --include="*.hpp" | cut -d':' -f1 | sort | uniq)
  
  for file in $PRIOR_FILES; do
    if [ -f "$file" ]; then
      echo "修补文件: $file"
      cp "$file" "$file.bak"
      sed -i.bak 's/boost::prior/std::prev/g' "$file"
    fi
  done
fi

echo "补丁应用完成" 