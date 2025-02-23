create_abstraction_targets(abstraction-memory-virtual)

if("${ARCHITECTURE}" STREQUAL "x86")
    add_project("x86")
    add_project("x86/memory/virtual")
    abstraction_add_sources(x86-memory-virtual)
    abstraction_include_interface_library(x86-memory)
    abstraction_add_sources(x86-memory)
    set(ABSTRACTION_MEMORY_VIRTUAL_INCLUDE "x86/memory/definitions.h")
else()
    message(FATAL_ERROR "Could not find matching lib for ${PROJECT_NAME}")
endif()

set(INCLUDE_LOCATION "include/abstraction/memory/virtual")
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${INCLUDE_LOCATION}/converter.in.h
    "${INCLUDE_LOCATION}/converter.h"
    @ONLY
)

abstraction_include_interface_location("${CMAKE_CURRENT_BINARY_DIR}/include")
