### Copyright (C) 2022 by Camden Mannett.  All rights reserved.

create_clangformat_target(
    NAME clangformat_example_just_cats
    FORMAT_FILE ${CMAKE_SOURCE_DIR}/.clang-format
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

add_executable(example_just_cats EXCLUDE_FROM_ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
add_dependencies(example_just_cats clangformat_example_just_cats arg_router)

target_compile_features(example_just_cats PUBLIC cxx_std_17)
set_target_properties(example_just_cats PROPERTIES CXX_EXTENSIONS OFF)

configure_test_build(example_basic_cat)

target_include_directories(example_just_cats
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)
