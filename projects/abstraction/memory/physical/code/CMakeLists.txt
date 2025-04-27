create_abstraction_targets(abstraction-memory-physical)

if(${ENVIRONMENT} STREQUAL "EFI")
    add_project("efi")
    abstraction_add_sources(efi-memory)
elseif(
    ${ENVIRONMENT} STREQUAL "FREESTANDING"
    OR ${ENVIRONMENT} STREQUAL "POSIX"
)
    add_project("shared")
    abstraction_add_sources(shared-memory-physical)
    abstraction_add_sources(shared-memory-physical-status)
else()
    message(FATAL_ERROR "Could not find matching lib for ${PROJECT_NAME}")
endif()
