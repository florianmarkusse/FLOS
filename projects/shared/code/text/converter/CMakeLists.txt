project(shared-text-converter LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/base.c" "src/buffer.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-text-converter-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)

add_subdirectory(float)
