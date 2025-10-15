project(shared-trees-red-black LANGUAGES C ASM)
add_library(
    ${PROJECT_NAME}
    OBJECT
    "src/red-black/common.c"
    "src/red-black/basic.c"
    "src/red-black/virtual-mapping-manager.c"
)

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-jmp-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-virtual-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)

if(${BUILD} STREQUAL "UNIT_TEST")
    add_subdirectory(tests)
endif()
