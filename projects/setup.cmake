set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -march=native -m64 -Wall -Wpedantic -Wextra -Wconversion -Wno-incompatible-pointer-types-discards-qualifiers -Wno-gnu-alignof-expression -Wno-gnu-statement-expression-from-macro-expansion -Wno-gnu-zero-variadic-macro-arguments -Wno-language-extension-token -Wno-gnu-pointer-arith -Wdouble-promotion -Wvla"
)
# In here because with default named struct arguments macros, you will get a
# warning when you override one of these values, which is the whole point of
# making them overridable.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-initializer-overrides")

option(FLOAT_OPERATIONS "Turn on/off floating-point operations" ON)
if(${FLOAT_OPERATIONS})
    add_compile_definitions(FLOAT_OPERATIONS)
else()
    add_compile_definitions(NO_FLOAT_OPERATIONS)
endif()

option(SERIAL "Turn on/off serial logging" OFF)
if(${SERIAL})
    add_compile_definitions(SERIAL)
else()
    add_compile_definitions(NO_SERIAL)
endif()

set(VALID_BUILDS "UNIT_TEST" "PROJECT")
list(FIND VALID_BUILDS ${BUILD} BUILD_INDEX)
if(BUILD_INDEX EQUAL -1)
    message(
        FATAL_ERROR
        "Invalid Build specified. Please choose one of: ${VALID_BUILD_TYPES}"
    )
endif()
if("${BUILD}" STREQUAL "PROJECT")
    add_compile_definitions(PROJECT)
endif()
if("${BUILD}" STREQUAL "UNIT_TEST")
    add_compile_definitions(UNIT_TEST)
endif()

set(VALID_ENVIRONMENTS "FREESTANDING" "POSIX" "EFI")
list(FIND VALID_ENVIRONMENTS ${ENVIRONMENT} VALID_ENVIRONMENT_INDEX)
if(VALID_ENVIRONMENT_INDEX EQUAL -1)
    message(
        FATAL_ERROR
        "Invalid environment specified. Please choose one of: ${VALID_ENVIRONMENTS}"
    )
endif()

if(${ENVIRONMENT} STREQUAL "FREESTANDING")
    add_compile_definitions(FREESTANDING)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -nostdlib -ffreestanding")
    add_link_options("--ld-path=${CMAKE_LINKER}")
endif()
if(${ENVIRONMENT} STREQUAL "EFI")
    add_compile_definitions(EFI)
    set(CMAKE_C_FLAGS
        "${CMAKE_C_FLAGS} -nostdinc -nostdlib -ffreestanding --target=x86_64-unknown-windows -mgeneral-regs-only -mno-stack-arg-probe"
    )
    get_filename_component(LINKER_FILENAME ${CMAKE_LINKER} NAME)
    add_link_options(
        -fuse-ld=${LINKER_FILENAME}
        -Wl,-entry:efi_main,-subsystem:efi_application
    )
endif()
if(${ENVIRONMENT} STREQUAL "POSIX")
    add_compile_definitions(POSIX)
    add_link_options("--ld-path=${CMAKE_LINKER}")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE
        "Release"
        CACHE STRING
        "Build type (Debug, Release, Profiling, Fuzzing)"
        FORCE
    )
endif()
set(VALID_BUILD_TYPES "DEBUG" "RELEASE" "PROFILING" "FUZZING")
list(FIND VALID_BUILD_TYPES ${CMAKE_BUILD_TYPE} VALID_BUILD_TYPE_INDEX)
if(VALID_BUILD_TYPE_INDEX EQUAL -1)
    message(
        FATAL_ERROR
        "Invalid build type specified. Please choose one of: ${VALID_BUILD_TYPES}"
    )
endif()
if(CMAKE_BUILD_TYPE STREQUAL "FUZZING" OR CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g3 -fno-omit-frame-pointer")
    if(${ENVIRONMENT} STREQUAL "POSIX")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address,undefined")
    endif()
    add_compile_definitions(DEBUG)
endif()
if(CMAKE_BUILD_TYPE STREQUAL "PROFILING")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg -O2 -pg")
    add_compile_definitions(PROFILING)
endif()
if(CMAKE_BUILD_TYPE STREQUAL "RELEASE")
    add_compile_definitions(RELEASE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -flto")
endif()

set(VALID_ARCHITECTURES "X86")
list(FIND VALID_ARCHITECTURES ${ARCHITECTURE} VALID_ARCHITECTURE_INDEX)
if(VALID_ARCHITECTURE_INDEX EQUAL -1)
    message(
        FATAL_ERROR
        "Invalid architecture specified. Please choose one of: ${VALID_ARCHITECTURES}"
    )
endif()
if(${ARCHITECTURE} STREQUAL "X86")
    add_compile_definitions(X86)
endif()

set(VALID_VENDORS "INTEL" "AMD")
list(FIND VALID_VENDORS ${VENDOR} VALID_VENDOR_INDEX)
if(VALID_VENDOR_INDEX EQUAL -1)
    message(
        FATAL_ERROR
        "Invalid VENDOR specified. Please choose one of: ${VALID_VENDORS}"
    )
endif()
if(${VENDOR} STREQUAL "INTEL")
    add_compile_definitions(VENDOR_INTEL)
endif()
if(${VENDOR} STREQUAL "AMD")
    add_compile_definitions(VENDOR_AMD)
endif()

set(VALID_UNDERLYINGS "QEMU" "HARDWARE")
list(FIND VALID_UNDERLYINGS ${UNDERLYING} VALID_UNDERLYING_INDEX)
if(VALID_UNDERLYING_INDEX EQUAL -1)
    message(
        FATAL_ERROR
        "Invalid UNDERLYING specified. Please choose one of: ${VALID_UNDERLYINGS}"
    )
endif()
if(${UNDERLYING} STREQUAL "QEMU")
    add_compile_definitions(UNDERLYING_QEMU)
endif()
if(${UNDERLYING} STREQUAL "HARDWARE")
    add_compile_definitions(UNDERLYING_HARDWARE)
endif()

option(AVX_512 "Turn on/off AVX512" ON)
if(${AVX_512})
    add_compile_definitions(AVX_512)
else()
    # NOTE: This does not actually turn off the cflags!!!! so it will just
    # mostly fail because we are not setting the state components in the IDT to
    # store all the avx512 components but still building with avx512 support
    # due to the c flags = BOOM
    add_compile_definitions(NO_AVX_512)
endif()

set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
# NOTE: embed-dir is not a supported asm flag
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --embed-dir=${REPO_PROJECTS}")

include("${REPO_PROJECTS}/project-target.cmake")
include("${REPO_PROJECTS}/add-project.cmake")
include("${REPO_PROJECTS}/abstraction.cmake")
include("${REPO_PROJECTS}/macros.cmake")
