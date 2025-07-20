project(x86-thread LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/thread.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-thread-i)
