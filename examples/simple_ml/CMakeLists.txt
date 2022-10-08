### Copyright (C) 2022 by Camden Mannett.  All rights reserved.

create_clangformat_target(
    NAME clangformat_example_simple_ml
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)

add_executable(example_simple_ml "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
add_dependencies(example_simple_ml clangformat_example_simple_ml arg_router)

target_compile_features(example_simple_ml PUBLIC cxx_std_17)
set_target_properties(example_simple_ml PROPERTIES CXX_EXTENSIONS OFF)

configure_test_build(example_simple_ml)
add_clangtidy_to_target(example_simple_ml)

target_include_directories(example_simple_ml
    PUBLIC "${CMAKE_SOURCE_DIR}/include"
)
