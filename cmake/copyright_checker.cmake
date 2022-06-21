### Copyright (C) 2022 by Camden Mannett.  All rights reserved.

find_package (Python3 REQUIRED COMPONENTS Interpreter)

# Creates a copyright_checker target
function(create_copyright_checker_target)
    set(single_value_args NAME SCRIPT)
    set(multi_value_args SOURCES DEPENDENCIES)
    cmake_parse_arguments(ARGS "" "${single_value_args}" "${multi_value_args}" ${ARGN})

    add_custom_target(${ARGS_NAME}
        COMMAND ${Python3_EXECUTABLE} ${ARGS_SCRIPT} presence ${CMAKE_CURRENT_SOURCE_DIR}
        SOURCES ${ARGS_SOURCES}
    )

    if(DEFINED ARGS_DEPENDENCIES)
        add_dependencies(${ARGS_NAME} ${ARGS_DEPENDENCIES})
    endif()
endfunction()
