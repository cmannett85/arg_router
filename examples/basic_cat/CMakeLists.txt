### Copyright (C) 2022 by Camden Mannett.  All rights reserved.

create_clangformat_target(
    NAME clangformat_example_basic_cat
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)

add_executable(example_basic_cat "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
add_dependencies(example_basic_cat clangformat_example_basic_cat arg_router)

target_compile_features(example_basic_cat PUBLIC cxx_std_17)
set_target_properties(example_basic_cat PROPERTIES CXX_EXTENSIONS OFF)

configure_test_build(example_basic_cat)
add_clangtidy_to_target(example_basic_cat)

target_include_directories(example_basic_cat
    PUBLIC "${CMAKE_SOURCE_DIR}/include"
)

create_ast_target(TARGET example_basic_cat_ast
                  SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
