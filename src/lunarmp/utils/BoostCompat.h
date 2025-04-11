#ifndef LUNAR_MP_BOOST_COMPAT_H
#define LUNAR_MP_BOOST_COMPAT_H

#include <iterator>

// 我们不需要重定义boost::prior，而是需要确保 
// CGAL正确使用现有的prior
#include <boost/version.hpp>

// 由于我们不能修改整个项目的源代码，我们使用补丁脚本
// 运行patch-cgal.sh来修复CGAL源码

#endif // LUNAR_MP_BOOST_COMPAT_H 