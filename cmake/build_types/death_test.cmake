set(DEATH_TEST_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/death_test/main.cpp
)

add_executable(arg_router_death_test EXCLUDE_FROM_ALL ${DEATH_TEST_SRCS})
add_dependencies(arg_router_death_test arg_router)

target_compile_features(arg_router_death_test PUBLIC cxx_std_17)
set_target_properties(arg_router_death_test PROPERTIES CXX_EXTENSIONS OFF)

target_compile_options(arg_router_death_test PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
        -Werror -Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4>
)

target_include_directories(arg_router_death_test
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(arg_router_death_test
    INTERFACE arg_router
)
