set(ADDED_PROJECT_TARGETS
    "${PROJECT_FOLDER}"
    CACHE INTERNAL
    "Used to ensure a module is only added once."
)

function(add_project project)
    if(NOT "${project}" IN_LIST ADDED_PROJECT_TARGETS)
        update_added_projects(${project})
        add_subdirectory(
            "${REPO_PROJECTS}/${project}/code"
            "${REPO_PROJECTS}/${project}/code/${BUILD_OUTPUT_PATH}"
        )
    endif()
endfunction()

function(update_added_projects target)
    if(NOT "${project}" IN_LIST ADDED_PROJECT_TARGETS)
        set(ADDED_PROJECT_TARGETS
            "${ADDED_PROJECT_TARGETS};${target}"
            CACHE INTERNAL
            "Used to ensure a module is only added once."
        )
    endif()
endfunction()
