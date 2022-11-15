### Copyright (C) 2022 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

include("${CMAKE_SOURCE_DIR}/cmake/build_types/ast.cmake")

message(STATUS "Configuring examples")
add_subdirectory(c++17)

if(BUILD_CPP20_EXAMPLES)
    add_subdirectory(c++20)
endif()
