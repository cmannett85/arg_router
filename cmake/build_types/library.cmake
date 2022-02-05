### Copyright (C) 2022 by Camden Mannett.  All rights reserved. 

add_library(arg_router INTERFACE ${HEADERS} ${FOR_IDE} ${VERSION_FILE})
add_dependencies(arg_router clangformat)

target_compile_features(arg_router INTERFACE cxx_std_17)
set_target_properties(arg_router PROPERTIES CXX_EXTENSIONS OFF)

target_include_directories(arg_router
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router
        DESTINATION include
        COMPONENT dev)
