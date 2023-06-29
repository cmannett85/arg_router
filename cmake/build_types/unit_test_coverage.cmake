### Copyright (C) 2022-2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

# Translation generator unit test input files
include("${CMAKE_SOURCE_DIR}/cmake/translation_generator/translation_generator.cmake")
arg_router_translation_generator(
    SOURCES "${CMAKE_SOURCE_DIR}/examples/resources/simple_ml_gen/en_GB.toml"
            "${CMAKE_SOURCE_DIR}/examples/resources/simple_ml_gen/ja.toml"
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/translations/"
    TARGET translation_arg_router_test_coverage
    INTERNAL
)

add_executable(arg_router_test_coverage EXCLUDE_FROM_ALL ${TEST_HEADERS} ${TEST_SRCS})
add_dependencies(
    arg_router_test_coverage
    translation_arg_router_test_coverage
    clangformat_test arg_router)

set_target_properties(arg_router_test_coverage PROPERTIES CXX_EXTENSIONS OFF)
target_compile_definitions(arg_router_test_coverage PRIVATE
    UNIT_TEST_BUILD
    AR_REPO_PATH="${CMAKE_SOURCE_DIR}"
    UNIT_TEST_BIN_DIR="${CMAKE_CURRENT_BINARY_DIR}"
)

# Default to C++20
if(NOT DEFINED CMAKE_CXX_STANDARD)
    target_compile_features(arg_router_test_coverage PUBLIC cxx_std_20)
endif()

configure_test_build(arg_router_test_coverage --coverage)
add_clangtidy_to_target(arg_router_test_coverage)
add_santizers_to_target(arg_router_test_coverage)

target_include_directories(arg_router_test_coverage
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(arg_router_test_coverage
    PRIVATE arg_router
    PUBLIC Boost::filesystem
    PUBLIC Threads::Threads
)

target_link_options(arg_router_test_coverage
    PRIVATE --coverage
)
