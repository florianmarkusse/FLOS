cmake_minimum_required(VERSION 3.22)
project(image-builder LANGUAGES C ASM)

include("${REPO_PROJECTS}/setup.cmake")
add_project("shared")
add_project("posix")
add_project("efi/uefi")
add_project("abstraction/log")
add_project("abstraction/memory/manipulation")
add_project("abstraction/jmp")
add_project("abstraction/text/converter")
include("${REPO_PROJECTS}/print-configuration.cmake")

add_executable(
    ${PROJECT_NAME}
    "src/main.c"
    "src/mbr.c"
    "src/configuration.c"
    "src/crc32.c"
    "src/gpt.c"
    "src/partitions/efi.c"
    "src/partitions/data.c"
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-log)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-jmp)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-text-converter)

target_link_libraries(${PROJECT_NAME} PRIVATE posix-i)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-text)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-log)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-text)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-uuid)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-uefi-i)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
