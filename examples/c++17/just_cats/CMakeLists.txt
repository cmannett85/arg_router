### Copyright (C) 2022-2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

create_clangformat_target(
    NAME clangformat_example_just_cats
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)

add_executable(example_just_cats "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
add_dependencies(example_just_cats clangformat_example_just_cats)

target_compile_features(example_just_cats PUBLIC cxx_std_17)
set_target_properties(example_just_cats PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(example_just_cats PRIVATE arg_router)

configure_test_build(example_just_cats)
add_clangtidy_to_target(example_just_cats)

add_dependencies(cpp17_examples example_just_cats)
