create_abstraction_targets(abstraction-memory-physical)

if(${ENVIRONMENT} STREQUAL "EFI")
    add_project("efi")
    abstraction_add_sources(efi-memory)
elseif(${ENVIRONMENT} STREQUAL "FREESTANDING")
    add_project("freestanding")
    abstraction_add_sources(freestanding-memory-virtual)
elseif(${ENVIRONMENT} STREQUAL "POSIX")
    # Should not need an implementation
else()
    message(FATAL_ERROR "Could not find matching lib for ${PROJECT_NAME}")
endif()
