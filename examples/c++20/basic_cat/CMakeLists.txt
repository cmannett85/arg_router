### Copyright (C) 2022-2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

create_clangformat_target(
    NAME clangformat_example_basic_cat_cpp20
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)

add_executable(example_basic_cat_cpp20 "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
add_dependencies(example_basic_cat_cpp20 clangformat_example_basic_cat_cpp20)

target_compile_features(example_basic_cat_cpp20 PUBLIC cxx_std_20)
set_target_properties(example_basic_cat_cpp20 PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(example_basic_cat_cpp20 PRIVATE arg_router)

configure_test_build(example_basic_cat_cpp20)
add_clangtidy_to_target(example_basic_cat_cpp20)

add_dependencies(cpp20_examples example_basic_cat_cpp20)
