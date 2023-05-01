### Copyright (C) 2022-2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

create_clangformat_target(
    NAME clangformat_example_custom_policy_and_node
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)

add_executable(example_custom_policy_and_node "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
add_dependencies(example_custom_policy_and_node clangformat_example_custom_policy_and_node)

target_compile_features(example_custom_policy_and_node PUBLIC cxx_std_17)
set_target_properties(example_custom_policy_and_node PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(example_custom_policy_and_node PRIVATE arg_router)

configure_example_build(example_custom_policy_and_node)
add_clangtidy_to_target(example_custom_policy_and_node)

add_dependencies(cpp17_examples example_custom_policy_and_node)
