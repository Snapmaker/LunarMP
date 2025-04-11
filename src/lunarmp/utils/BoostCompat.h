#ifndef LUNAR_MP_BOOST_COMPAT_H
#define LUNAR_MP_BOOST_COMPAT_H

#include <iterator>

// 确保引入boost头文件
#include <boost/version.hpp>

// 从源代码错误来看，boost::prior已经定义但CGAL无法找到它
// 我们只提供一个简单的命名空间级别的定义
namespace boost {
    // 这个函数会在boost::prior不存在时被使用
    template <typename Iterator>
    Iterator prior(Iterator it) {
        return std::prev(it);
    }
}

#endif // LUNAR_MP_BOOST_COMPAT_H 