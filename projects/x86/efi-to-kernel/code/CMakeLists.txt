cmake_minimum_required(VERSION 3.22)
project(x86-efi-to-kernel-i LANGUAGES C ASM)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include(${REPO_PROJECTS}/setup.cmake)
    add_project("x86")
    add_project("shared")
    add_project("abstraction/memory/manipulation")
    include(${REPO_PROJECTS}/print-configuration.cmake)
endif()

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(
    ${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} INTERFACE x86-i)
target_link_libraries(${PROJECT_NAME} INTERFACE shared-i)
target_link_libraries(
    ${PROJECT_NAME}
    INTERFACE abstraction-memory-manipulation-i
)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
