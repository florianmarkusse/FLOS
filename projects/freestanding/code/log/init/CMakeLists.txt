project(freestanding-log-init LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/init.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-jmp-i)
