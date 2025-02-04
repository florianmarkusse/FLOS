project(x86-memory LANGUAGES C ASM)

add_library(
    ${PROJECT_NAME}
    OBJECT
    "src/page.c"
    "src/physical.c"
    "src/virtual.c"
    "src/policy.c"
    "src/init.c"
)

if("${UNIT_TEST_BUILD}")
    add_subdirectory(tests)
endif()

add_subdirectory(definitions)
add_subdirectory(status)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-to-kernel-i)

target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-i)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-i)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_include_directories(
    ${AGGREGATED_INTERFACE}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
