project(x86-idt LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/idt.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-interrupts-i)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)
