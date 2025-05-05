project(x86-time LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/time.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-time-i)
