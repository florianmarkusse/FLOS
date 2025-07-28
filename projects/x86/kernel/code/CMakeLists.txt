cmake_minimum_required(VERSION 3.22)
project(x86-kernel LANGUAGES C ASM)
set(AGGREGATED_INTERFACE "${PROJECT_NAME}-i")
add_library(${AGGREGATED_INTERFACE} INTERFACE)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("${REPO_PROJECTS}/setup.cmake")
    add_project("shared")
    add_project("x86")
    add_project("x86/efi-to-kernel")
    add_project("freestanding")
    add_project("abstraction/kernel")
    add_project("abstraction/log")
    add_project("abstraction/text/converter")
    add_project("abstraction/jmp")
    add_project("abstraction/interrupts")
    add_project("abstraction/memory/manipulation")
    add_project("abstraction/memory/virtual")
    add_project("abstraction/thread")
    add_project("efi-to-kernel")
    include("${REPO_PROJECTS}/print-configuration.cmake")
endif()

add_library(${PROJECT_NAME} OBJECT "src/kernel.c")

add_includes_for_sublibrary()

add_subdirectory(idt)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-jmp-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-kernel-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-interrupts-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-i)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-kernel-i)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-to-kernel-i)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-efi-to-kernel-i)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
