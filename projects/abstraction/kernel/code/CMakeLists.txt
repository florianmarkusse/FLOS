create_abstraction_targets(abstraction-kernel)

add_project("efi-to-kernel")
abstraction_include_interface_library(efi-to-kernel-i)

if(${ARCHITECTURE} STREQUAL "X86")
    add_project("x86/kernel")
    abstraction_add_sources(x86-kernel)
else()
    message(FATAL_ERROR "Could not match ARCHITECTURE variable")
endif()
