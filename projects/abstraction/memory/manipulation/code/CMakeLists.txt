create_abstraction_targets(abstraction-memory-manipulation)

if("${ENVIRONMENT}" STREQUAL "freestanding")
    add_project("freestanding")
    abstraction_add_sources(freestanding-memory-manipulation)
    abstraction_include_interface_library(freestanding-memory-manipulation)
    set(ABSTRACTION_MEMORY_MANIPULATION_INCLUDE
        "freestanding/memory/manipulation.h"
    )
elseif("${ENVIRONMENT}" STREQUAL "efi")
    add_project("freestanding")
    abstraction_add_sources(freestanding-memory-manipulation)
    abstraction_include_interface_library(freestanding-memory-manipulation)
    set(ABSTRACTION_MEMORY_MANIPULATION_INCLUDE
        "freestanding/memory/manipulation.h"
    )
elseif("${ENVIRONMENT}" STREQUAL "posix")
    abstraction_include_interface_library(posix-memory-manipulation)
    set(ABSTRACTION_MEMORY_MANIPULATION_INCLUDE "posix/memory/manipulation.h")
    # Posix should link with a stdlib that contains the right implementations
else()
    message(FATAL_ERROR "no memory manipulation implementation defined")
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/manipulation.h.in
    "include/abstraction/memory/manipulation.h"
    @ONLY
)

abstraction_include_interface_location("${CMAKE_CURRENT_BINARY_DIR}/include")
