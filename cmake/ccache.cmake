### Copyright (C) 2022 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

if (BUILD_UNIT_TESTS_AND_EXAMPLES)
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM AND NOT (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
        message(STATUS "Found ccache")
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    endif()
endif()