project(googletest-download NONE)

include(ExternalProject)

set(SOURCE_DIR "${CMAKE_BINARY_DIR}/googletest-src")
set(BINARY_DIR "${CMAKE_BINARY_DIR}/googletest-build")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

ExternalProject_Add(googletest
    GIT_REPOSITORY    https://github.com/google/googletest.git
    GIT_TAG           master
    CMAKE_ARGS
        -Dgtest_force_shared_crt=ON
        -Dgtest_disable_pthreads=OFF
        -DBUILD_GTEST=ON
        -DBUILD_GMOCK=ON
    SOURCE_DIR        ${SOURCE_DIR}
    BINARY_DIR        ${BINARY_DIR}
    INSTALL_COMMAND   ""
)

set(GTEST_INCLUDE_DIRS ${SOURCE_DIR}/googletest/include PARENT_SCOPE)
set(GMOCK_INCLUDE_DIRS ${SOURCE_DIR}/googlemock/include PARENT_SCOPE)
set(GMOCK_LIBS_DIR ${BINARY_DIR}/googlemock)

add_library(gmock_main STATIC IMPORTED GLOBAL)
set_target_properties(
    gmock_main
    PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES CXX
        IMPORTED_LOCATION ${GMOCK_LIBS_DIR}/libgmock_main.a
)

set(GTEST_LIBS gmock_main ${CMAKE_THREAD_LIBS_INIT} PARENT_SCOPE)

