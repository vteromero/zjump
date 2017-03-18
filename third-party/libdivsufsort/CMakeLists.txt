project(libdivsufsort-download NONE)

include(ExternalProject)

set(SOURCE_DIR "${CMAKE_BINARY_DIR}/libdivsufsort-src")
set(BINARY_DIR "${CMAKE_BINARY_DIR}/libdivsufsort-build")

ExternalProject_Add(libdivsufsort
    GIT_REPOSITORY    https://github.com/y-256/libdivsufsort.git
    GIT_TAG           master
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=RELEASE
        -DBUILD_SHARED_LIBS=OFF
    SOURCE_DIR        ${SOURCE_DIR}
    BINARY_DIR        ${BINARY_DIR}
    INSTALL_COMMAND   ""
)

set(LIBDIVSUFSORT_INCLUDE_DIRS ${BINARY_DIR}/include PARENT_SCOPE)
set(LIBDIVSUFSORT_LIBS_DIR ${BINARY_DIR}/lib PARENT_SCOPE)
set(LIBDIVSUFSORT_LIBS divsufsort PARENT_SCOPE)

