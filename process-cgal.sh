#!/bin/bash

echo "开始全面处理CGAL与Boost兼容性问题..."

# 步骤1：创建兼容层
mkdir -p boost_compat/boost
cat > boost_compat/boost/next_prior.hpp << 'EOF'
#ifndef BOOST_NEXT_PRIOR_HPP_INCLUDED
#define BOOST_NEXT_PRIOR_HPP_INCLUDED

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

#endif // BOOST_NEXT_PRIOR_HPP_INCLUDED
EOF

# 步骤2：等待CGAL安装完成
echo "等待CGAL安装完成..."
DEPS_DIR="./deps/build/install/usr/local/include/CGAL"
SYSTEM_CGAL="/usr/local/include/CGAL"
BREW_CGAL="$(brew --prefix 2>/dev/null)/include/CGAL"

if [ -d "$DEPS_DIR" ]; then
    CGAL_DIR="$DEPS_DIR"
elif [ -d "$SYSTEM_CGAL" ]; then
    CGAL_DIR="$SYSTEM_CGAL"
elif [ -d "$BREW_CGAL" ]; then
    CGAL_DIR="$BREW_CGAL"
else
    echo "找不到CGAL目录，将在构建过程中再次尝试..."
    # 在没找到CGAL目录的情况下也能继续运行
    exit 0
fi

echo "找到CGAL目录: $CGAL_DIR"

# 步骤3：全面修复boost::prior/next引用
echo "全面查找和修复boost::prior/next引用..."

# 先对所有可能使用boost::prior的文件进行处理
find "$CGAL_DIR" -type f -name "*.h" -o -name "*.hpp" | xargs grep -l "boost::prior\|boost::next" | while read file; do
    echo "处理文件: $file"
    
    # 在文件开头添加兼容层 - 确保不重复添加
    if ! grep -q "#include .*/boost/next_prior.hpp" "$file"; then
        sed -i.bak '1s/^/#include <boost\/next_prior.hpp>\n/' "$file"
    fi
    
    # 可选：更直接的方法是替换全部boost::prior/next为标准库函数
    # sed -i.bak2 's/boost::prior/std::prev/g' "$file"
    # sed -i.bak3 's/boost::next/std::next/g' "$file"
done

# 步骤4：输出编译相关的环境变量到文件，供后续步骤使用
echo "输出编译环境变量..."
echo "CPPFLAGS=-I$(pwd)/boost_compat" > compile_env.sh
echo "BOOST_INCLUDEDIR=$(brew --prefix boost 2>/dev/null)/include" >> compile_env.sh

echo "CGAL/Boost兼容性处理完成！" 