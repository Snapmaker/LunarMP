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

# CGAL and its components
find_package(CGAL REQUIRED)
target_include_directories(_LunarMP PUBLIC ${CGAL_INCLUDE_DIRS})

# Boost and its components - 首先尝试找到特定版本
find_package(Boost 1.76 EXACT REQUIRED COMPONENTS graph)

# 如果无法找到特定版本，则查找范围内的版本
if(NOT Boost_FOUND)
  find_package(Boost 1.65.1...1.76.0 REQUIRED COMPONENTS graph)
endif()

# 如果仍未找到，则查找任何可用版本
if(NOT Boost_FOUND)
  message(STATUS "尝试查找任何可用的Boost版本...")
  find_package(Boost REQUIRED COMPONENTS graph)
endif()

message(STATUS "找到的Boost版本: ${Boost_VERSION}")
message(STATUS "Boost包含目录: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost库目录: ${Boost_LIBRARY_DIRS}")

target_link_libraries(_LunarMP PUBLIC CGAL::CGAL)

target_include_directories(_LunarMP PUBLIC src/rapidjson)

# 添加兼容性相关定义
add_definitions(-DBOOST_ALLOW_DEPRECATED_HEADERS)
add_definitions(-DBOOST_PENDING_INTEGER_LOG2_HPP)
add_definitions(-DCGAL_DISABLE_ROUNDING_MATH_CHECK=ON)

# 添加包含路径，确保我们的兼容性头文件优先级高于CGAL
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# 删除有问题的宏定义
# add_definitions(-Dboost::prior=std::prev)

add_executable(LunarMP src/lunarmp/main.cpp)

target_link_libraries(LunarMP _LunarMP)

target_include_directories(LunarMP PUBLIC ${CGAL_INCLUDE_DIRS})
