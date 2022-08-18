### Copyright (C) 2022 by Camden Mannett.  All rights reserved.

add_library(arg_router INTERFACE)
target_sources(arg_router INTERFACE ${HEADERS} ${FOR_IDE})
add_dependencies(arg_router gen_version)

target_compile_features(arg_router INTERFACE cxx_std_17)

target_include_directories(arg_router
    INTERFACE "${CMAKE_SOURCE_DIR}/include"
)
