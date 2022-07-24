### Copyright (C) 2022 by Camden Mannett.  All rights reserved.

create_clangformat_target(
    NAME clangformat_example_simple
    FORMAT_FILE ${CMAKE_SOURCE_DIR}/.clang-format
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

add_executable(example_simple EXCLUDE_FROM_ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
add_dependencies(example_simple clangformat_example_simple arg_router)

target_compile_features(example_simple PUBLIC cxx_std_17)
set_target_properties(example_simple PROPERTIES CXX_EXTENSIONS OFF)

configure_test_build(example_simple)
add_clangtidy_to_target(example_simple)

target_include_directories(example_simple
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)
