project(shared-memory-page LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/page.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-to-kernel-i)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-jmp-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-virtual-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)
