#ifndef LUNAR_MP_CGAL_FIXES_H
#define LUNAR_MP_CGAL_FIXES_H

// 包含这个头文件在所有CGAL头文件之前
#include <iterator>

// 临时重定向boost::prior到std::prev，这是一个简单的解决方案
namespace boost {
  template <typename Iterator>
  inline Iterator prior(Iterator it) { 
    return std::prev(it); 
  }
  
  template <typename Iterator, typename Distance>
  inline Iterator prior(Iterator it, Distance n) { 
    return std::prev(it, n); 
  }
}

#endif // LUNAR_MP_CGAL_FIXES_H 