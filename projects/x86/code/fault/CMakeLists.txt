project(x86-fault LANGUAGES C ASM)
add_library(${PROJECT_NAME} INTERFACE)

target_link_libraries(${PROJECT_NAME} INTERFACE efi-to-kernel-i)

add_includes_for_sublibrary()
