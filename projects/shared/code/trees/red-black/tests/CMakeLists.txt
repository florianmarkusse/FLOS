project(shared-trees-red-black-tests LANGUAGES C)
add_executable(
    ${PROJECT_NAME}
    "src/main.c"
    "src/red-black/basic.c"
    "src/red-black/memory-manager.c"
    "src/assert.c"
    "src/assert-basic.c"
    "src/assert-memory-manager.c"
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-log)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-jmp)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-log)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-text-converter)

target_link_libraries(${PROJECT_NAME} PRIVATE posix-i)
target_link_libraries(${PROJECT_NAME} PRIVATE posix-test-framework)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-text)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-trees-red-black)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-allocator)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-management)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-management-status)
