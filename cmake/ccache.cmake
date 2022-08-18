### Copyright (C) 2022 by Camden Mannett.  All rights reserved. 

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM AND NOT (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
    message(STATUS "Found ccache")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()
