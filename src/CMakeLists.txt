file(GLOB SOURCES "*.cc")
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/zjump.cc")

add_library(zjump_lib ${SOURCES})
target_link_libraries(zjump_lib ${LIBDIVSUFSORT_LIBS})

add_executable(zjump "zjump.cc")
target_link_libraries(zjump zjump_lib)
set_target_properties(
    zjump
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

#add_subdirectory(tests)

