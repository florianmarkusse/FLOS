project(freestanding-memory-virtual LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/allocator.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-virtual-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-to-kernel-i)
