project(posix-file LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/path.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE posix-i)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-text-converter-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-jmp-i)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)
