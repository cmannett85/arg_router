### Copyright (C) 2022-2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

set(LIBRARY_TYPE STATIC)
if (NOT BUILD_STATIC)
    set(LIBRARY_TYPE SHARED)
endif()

add_library(arg_router ${LIBRARY_TYPE} ${HEADERS} ${SOURCES})

set_target_properties(arg_router PROPERTIES
    CXX_EXTENSIONS OFF
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<NOT:$<BOOL:${BUILD_STATIC}>>:DLL>")
target_compile_features(arg_router PUBLIC cxx_std_20)

target_link_libraries(arg_router PUBLIC
    Boost::boost
)

target_include_directories(arg_router AFTER PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>  # <prefix>/include
)

add_dependencies(arg_router clangformat)
