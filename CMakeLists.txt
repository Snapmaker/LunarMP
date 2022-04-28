cmake_minimum_required(VERSION 3.16)

project(LunarMP VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 14)

set(PROJECT_SRCS
        src/lunarmp/model/ModelCheck.cpp
        src/lunarmp/model/ModelRepair.cpp
        src/lunarmp/model/ModelCompare.cpp
        src/lunarmp/model/ModelSimplification.cpp

        srd/lunarmp/test/ModelTest.cpp

        src/lunarmp/utils/logoutput.cpp
        )

add_library(_LunarMP STATIC ${PROJECT_SRCS})

target_include_directories(_LunarMP PUBLIC "C:/msys64/mingw64/include/")

# CGAL and its components
find_package(CGAL REQUIRED)
target_include_directories(_LunarMP PUBLIC ${CGAL_INCLUDE_DIRS})

find_package(EIGEN3 REQUIRED)
target_include_directories(_LunarMP PUBLIC ${EIGEN3_INCLUDE_DIR})

# Boost and its components
find_package(Boost REQUIRED)


target_link_libraries(_LunarMP PUBLIC CGAL::CGAL)

add_executable(LunarMP src/lunarmp/main.cpp)

target_link_libraries(LunarMP _LunarMP)

target_include_directories(LunarMP PUBLIC ${CGAL_INCLUDE_DIRS})
