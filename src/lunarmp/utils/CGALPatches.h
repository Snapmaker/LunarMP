#ifndef LUNAR_MP_CGAL_PATCHES_H
#define LUNAR_MP_CGAL_PATCHES_H

// 在CGAL头文件之前包含此文件
#include <iterator>

// 定义一个辅助函数来替代boost::prior调用
namespace cgal_compat {
    template <typename Iterator>
    Iterator prior(Iterator it) {
        return std::prev(it);
    }
}

// 使用宏重定向boost::prior到std::prev
#define boost::prior(x) std::prev(x)

#endif // LUNAR_MP_CGAL_PATCHES_H 