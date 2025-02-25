- send msg to iwyu folks?
  - How to only run "iwyu" on a project and not all its dependencies?
  - this is a cmake issue and not iwyu issue tho but is worth seeing if they support such a flag or have plans to
- Figure out what to do with memory allocation & mapping in os-loader
- Fix posix tests in physical memory
- Rethink memory allocation for kernel structures in uefi and whether or not to add them to free physical memory in kernel --- definitely some bugs now.
- if you ever decide to use unit tests in the kernel, need to create an abstraction for test-framework so it can be used in both posix and freestanding environments

CPU features to implement/turn on in x86

- fpu
- sse
- avx
- gpe

project(platform-abstraction-log LANGUAGES C ASM)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
${PROJECT_NAME}
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if("${ENVIRONMENT}" STREQUAL "freestanding")
    include_and_link_object_library(kernel-log)
elseif("${ENVIRONMENT}" STREQUAL "efi")
include_and_link_object_library(efi-log)
elseif("${ENVIRONMENT}" STREQUAL "posix")
include_and_link_object_library(posix-log)
else()
message(FATAL_ERROR "Could not match ENVIRONMENT variable")
endif()

target_include_directories(
${AGGREGATED_INTERFACE}
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
