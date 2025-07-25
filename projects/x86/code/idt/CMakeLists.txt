project(x86-idt LANGUAGES C ASM)
add_library(
    ${PROJECT_NAME}
    OBJECT
    "src/trigger.c"
    "src/fault.c"
    "src/idt.c"
    "src/isr.S"
)

set_source_files_properties(
    "src/fault.c"
    PROPERTIES COMPILE_FLAGS "-mno-vzeroupper"
)

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-interrupts-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-log-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-text-converter-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-virtual-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-jmp-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-thread-i)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-to-kernel-i)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-i)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)
