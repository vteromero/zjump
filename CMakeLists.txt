cmake_minimum_required(VERSION 3.1)

project(Zjump CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB SOURCES "src/*.cc")

add_executable(zjump ${SOURCES})

target_link_libraries(zjump divsufsort)

