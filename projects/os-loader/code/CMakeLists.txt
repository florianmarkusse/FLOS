cmake_minimum_required(VERSION 3.22)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(os-loader LANGUAGES C ASM)

include("${REPO_PROJECTS}/setup.cmake")
add_project("shared")
add_project("efi")
add_project("efi/uefi")
add_project("efi-to-kernel")
add_project("abstraction/efi")
add_project("abstraction/memory/manipulation")
add_project("abstraction/memory/physical")
add_project("abstraction/memory/virtual")
add_project("abstraction/log")
add_project("abstraction/jmp")
add_project("abstraction/text/converter")
include("${REPO_PROJECTS}/print-configuration.cmake")

add_executable(${PROJECT_NAME} "src/main.c" "src/data-reading.c" "src/memory.c")

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-efi)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-jmp)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-physical)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-virtual)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-log)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-text-converter)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-text)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-maths)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-uuid)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-converter)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-allocator)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-i)
target_link_libraries(${PROJECT_NAME} PRIVATE efi-error)
target_link_libraries(${PROJECT_NAME} PRIVATE efi-acpi)
target_link_libraries(${PROJECT_NAME} PRIVATE efi-globals)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-uefi-i)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-to-kernel-i)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
