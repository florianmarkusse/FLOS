project(x86-idt LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/idt.c")

add_includes_for_sublibrary()

add_header_for_target("abstraction/interrupts")

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)
