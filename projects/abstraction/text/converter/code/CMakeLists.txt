create_abstraction_targets(abstraction-text-converter)

add_project("shared")

abstraction_add_sources(shared-text-converter)

if(${FLOAT_OPERATIONS})
    abstraction_add_sources(shared-text-converter-float)
else()
endif()
