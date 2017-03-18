cmake_minimum_required(VERSION 2.8.7)

project(Zjump C CXX)

set(CMAKE_CXX_FLAGS "-std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -Wall")

enable_testing()

add_subdirectory(third-party/gtest)
include_directories(${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS})
add_library(gmock_main STATIC IMPORTED GLOBAL)
set_target_properties(
    gmock_main
    PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES CXX
        IMPORTED_LOCATION ${GMOCK_LIBS_DIR}/libgmock_main.a
)

add_subdirectory(third-party/libdivsufsort)
include_directories(${LIBDIVSUFSORT_INCLUDE_DIRS})
add_library(divsufsort STATIC IMPORTED GLOBAL)
set_target_properties(
    divsufsort
    PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES CXX
        IMPORTED_LOCATION ${LIBDIVSUFSORT_LIBS_DIR}/libdivsufsort.a
)

add_subdirectory(src)

