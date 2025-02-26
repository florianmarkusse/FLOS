project(x86-configuration LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/features.c" "src/cpu.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-i)
