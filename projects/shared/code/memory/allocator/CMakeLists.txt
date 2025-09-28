project(shared-memory-allocator LANGUAGES C ASM)
add_library(
    ${PROJECT_NAME}
    OBJECT
    "src/arena.c"
    "src/buddy.c"
    "src/node.c"
    # NOTE: Unused for now.
    # "src/pool.c"
)

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-virtual-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-jmp-i)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

add_subdirectory(status)

if(${BUILD} STREQUAL "UNIT_TEST")
    add_subdirectory(tests)
endif()
