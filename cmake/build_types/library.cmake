add_library(arg_router INTERFACE)
target_sources(arg_router INTERFACE ${HEADERS} ${FOR_IDE} ${VERSION_FILE})
add_dependencies(arg_router clangformat)

target_compile_features(arg_router INTERFACE cxx_std_17)

target_compile_options(arg_router INTERFACE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
        -Werror -Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4>
)

target_include_directories(arg_router
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(arg_router
    INTERFACE fmt::fmt
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router
        DESTINATION include
        COMPONENT dev)
