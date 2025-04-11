#!/bin/bash

# 找到 CGAL 包含目录
CGAL_DIR=$(find /usr -name CGAL -type d 2>/dev/null | grep -v CMakeFiles | head -1)
if [ -z "$CGAL_DIR" ]; then
  CGAL_DIR=$(find /opt -name CGAL -type d 2>/dev/null | grep -v CMakeFiles | head -1)
fi

if [ -z "$CGAL_DIR" ]; then
  echo "无法找到CGAL目录"
  exit 1
fi

echo "CGAL目录: $CGAL_DIR"

# 修补存在问题的文件
FILES_TO_PATCH=(
  "$CGAL_DIR/Intersections_3/internal/Plane_3_Triangle_3_intersection.h"
)

for file in "${FILES_TO_PATCH[@]}"; do
  if [ -f "$file" ]; then
    echo "修补文件: $file"
    # 替换 boost::prior 为 std::prev
    sed -i'.bak' 's/boost::prior/std::prev/g' "$file"
  else
    echo "文件不存在: $file"
  fi
done

echo "补丁应用完成" 