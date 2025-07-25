cmake_minimum_required(VERSION 3.22)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(kernel LANGUAGES C ASM)
set(AGGREGATED_INTERFACE "${PROJECT_NAME}-i")
add_library(${AGGREGATED_INTERFACE} INTERFACE)

# TODO: Afer integration red zone, we can remove this flag here :)
set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -fno-pic -fno-pie -mcmodel=kernel -m64 -fno-stack-protector -mno-red-zone"
)
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")

include("${REPO_PROJECTS}/setup.cmake")

add_project("shared")
add_project("efi-to-kernel")
add_project("freestanding")
add_project("abstraction/jmp")
add_project("abstraction/efi")
add_project("abstraction/log")
add_project("abstraction/text/converter")
add_project("abstraction/interrupts")
add_project("abstraction/time")
add_project("abstraction/thread")
add_project("abstraction/serial")
add_project("abstraction/kernel")
add_project("abstraction/memory/manipulation")
add_project("abstraction/memory/virtual")

include("${REPO_PROJECTS}/print-configuration.cmake")

add_executable(${PROJECT_NAME} "src/main.c")

if(${ARCHITECTURE} STREQUAL "X86")
    set(EMULATION_TARGET "elf_x86_64")
else()
    message(FATAL_ERROR "No emulation target set to link")
endif()
set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        LINK_FLAGS
            "-T ${CMAKE_CURRENT_SOURCE_DIR}/main.ld -no-pie -Wl,-n,-m,${EMULATION_TARGET},--build-id=none"
        EXCLUDE_FROM_ALL TRUE
)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-to-kernel-i)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-virtual-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-efi-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-interrupts)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-kernel)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-time)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-thread)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-jmp)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-log)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-text-converter)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-serial)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-virtual)

target_link_libraries(${PROJECT_NAME} PRIVATE freestanding-i)
target_link_libraries(${PROJECT_NAME} PRIVATE freestanding-log-init)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-management)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-management-status)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-policy)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-policy-status)
target_link_libraries(${PROJECT_NAME} PRIVATE freestanding-peripheral-screen)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-text)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-maths)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-prng)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-log)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-trees-red-black)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-allocator)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-converter)

set(BINARIZER "${PROJECT_NAME}.bin")
set(PROJECT_NAME_BIN "${PROJECT_NAME}.bin")
set(KERNEL_MAGIC_FOLDER
    "${REPO_PROJECTS}/efi-to-kernel/code/include/efi-to-kernel/generated"
)
set(KERNEL_MAGIC_OUTPUT "${KERNEL_MAGIC_FOLDER}/kernel-magic.h")
add_custom_target(
    ${BINARIZER}
    ALL
    COMMAND
        objcopy -O binary --strip-all --remove-section=.interp
        --remove-section=.symtab --remove-section=.shstrtab
        --remove-section=.strtab ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_BIN}
    COMMAND
        # TODO: remove this once moved on to better UEFI support, just reading it by GUID instead.
        mkdir -p ${KERNEL_MAGIC_FOLDER} && python3
        "${REPO_PROJECTS}/efi-to-kernel/code/create_magic.py"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_BIN}" ${KERNEL_MAGIC_OUTPUT}
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
    COMMAND_EXPAND_LISTS
    BYPRODUCTS ${KERNEL_MAGIC_OUTPUT}
    VERBATIM
    COMMENT "Transforming ${PROJECT_NAME} into ${PROJECT_NAME_BIN}"
)
add_dependencies(${BINARIZER} ${PROJECT_NAME})

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
