project(x86-jmp LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/jmp.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-jmp-i)
