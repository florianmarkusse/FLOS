project(efi-firmware LANGUAGES C ASM)
add_library(${PROJECT_NAME} INTERFACE)

add_includes_for_sublibrary()

target_link_libraries(
    ${PROJECT_NAME}
    INTERFACE abstraction-memory-manipulation-i
)

target_link_libraries(${PROJECT_NAME} INTERFACE shared-i)

target_link_libraries(${PROJECT_NAME} INTERFACE efi-i)
target_link_libraries(${PROJECT_NAME} INTERFACE efi-uefi-i)
