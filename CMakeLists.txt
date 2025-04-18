cmake_minimum_required(VERSION 3.16)

project(LunarMP VERSION 1.0.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_PREFIX_PATH "/home/linuxbrew/.linuxbrew")

set(CMAKE_CXX_STANDARD 14)

set(PROJECT_SRCS
        src/lunarmp/Application.cpp

        src/lunarmp/communication/CommandLine.cpp

        src/lunarmp/settings/Settings.cpp

        src/lunarmp/controller/TaskWorker.cpp
        src/lunarmp/controller/ModelController.cpp

        src/lunarmp/model/ModelCheck.cpp
        src/lunarmp/model/ModelRepair.cpp
        src/lunarmp/model/ModelCompare.cpp
        src/lunarmp/model/ModelSimplification.cpp

        src/lunarmp/test/ModelTest.cpp

        src/lunarmp/utils/logoutput.cpp
        src/lunarmp/utils/SMProgress.cpp
        src/lunarmp/utils/TimeKeeper.cpp
        )

add_library(_LunarMP STATIC ${PROJECT_SRCS})

# GMP
find_package(GMP REQUIRED)
target_include_directories(_LunarMP PUBLIC ${GMP_INCLUDE_DIR})
target_link_libraries(_LunarMP PUBLIC ${GMP_LIBRARIES})

# MPFR
find_package(MPFR REQUIRED)

# EIGEN3
find_package(Eigen3 REQUIRED)
target_include_directories(_LunarMP PUBLIC ${EIGEN3_INCLUDE_DIR})

# CGAL - 查找最新版本，不指定特定版本
find_package(CGAL REQUIRED)
target_include_directories(_LunarMP PUBLIC ${CGAL_INCLUDE_DIRS})

# Boost - 查找任何可用版本，不再指定特定版本
find_package(Boost REQUIRED COMPONENTS graph)

message(STATUS "找到的Boost版本: ${Boost_VERSION}")
message(STATUS "Boost包含目录: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost库目录: ${Boost_LIBRARY_DIRS}")

# 链接CGAL库
target_link_libraries(_LunarMP PUBLIC CGAL::CGAL)

target_include_directories(_LunarMP PUBLIC src/rapidjson)

# 添加兼容性相关定义 - 使用新版Boost和CGAL时仍然需要部分标志
add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)
add_definitions(-DBOOST_DISABLE_PRAGMA_MESSAGE)
add_definitions(-DCGAL_DISABLE_ROUNDING_MATH_CHECK=ON)

# 确保我们的兼容性头文件优先级高于CGAL
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# 确保兼容层优先被包含
include_directories(BEFORE "${CMAKE_CURRENT_SOURCE_DIR}/boost_compat")

# 删除有问题的宏定义
# add_definitions(-Dboost::prior=std::prev)

add_executable(LunarMP src/lunarmp/main.cpp)

target_link_libraries(LunarMP _LunarMP)

target_include_directories(LunarMP PUBLIC ${CGAL_INCLUDE_DIRS})
