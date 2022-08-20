### Copyright (C) 2022 by Camden Mannett.  All rights reserved. 

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
    arg_test.cpp
    counting_flag_test.cpp
    dependency/alias_group_test.cpp
    dependency/one_of_test.cpp
    flag_test.cpp
    help_test.cpp
    list_test.cpp
    math_test.cpp
    mode_test.cpp
    parsing/dynamic_token_adapter_test.cpp
    parsing/global_parser_test.cpp
    parsing/parse_target_test.cpp
    parsing/parsing_test.cpp
    parsing/pre_parse_data_test.cpp
    policy/custom_parser_test.cpp
    policy/alias_test.cpp
    policy/default_value_test.cpp
    policy/dependent_test.cpp
    policy/description_test.cpp
    policy/display_name_test.cpp
    policy/long_name_test.cpp
    policy/min_max_count_test.cpp
    policy/min_max_value_test.cpp
    policy/required_test.cpp
    policy/router_test.cpp
    policy/short_form_expander_test.cpp
    policy/short_name_test.cpp
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
    tree_node_test.cpp
    utility/compile_time_string_test.cpp
    utility/compile_time_optional_test.cpp
    utility/result_test.cpp
    utility/string_view_ops_test.cpp
    utility/tree_recursor_test.cpp
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

add_executable(arg_router_test EXCLUDE_FROM_ALL ${TEST_HEADERS} ${TEST_SRCS})
add_dependencies(arg_router_test clangformat_test arg_router)

target_compile_features(arg_router_test PUBLIC cxx_std_17)
set_target_properties(arg_router_test PROPERTIES CXX_EXTENSIONS OFF)
target_compile_definitions(arg_router_test PRIVATE UNIT_TEST_BUILD)

set(DEATH_TEST_PARALLEL 8 CACHE STRING "Maximum number of parallel death tests to perform per suite")

function(configure_test_build TARGET)
    # Clang can run in different command line argument modes to mimic gcc or cl.exe,
    # so we have to test for a 'frontent variant' too
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        set(EXTRA_FLAGS /W4 /Z7 /MP /clang:-fconstexpr-steps=10000000 ${ARGN})
        set(EXTRA_DEFINES NOMINMAX BOOST_USE_WINDOWS_H WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS)

        # /MT by default as it simplifies the running of the unit tests
        set_property(TARGET ${TARGET} PROPERTY
                     MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else()
        set(EXTRA_FLAGS -Werror -Wall -Wextra -ftemplate-backtrace-limit=0 ${ARGN})
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

target_include_directories(arg_router_test
    PUBLIC "${CMAKE_SOURCE_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(arg_router_test
    PUBLIC Boost::filesystem
    PUBLIC Threads::Threads
)

target_compile_definitions(arg_router_test PRIVATE UNIT_TEST_BUILD)

add_test(NAME arg_router_test COMMAND arg_router_test -l message)
