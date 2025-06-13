cmake_minimum_required(VERSION 3.22)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(mapping-test LANGUAGES C ASM)

include("${REPO_PROJECTS}/setup.cmake")
add_project("shared")
add_project("posix")
add_project("abstraction/memory/manipulation")
add_project("abstraction/log")
add_project("abstraction/jmp")
add_project("abstraction/text/converter")
add_project("abstraction/time")
add_project("abstraction/serial")
include("${REPO_PROJECTS}/print-configuration.cmake")

add_executable(${PROJECT_NAME} "src/main.c")

target_link_libraries(${PROJECT_NAME} PRIVATE posix-i)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-time)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-jmp)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-log)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-text-converter)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-text)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-prng)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-log)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-maths)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
