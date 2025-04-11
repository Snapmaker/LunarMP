#ifndef LUNAR_MP_BOOST_COMPAT_H
#define LUNAR_MP_BOOST_COMPAT_H

#include <iterator>

// 提供兼容较新版本Boost库的函数
namespace boost {
    template <typename Iterator>
    Iterator prior(Iterator it) {
        return std::prev(it);
    }
}

#endif // LUNAR_MP_BOOST_COMPAT_H 