project(libdivsufsort-download NONE)

include(ExternalProject)

set(SOURCE_DIR "${CMAKE_BINARY_DIR}/libdivsufsort-src")
set(BINARY_DIR "${CMAKE_BINARY_DIR}/libdivsufsort-build")

ExternalProject_Add(libdivsufsort
    GIT_REPOSITORY    https://github.com/y-256/libdivsufsort.git
    GIT_TAG           master
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=RELEASE
    SOURCE_DIR        ${SOURCE_DIR}
    BINARY_DIR        ${BINARY_DIR}
    INSTALL_COMMAND   ""
)

set(LIBDIVSUFSORT_INCLUDE_DIRS ${BINARY_DIR}/include PARENT_SCOPE)
set(LIBDIVSUFSORT_LIBS_DIR ${BINARY_DIR}/lib)

add_library(divsufsort SHARED IMPORTED GLOBAL)
set_target_properties(
    divsufsort
    PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES CXX
        IMPORTED_LOCATION ${LIBDIVSUFSORT_LIBS_DIR}/libdivsufsort.so
)

set(LIBDIVSUFSORT_LIBS divsufsort PARENT_SCOPE)

