### Copyright (C) 2022 by Camden Mannett.  All rights reserved. 

add_library(arg_router_coverage INTERFACE ${HEADERS} ${FOR_IDE} ${VERSION_FILE})
add_dependencies(arg_router_coverage clangformat)

target_compile_features(arg_router_coverage INTERFACE cxx_std_17)
set_target_properties(arg_router_coverage PROPERTIES CXX_EXTENSIONS OFF)

target_compile_options(arg_router_coverage
    INTERFACE -Werror -Wall -Wextra
    INTERFACE --coverage
)

target_include_directories(arg_router_coverage
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
