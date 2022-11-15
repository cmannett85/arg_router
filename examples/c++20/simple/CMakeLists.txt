### Copyright (C) 2022 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

create_clangformat_target(
    NAME clangformat_example_simple_cpp20
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)

add_executable(example_simple_cpp20 "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
add_dependencies(example_simple_cpp20 clangformat_example_simple_cpp20 arg_router)

target_compile_features(example_simple_cpp20 PUBLIC cxx_std_20)
set_target_properties(example_simple_cpp20 PROPERTIES CXX_EXTENSIONS OFF)

configure_test_build(example_simple_cpp20)
add_clangtidy_to_target(example_simple_cpp20)

target_include_directories(example_simple_cpp20
    PUBLIC "${CMAKE_SOURCE_DIR}/include"
)

add_dependencies(cpp20_examples example_simple_cpp20)