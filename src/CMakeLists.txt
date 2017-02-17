file(GLOB SOURCES "*.cc")

add_executable(zjump ${SOURCES})

target_link_libraries(zjump divsufsort)

set_target_properties(
    zjump
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

