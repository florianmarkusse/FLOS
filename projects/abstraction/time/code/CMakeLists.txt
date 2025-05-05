create_abstraction_targets(abstraction-time)

if(${ARCHITECTURE} STREQUAL "X86")
    add_project("x86")
    abstraction_add_sources(x86-time)
else()
    message(FATAL_ERROR "Could not match ARCHITECTURE variable")
endif()
