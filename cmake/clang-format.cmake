### Copyright (C) 2022 by Camden Mannett.  All rights reserved. 

find_program(CLANG_FORMAT clang-format REQUIRED)
message(STATUS "Found clang-format at ${CLANG_FORMAT}")

# Creates a clang-format target
function(create_clangformat_target)
    set(single_value_args NAME FORMAT_FILE)
    set(multi_value_args SOURCES DEPENDS)
    cmake_parse_arguments(ARGS "" "${single_value_args}" "${multi_value_args}" ${ARGN})

    add_custom_target(${ARGS_NAME}
        COMMAND "${CLANG_FORMAT}" --style=file --Werror -i ${ARGS_SOURCES}
        SOURCES ${ARGS_FORMAT_FILE} ${ARGS_SOURCES}
    )

    if(DEFINED ARGS_DEPENDS)
        add_dependencies(${ARGS_NAME} ${ARGS_DEPENDS})
    endif()
endfunction()
