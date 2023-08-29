### Copyright (C) 2022-2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

add_library(arg_router INTERFACE ${HEADERS})
target_link_libraries(arg_router INTERFACE
    Boost::boost
)

target_include_directories(arg_router AFTER INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>  # <prefix>/include
)

if(NOT INSTALLATION_ONLY)
    add_dependencies(arg_router clangformat)
endif()
