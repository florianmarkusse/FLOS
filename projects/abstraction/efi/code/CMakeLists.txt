create_abstraction_targets(abstraction-efi)

add_project("shared")
add_project("efi-to-kernel")

if(${ARCHITECTURE} STREQUAL "X86")
    add_project("x86")
    abstraction_add_sources(x86-gdt)
    abstraction_add_sources(x86-configuration)
    add_project("x86/efi")
    abstraction_add_sources(x86-efi)
    add_project("x86/efi-to-kernel")
    abstraction_include_interface_library(x86-efi-to-kernel-i)
else()
    message(FATAL_ERROR "Could not match ARCHITECTURE variable")
endif()
