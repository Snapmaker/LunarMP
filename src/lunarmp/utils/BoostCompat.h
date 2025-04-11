#ifndef LUNAR_MP_BOOST_COMPAT_H
#define LUNAR_MP_BOOST_COMPAT_H

#include <iterator>

// 确保引入boost/next_prior.hpp
#include <boost/next_prior.hpp>

// 如果需要，提供导入的引用
#ifndef BOOST_ALLOW_DEPRECATED_HEADERS
#define BOOST_ALLOW_DEPRECATED_HEADERS
#endif

// 提供兼容较新版本Boost库的函数
namespace boost {
    template <typename Iterator>
    Iterator prior(Iterator it) {
        return std::prev(it);
    }
}

#endif // LUNAR_MP_BOOST_COMPAT_H 