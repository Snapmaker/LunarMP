cmake_minimum_required(VERSION 3.16)

project(LunarMP-deps)

include(ExternalProject)
include(ProcessorCount)

ProcessorCount(NPROC)
if (NPROC EQUAL 0)
    set(NPROC 1)
endif ()

set(DEPS_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/install" CACHE PATH "Install directory")
set(DEPS_DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/down" CACHE PATH "Path for downloaded source packages.")

message(STATUS "LunarMP deps DEPS_INSTALL_DIR: ${DEPS_INSTALL_DIR}")

function(add_deps_cmake_project projectname)
    cmake_parse_arguments(P_ARGS "" "INSTALL_DIR;BUILD_COMMAND;INSTALL_COMMAND" "CMAKE_ARGS" ${ARGN})

    set(_configs_line -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE})
    if (_is_multi OR MSVC)
        set(_configs_line "")
    endif ()

    set(_gen "")
    set(_build_j "-j${NPROC}")
    # if(MSVC)
    #   set(_gen CMAKE_GENERATOR "${DEP_MSVC_GEN}" CMAKE_GENERATOR_PLATFORM
    #            "${DEP_PLATFORM}")
    #   set(_build_j "/m")
    # endif()
    ExternalProject_Add(
            ${projectname}
            EXCLUDE_FROM_ALL ON
            INSTALL_DIR ${DEPS_INSTALL_DIR}/usr/local
            DOWNLOAD_DIR ${DEPS_DOWNLOAD_DIR}/${projectname}
            ${_gen}
            CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX:STRING=${DEPS_INSTALL_DIR}/usr/local
            -DCMAKE_MODULE_PATH:STRING=${PROJECT_SOURCE_DIR}/../cmake
            -DCMAKE_PREFIX_PATH:STRING=${DEPS_INSTALL_DIR}/usr/local
            -DCMAKE_DEBUG_POSTFIX:STRING=d
            -DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}
            -DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}
            -DCMAKE_TOOLCHAIN_FILE:STRING=${CMAKE_TOOLCHAIN_FILE}
            -DBUILD_SHARED_LIBS:BOOL=OFF
            "${_configs_line}"
            ${DEP_CMAKE_OPTS}
            ${P_ARGS_CMAKE_ARGS}
            ${P_ARGS_UNPARSED_ARGUMENTS}
            BUILD_COMMAND ${CMAKE_COMMAND} --build . --config Release -- ${_build_j}
            INSTALL_COMMAND ${CMAKE_COMMAND} --build . --target install --config Release)

endfunction(add_deps_cmake_project)


if (MSVC)
    if ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
        message(STATUS "\nDetected 64-bit compiler => building 64-bit deps bundle\n")
        set(DEPS_BITS 64)
        include("deps-windows.cmake")
    elseif ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
        message(STATUS "\nDetected 32-bit compiler => building 32-bit deps bundle\n")
        set(DEPS_BITS 32)
        include("deps-windows.cmake")
    else ()
        message(FATAL_ERROR "Unable to detect architecture")
    endif ()
elseif (APPLE)
    message("OS X SDK Path: ${CMAKE_OSX_SYSROOT}")
    if (CMAKE_OSX_DEPLOYMENT_TARGET)
        set(DEP_OSX_TARGET "${CMAKE_OSX_DEPLOYMENT_TARGET}")
        message("OS X Deployment Target: ${DEP_OSX_TARGET}")
    else ()
        # Attempt to infer the SDK version from the CMAKE_OSX_SYSROOT,
        # this is done because wxWidgets need the min version explicitly set
        string(REGEX MATCH "[0-9]+[.][0-9]+[.]sdk$" DEP_OSX_TARGET "${CMAKE_OSX_SYSROOT}")
        string(REGEX MATCH "^[0-9]+[.][0-9]+" DEP_OSX_TARGET "${DEP_OSX_TARGET}")

        if (NOT DEP_OSX_TARGET)
            message(FATAL_ERROR "Could not determine OS X SDK version. Please use -DCMAKE_OSX_DEPLOYMENT_TARGET=<version>")
        endif ()

        message("OS X Deployment Target (inferred from SDK): ${DEP_OSX_TARGET}")
    endif ()

    include("deps-macos.cmake")
elseif (MINGW)
    message(STATUS "Building for MinGW...")
    include("deps-mingw.cmake")
else ()
    include("deps-linux.cmake")
endif ()

include(Boost/Boost.cmake)
include(EIGEN3/EIGEN3.cmake)
include(GMP/GMP.cmake)
include(MPFR/MPFR.cmake)
include(CGAL/CGAL.cmake)

set(dep_list Boost CGAL)

add_custom_target(deps ALL DEPENDS ${dep_list})