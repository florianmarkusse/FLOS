project(shared-memory-management LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/management.c" "src/page.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-to-kernel-i)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-jmp-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-efi-to-kernel-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-thread-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-interrupts-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-log-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-text-converter-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-virtual-i)

add_subdirectory(status)
