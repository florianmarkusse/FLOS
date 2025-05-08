project(x86-memory-virtual LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/virtual.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-physical-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-jmp-i)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-i)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-to-kernel-i)

target_link_libraries(${PROJECT_NAME} PRIVATE freestanding-i)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)
