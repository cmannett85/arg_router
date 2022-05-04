### Copyright (C) 2022 by Camden Mannett.  All rights reserved. 

find_package(Boost ${BOOST_VERSION} REQUIRED COMPONENTS
    unit_test_framework
    filesystem
)

set(TEST_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/test_printers.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_helpers.hpp
)

set(TEST_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/main_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/algorithm_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/arg_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/counting_flag_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dependency/alias_group_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dependency/one_of_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flag_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/global_parser_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/help_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/list_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/math_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parsing_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/custom_parser_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/alias_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/default_value_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/dependent_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/description_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/display_name_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/long_name_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/min_max_count_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/min_max_value_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/required_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/router_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/short_form_expander_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/short_name_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/validator_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/value_separator_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/positional_arg_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dynamic_token_adapter_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/root_tests/death_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/root_tests/positional_arg_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/root_tests/top_level_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/root_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_helpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/token_list_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/traits_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tree_node_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utility/compile_time_string_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utility/result_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utility/string_view_ops_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utility/tree_recursor_test.cpp
)

# Format just the unit test files
create_clangformat_target(
    NAME clangformat_test
    FORMAT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../.clang-format
    DEPENDS clangformat
    SOURCES ${TEST_HEADERS} ${TEST_SRCS}
)

add_executable(arg_router_test EXCLUDE_FROM_ALL
    ${TEST_HEADERS} ${TEST_SRCS})
add_dependencies(arg_router_test clangformat_test arg_router)

target_compile_features(arg_router_test PUBLIC cxx_std_17)
set_target_properties(arg_router_test PROPERTIES CXX_EXTENSIONS OFF)

function(configure_test_build TARGET)
    # Clang can run in different command line argument modes to mimic gcc or cl.exe,
    # so we have to test for a 'frontent variant' too
    set(EXTRA_FLAGS -Werror -Wall -Wextra -ftemplate-backtrace-limit=0 ${ARGN})
    set(EXTRA_DEFINES "")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        set(EXTRA_FLAGS /W4)
        set(EXTRA_DEFINES NOMINMAX BOOST_USE_WINDOWS_H WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS)

        # /MT by default as it simplifies the running of the unit tests
        set_property(TARGET ${TARGET} PROPERTY
                     MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
    target_compile_options(${TARGET} PRIVATE ${EXTRA_FLAGS})
    target_compile_definitions(${TARGET} PRIVATE ${EXTRA_DEFINES})
endfunction()

configure_test_build(arg_router_test)

target_include_directories(arg_router_test
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(arg_router_test
    PUBLIC Boost::unit_test_framework
    PUBLIC Boost::filesystem
    PUBLIC Threads::Threads
)

add_test(NAME arg_router_test COMMAND arg_router_test -l message)
