### Copyright (C) 2022 by Camden Mannett.  All rights reserved. 

add_library(arg_router_coverage ${HEADERS} ${SRCS} ${FOR_IDE} ${VERSION_FILE})
add_dependencies(arg_router_coverage clangformat)

target_compile_features(arg_router_coverage PUBLIC cxx_std_17)
set_target_properties(arg_router_coverage PROPERTIES CXX_EXTENSIONS OFF)

target_compile_options(arg_router_coverage
    PRIVATE -Werror -Wall -Wextra
    PRIVATE --coverage
)

target_include_directories(arg_router_coverage
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_options(arg_router_coverage
    PRIVATE --coverage
)
