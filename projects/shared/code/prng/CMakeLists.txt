project(shared-prng LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/biski.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)
