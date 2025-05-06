project(freestanding-x86-kernel LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/kernel.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-interrupts-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-kernel-i)
target_link_libraries(${PROJECT_NAME} PRIVATE x86-i)
target_link_libraries(${PROJECT_NAME} PRIVATE x86-memory-i)
