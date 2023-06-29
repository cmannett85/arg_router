### Copyright (C) 2022-2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

set(Boost_USE_MULTITHREADED ON)
find_package(Boost ${BOOST_VERSION} REQUIRED COMPONENTS
    filesystem
)

find_package(Threads REQUIRED)

path_prefixer(TEST_HEADERS
    test_printers.hpp
    test_helpers.hpp
)

path_prefixer(TEST_SRCS
    main_test.cpp
    algorithm_test.cpp
    allocator_test.cpp
    arg_test.cpp
    counting_flag_test.cpp
    counting_flag_same_prefix_test.cpp
    dependency/alias_group_test.cpp
    dependency/one_of_test.cpp
    flag_same_prefix_test.cpp
    flag_test.cpp
    forwarding_arg_test.cpp
    help_test.cpp
    list_test.cpp
    math_test.cpp
    mode_test.cpp
    multi_arg_test.cpp
    multi_lang/iso_locale_test.cpp
    multi_lang/root_test.cpp
    multi_lang/root_wrapper_test.cpp
    multi_lang/string_selector_test.cpp
    parsing/dynamic_token_adapter_test.cpp
    parsing/global_parser_test.cpp
    parsing/parse_target_test.cpp
    parsing/parsing_test.cpp
    parsing/pre_parse_data_test.cpp
    policy/alias_test.cpp
    policy/colour_help_formatter_test.cpp
    policy/custom_parser_test.cpp
    policy/default_help_formatter_test.cpp
    policy/default_value_test.cpp
    policy/dependent_test.cpp
    policy/description_test.cpp
    policy/display_name_test.cpp
    policy/error_name_test.cpp
    policy/exception_translator_test.cpp
    policy/long_name_test.cpp
    policy/min_max_count_test.cpp
    policy/min_max_value_ct_test.cpp
    policy/min_max_value_t_test.cpp
    policy/required_test.cpp
    policy/router_test.cpp
    policy/runtime_enable_test.cpp
    policy/short_form_expander_test.cpp
    policy/short_name_test.cpp
    policy/token_end_marker_test.cpp
    policy/validator_rule_utilities_test.cpp
    policy/validator_test.cpp
    policy/value_separator_test.cpp
    positional_arg_test.cpp
    root_tests/death_test.cpp
    root_tests/positional_arg_test.cpp
    root_tests/top_level_test.cpp
    root_test.cpp
    test_helpers.cpp
    traits_test.cpp
    translation_generator_test.cpp
    tree_node_test.cpp
    utility/compile_time_string_test.cpp
    utility/compile_time_optional_test.cpp
    utility/dynamic_string_view_test.cpp
    utility/result_test.cpp
    utility/string_to_policy_test.cpp
    utility/string_view_ops_test.cpp
    utility/tree_recursor_test.cpp
    utility/type_hash_test.cpp
    utility/unsafe_any_test.cpp
    utility/utf8/code_point_test.cpp
    utility/utf8/grapheme_cluster_break_test.cpp
    utility/utf8/line_break_test.cpp
    utility/utf8_test.cpp
)

# Format just the unit test files
create_clangformat_target(
    NAME clangformat_test
    SOURCES ${TEST_HEADERS} ${TEST_SRCS}
)

# Translation generator unit test input files
include("${CMAKE_SOURCE_DIR}/cmake/translation_generator/translation_generator.cmake")
arg_router_translation_generator(
    SOURCES "${CMAKE_SOURCE_DIR}/examples/resources/simple_ml_gen/en_GB.toml"
            "${CMAKE_SOURCE_DIR}/examples/resources/simple_ml_gen/ja.toml"
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/translations/"
    TARGET translation_arg_router_test
)

add_executable(arg_router_test ${TEST_HEADERS} ${TEST_SRCS})
add_dependencies(arg_router_test translation_arg_router_test clangformat_test arg_router)

set_target_properties(arg_router_test PROPERTIES CXX_EXTENSIONS OFF)
target_compile_definitions(arg_router_test PRIVATE UNIT_TEST_BUILD)

# Default to C++20
if(NOT DEFINED CMAKE_CXX_STANDARD)
    target_compile_features(arg_router_test PUBLIC cxx_std_20)
endif()

set(DEATH_TEST_PARALLEL 8 CACHE STRING "Maximum number of parallel death tests to perform per suite")

function(configure_test_build TARGET)
    # Clang can run in different command line argument modes to mimic gcc or cl.exe,
    # so we have to test for a 'frontent variant' too
    if (MSVC_FRONTEND)
        set(EXTRA_FLAGS /MP /Zc:__cplusplus /W4 /Z7 /GR- /permissive- /bigobj /wd4996 ${ARGN})
        set(EXTRA_DEFINES NOMINMAX WIN32_LEAN_AND_MEAN BOOST_USE_WINDOWS_H _CRT_SECURE_NO_WARNINGS)

        # /MT by default as it simplifies the running of the unit tests
        set_property(TARGET ${TARGET} PROPERTY
                     MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

        if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            set(EXTRA_FLAGS ${EXTRA_FLAGS} /clang:-fconstexpr-steps=10000000)
        endif()
    else()
        set(EXTRA_FLAGS -Werror -Wall -Wextra -ftemplate-backtrace-limit=0 -fno-rtti
            -Wno-deprecated-declarations ${ARGN})
        set(EXTRA_DEFINES "")
    endif()
    target_compile_options(${TARGET} PRIVATE ${EXTRA_FLAGS})
    target_compile_definitions(${TARGET} PRIVATE
        ${EXTRA_DEFINES}
        AR_DEATH_TEST_PARALLEL=${DEATH_TEST_PARALLEL}
    )
endfunction()

configure_test_build(arg_router_test)
add_clangtidy_to_target(arg_router_test)
add_santizers_to_target(arg_router_test)

target_include_directories(arg_router_test
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(arg_router_test
    PRIVATE arg_router
    PUBLIC Boost::filesystem
    PUBLIC Threads::Threads
)

target_compile_definitions(arg_router_test PRIVATE
    UNIT_TEST_BUILD
    AR_REPO_PATH="${CMAKE_SOURCE_DIR}"
    UNIT_TEST_BIN_DIR="${CMAKE_CURRENT_BINARY_DIR}"
)

add_test(NAME arg_router_test COMMAND arg_router_test -l message)
