include_directories(${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS})

file(GLOB SOURCES "*.cc")

add_executable(zjump_unittesting ${SOURCES})

target_link_libraries(
    zjump_unittesting

    zjump_lib
    ${GTEST_LIBS}
)

add_test(
    NAME zjump_unittesting
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/zjump_unittesting
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

