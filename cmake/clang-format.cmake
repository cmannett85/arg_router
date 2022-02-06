### Copyright (C) 2022 by Camden Mannett.  All rights reserved. 

# Creates a clang-format target
function(create_clangformat_target)
    set(single_value_args NAME FORMAT_FILE)
    set(multi_value_args SOURCES DEPENDENCIES)
    cmake_parse_arguments(ARGS "" "${single_value_args}"
                          "${multi_value_args}" ${ARGN})

    add_custom_target(${ARGS_NAME}
        COMMAND clang-format --style=file --Werror -i ${ARGS_SOURCES}
        SOURCES ${ARGS_FORMAT_FILE} ${ARGS_SOURCES}
    )

    if(DEFINED ARGS_DEPENDENCIES)
        add_dependencies(${ARGS_NAME} ${ARGS_DEPENDENCIES})
    endif()
endfunction()