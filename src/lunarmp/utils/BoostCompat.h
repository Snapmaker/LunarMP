#ifndef LUNAR_MP_BOOST_COMPAT_H
#define LUNAR_MP_BOOST_COMPAT_H

#include <iterator>

// 确保引入boost/next_prior.hpp
#include <boost/next_prior.hpp>

// 启用boost弃用的头文件
#ifndef BOOST_ALLOW_DEPRECATED_HEADERS
#define BOOST_ALLOW_DEPRECATED_HEADERS
#endif

// 如果boost::prior不可用，使用宏定义提供兼容性
#ifndef BOOST_NO_CXX11_DECLTYPE_N3276
  #ifndef BOOST_PRIOR_DEFINED
  #define BOOST_PRIOR_DEFINED
  namespace boost {
    // 使用SFINAE检查prior是否已经存在
    template<typename T>
    static auto check_prior(T* t) -> decltype(prior(std::declval<T>()), std::true_type());
    static std::false_type check_prior(...);

    // 仅当prior不存在时提供定义
    template<typename Iterator, 
             typename = typename std::enable_if<!decltype(check_prior((Iterator*)nullptr))::value>::type>
    Iterator prior_compat(Iterator it) {
        return std::prev(it);
    }
  }
  // 重定向CGAL对prior的调用到我们的兼容函数
  #define prior prior_compat
  #endif
#endif

#endif // LUNAR_MP_BOOST_COMPAT_H 