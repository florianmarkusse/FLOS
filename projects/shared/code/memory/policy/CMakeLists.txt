project(shared-memory-policy LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/policy.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-to-kernel-i)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-jmp-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-log-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-text-converter-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-virtual-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)

add_subdirectory(status)
