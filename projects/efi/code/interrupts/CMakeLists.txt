project(efi-interrupts LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/interrupts.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE efi-i)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-interrupts-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-log-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-text-converter-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)
