create_abstraction_targets(abstraction-thread)

if(${ARCHITECTURE} STREQUAL "X86")
    add_project("x86")
    abstraction_add_sources(x86-thread)
else()
    message(FATAL_ERROR "Could not match ARCHITECTURE variable")
endif()
