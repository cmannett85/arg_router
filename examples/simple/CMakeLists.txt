### Copyright (C) 2022-2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

create_clangformat_target(
    NAME clangformat_example_simple
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)

add_executable(example_simple EXCLUDE_FROM_ALL "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
add_dependencies(example_simple clangformat_example_simple)

target_compile_features(example_simple PUBLIC cxx_std_20)
set_target_properties(example_simple PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(example_simple PRIVATE arg_router)

configure_example_build(example_simple)
add_clangtidy_to_target(example_simple)

add_dependencies(examples example_simple)
