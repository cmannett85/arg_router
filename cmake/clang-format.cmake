### Copyright (C) 2022 by Camden Mannett.  All rights reserved. 

find_program(CLANG_FORMAT clang-format REQUIRED)
message(STATUS "Found clang-format at ${CLANG_FORMAT}")

# Creates a clang-format target
function(create_clangformat_target)
    cmake_parse_arguments(ARGS "" "NAME" "SOURCES" ${ARGN})

    add_custom_target(${ARGS_NAME}
        COMMAND "${CLANG_FORMAT}" --style=file --Werror -i ${ARGS_SOURCES}
        SOURCES "${CMAKE_SOURCE_DIR}/.clang-format" ${ARGS_SOURCES}
    )
endfunction()
