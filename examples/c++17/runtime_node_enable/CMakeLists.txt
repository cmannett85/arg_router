### Copyright (C) 2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

create_clangformat_target(
    NAME clangformat_example_runtime_node_enable
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)

add_executable(example_runtime_node_enable "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
add_dependencies(example_runtime_node_enable clangformat_example_runtime_node_enable)

target_compile_features(example_runtime_node_enable PUBLIC cxx_std_17)
set_target_properties(example_runtime_node_enable PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(example_runtime_node_enable
    PRIVATE arg_router
)

configure_example_build(example_runtime_node_enable)
add_clangtidy_to_target(example_runtime_node_enable)

add_dependencies(cpp17_examples example_runtime_node_enable)
