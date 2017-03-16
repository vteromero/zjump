cmake_minimum_required(VERSION 2.8.7)

project(Zjump C CXX)

set(CMAKE_CXX_FLAGS "-std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -Wall")

enable_testing()

add_subdirectory(third-party/gtest)
include_directories(${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS})

add_subdirectory(third-party/libdivsufsort)
include_directories(${LIBDIVSUFSORT_INCLUDE_DIRS})

add_subdirectory(src)

