project(shared-text LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/parser.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)

add_subdirectory(converter)
