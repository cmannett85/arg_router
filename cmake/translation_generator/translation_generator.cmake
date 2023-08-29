### Copyright (C) 2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

function(arg_router_translation_generator)
    set(single_value_args OUTPUT_DIR TARGET)
    set(multi_value_args SOURCES)
    cmake_parse_arguments(ARGS "" "${single_value_args}" "${multi_value_args}" ${ARGN})

    if(NOT DEFINED ARGS_SOURCES)
        message(FATAL_ERROR "Translation generator requires at least one source file")
    endif()
    if(NOT DEFINED ARGS_OUTPUT_DIR)
        message(FATAL_ERROR "Translation generator output directory must be set, typically CMAKE_CURRENT_BINARY_DIR")
    endif()
    if(NOT DEFINED ARGS_TARGET)
        message(FATAL_ERROR "Translation generator requires a target output variable")
    endif()

    # The script file is always in the dir as this file
    get_filename_component(this_file_path "${CMAKE_CURRENT_FUNCTION_LIST_FILE}" DIRECTORY)
    set(script_path "${this_file_path}/translation_generator_script.cmake")

    add_custom_target(${ARGS_TARGET}
        COMMAND ${CMAKE_COMMAND}
                "-DSOURCES=${ARGS_SOURCES}"
                -DOUTPUT_DIR=${ARGS_OUTPUT_DIR}
                -P ${script_path}
        SOURCES ${ARGS_SOURCES}
                ${script_path}
        VERBATIM)

    # set_source_files_properties cannot be used in a script so that's set here
    foreach(source ${ARGS_SOURCES})
        get_filename_component(language_id "${source}" NAME_WLE)
        set(output_file "${ARGS_OUTPUT_DIR}/${language_id}.hpp")

        target_sources(${ARGS_TARGET} PRIVATE "${output_file}")
        set_source_files_properties("${output_file}" PROPERTIES GENERATED TRUE)
    endforeach()
endfunction()
