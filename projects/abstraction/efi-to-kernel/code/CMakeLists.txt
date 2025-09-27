create_abstraction_targets("abstraction-efi-to-kernel")

if(${ARCHITECTURE} STREQUAL "X86")
    add_project("x86/efi-to-kernel")
    abstraction_include_interface_library(x86-efi-to-kernel-i)
else()
    message(FATAL_ERROR "Could not match ARCHITECTURE variable")
endif()
