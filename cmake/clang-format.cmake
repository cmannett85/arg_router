### Copyright (C) 2022 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

if(ENABLE_CLANG_FORMAT_CHECKS)
    find_program(CLANG_FORMAT clang-format REQUIRED)
    message(STATUS "Found clang-format at ${CLANG_FORMAT}")
endif()

# Creates a clang-format target, or an empty target if ENABLE_CLANG_FORMAT_CHECKS is off.  This
# target will fail the build if the formatting is incorrect
function(create_clangformat_target)
    cmake_parse_arguments(ARGS "" "NAME" "SOURCES" ${ARGN})

    if(ENABLE_CLANG_FORMAT_CHECKS)
        add_custom_target(${ARGS_NAME}
            COMMENT "Running clang-format checks for ${ARGS_NAME}"
            COMMAND "${CLANG_FORMAT}" --style=file --dry-run --Werror ${ARGS_SOURCES}
            SOURCES "${CMAKE_SOURCE_DIR}/.clang-format" ${ARGS_SOURCES}
        )
    else()
        add_custom_target(${ARGS_NAME})
    endif()
endfunction()
