### Copyright (C) 2022 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

function(create_ast_target)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        cmake_parse_arguments(ARG "" "TARGET" "SOURCES" ${ARGN})
        if (NOT ARG_TARGET)
            message(FATAL_ERROR "Target not defined")
        endif()
        if (NOT ARG_SOURCES)
            message(FATAL_ERROR "Sources not defined")
        endif()

        add_executable(${ARG_TARGET} EXCLUDE_FROM_ALL ${ARG_SOURCES})
        target_compile_features(${ARG_TARGET} PUBLIC cxx_std_17)
        set_target_properties(${ARG_TARGET} PROPERTIES CXX_EXTENSIONS OFF)
        target_include_directories(${ARG_TARGET} PUBLIC "${CMAKE_SOURCE_DIR}/include")
        target_compile_options(${ARG_TARGET} PRIVATE -Xclang -ast-print -fsyntax-only)
    else()
        message(STATUS "Skipping AST targets as they are only for Clang compilers")
    endif()
endfunction()
