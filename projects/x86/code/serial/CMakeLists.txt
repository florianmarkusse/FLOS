project(x86-serial LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/serial.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-serial-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-interrupts-i)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-i)
