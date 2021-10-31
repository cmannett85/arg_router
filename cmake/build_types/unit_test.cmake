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
    ${CMAKE_CURRENT_SOURCE_DIR}/flag_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/list_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mode_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/node_category_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parsing_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/custom_parser_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/alias_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/count_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/default_value_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/description_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/long_name_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/max_count_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/min_count_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/required_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/router_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/short_name_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/policy/validator_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/positional_arg_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/root_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_helpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/traits_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tree_node_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utility/compile_time_string_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utility/string_view_ops_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utility/tree_recursor_test.cpp
)

# Format just the unit test files
add_custom_target(clangformat_test
    COMMAND clang-format --style=file --Werror -i ${TEST_HEADERS} ${TEST_SRCS}
    SOURCES ${CLANG_FORMAT_FILE} ${TEST_HEADERS} ${TEST_SRCS}
)

add_executable(arg_router_test EXCLUDE_FROM_ALL
    ${TEST_HEADERS} ${TEST_SRCS})
add_dependencies(arg_router_test clangformat_test arg_router)

target_compile_features(arg_router_test PUBLIC cxx_std_17)
set_target_properties(arg_router_test PROPERTIES CXX_EXTENSIONS OFF)

target_compile_options(arg_router_test PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
        -Werror -Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4>
)

target_include_directories(arg_router_test
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(arg_router_test
    PUBLIC Boost::unit_test_framework
    PUBLIC Boost::filesystem
    PUBLIC Threads::Threads
    PUBLIC arg_router
)

add_test(NAME arg_router_test COMMAND arg_router_test -l message)
