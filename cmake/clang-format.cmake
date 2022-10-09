### Copyright (C) 2022 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

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
