function(get_project_targets result currentDir)
    get_property(
        subdirectories
        DIRECTORY "${currentDir}"
        PROPERTY SUBDIRECTORIES
    )
    foreach(subdirectory IN LISTS subdirectories)
        get_project_targets(${result} "${subdirectory}")
    endforeach()
    get_directory_property(
        all_targets
        DIRECTORY "${currentDir}"
        BUILDSYSTEM_TARGETS
    )
    set(buildable_targets)
    foreach(target IN LISTS all_targets)
        get_property(target_type TARGET ${target} PROPERTY TYPE)
        if(NOT target_type STREQUAL "INTERFACE_LIBRARY")
            list(APPEND buildable_targets ${target})
        endif()
    endforeach()
    list(FILTER buildable_targets INCLUDE REGEX "^${PROJECT_NAME}.*")
    set(${result} ${${result}} ${buildable_targets} PARENT_SCOPE)
endfunction()

function(fetch_and_write_project_targets)
    set(project_targets)
    get_project_targets(project_targets ${CMAKE_CURRENT_BINARY_DIR})

    file(WRITE ${PROJECT_TARGETS_FILE} "")
    foreach(target ${project_targets})
        file(APPEND ${PROJECT_TARGETS_FILE} "${target}\n")
    endforeach()
endfunction()
