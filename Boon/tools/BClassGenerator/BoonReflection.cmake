include(CMakeParseArguments)

function(boon_add_reflected_module)
    set(options VERBOSE MINIMAL)
    set(oneValueArgs NAME OUTPUT)
    set(multiValueArgs SCAN_DIRS DEPENDS)

    cmake_parse_arguments(REF "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT REF_NAME)
        message(FATAL_ERROR "boon_add_reflected_module: NAME is required")
    endif()

    if (NOT REF_OUTPUT)
        message(FATAL_ERROR "boon_add_reflected_module: OUTPUT is required")
    endif()

    if (NOT REF_SCAN_DIRS)
        message(FATAL_ERROR "boon_add_reflected_module: SCAN_DIRS is required")
    endif()

    set(REFLECT_ARGS)

    if (REF_VERBOSE)
        list(APPEND REFLECT_ARGS --verbose)
    endif()

    if (REF_MINIMAL)
        list(APPEND REFLECT_ARGS --minimal)
    endif()

    get_filename_component(REF_OUTPUT_DIR "${REF_OUTPUT}" DIRECTORY)
    file(MAKE_DIRECTORY "${REF_OUTPUT_DIR}")

    add_custom_command(
        OUTPUT "${REF_OUTPUT}"
        DEPENDS BClassGenerator ${REF_DEPENDS}
        COMMAND $<TARGET_FILE:BClassGenerator>
            ${REFLECT_ARGS}
            --module "${REF_NAME}"
            "${REF_OUTPUT}"
            ${REF_SCAN_DIRS}
        COMMENT "Generating reflection for ${REF_NAME}"
        VERBATIM
    )

    add_custom_target(GenerateReflection_${REF_NAME}
        DEPENDS "${REF_OUTPUT}"
    )

    add_dependencies(GenerateReflection GenerateReflection_${REF_NAME})

    set(BOON_REFLECTION_${REF_NAME}_OUTPUT "${REF_OUTPUT}" PARENT_SCOPE)
endfunction()