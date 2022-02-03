### Copyright (C) 2022 by Camden Mannett.  All rights reserved. 

option(BUILD_SHARED_LIBS "Build shared arg_router library")

add_library(arg_router ${HEADERS} ${SRCS} ${FOR_IDE} ${VERSION_FILE})
add_dependencies(arg_router clangformat)

target_compile_features(arg_router PUBLIC cxx_std_17)
set_target_properties(arg_router PROPERTIES CXX_EXTENSIONS OFF)

target_compile_options(arg_router PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
        -Werror -Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4>
)

target_include_directories(arg_router
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(LIB_COMP dev)
if(BUILD_SHARED_LIBS)
    set(LIB_COMP exe)
endif()

install(TARGETS arg_router
        COMPONENT ${LIB_COMP})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router
        DESTINATION include
        COMPONENT dev)
