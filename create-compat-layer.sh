#!/bin/bash

# 创建boost兼容层目录
mkdir -p boost_compat/boost

# 创建boost/next_prior.hpp文件
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

echo "兼容层创建完成" 