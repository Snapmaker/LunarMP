cmake_minimum_required(VERSION 3.16)

project(LunarMP VERSION 1.0.0)

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

## MPFR
#find_package(MPFR REQUIRED)
#
## GMP
#find_package(GMP REQUIRED)

# EIGEN3
find_package(EIGEN3 REQUIRED)
target_include_directories(_LunarMP PUBLIC ${EIGEN3_INCLUDE_DIR})

# CGAL and its components
find_package(CGAL REQUIRED)
target_include_directories(_LunarMP PUBLIC ${CGAL_INCLUDE_DIRS})

# Boost and its components
find_package(Boost REQUIRED)

target_link_libraries(_LunarMP PUBLIC CGAL::CGAL)

target_include_directories(_LunarMP PUBLIC src/rapidjson)

add_executable(LunarMP src/lunarmp/main.cpp)

target_link_libraries(LunarMP _LunarMP)

target_include_directories(LunarMP PUBLIC ${CGAL_INCLUDE_DIRS})
