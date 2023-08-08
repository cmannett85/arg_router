### Copyright (C) 2022-2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

create_clangformat_target(
    NAME clangformat_example_simple_ml
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)

add_executable(example_simple_ml "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
add_dependencies(example_simple_ml clangformat_example_simple_ml)

target_compile_features(example_simple_ml PUBLIC cxx_std_17)
set_target_properties(example_simple_ml PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(example_simple_ml PRIVATE arg_router)

configure_example_build(example_simple_ml)
add_clangtidy_to_target(example_simple_ml)

add_dependencies(cpp17_examples example_simple_ml)
