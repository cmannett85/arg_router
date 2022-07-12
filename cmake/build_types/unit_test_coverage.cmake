### Copyright (C) 2022 by Camden Mannett.  All rights reserved. 

set(TEST_COVERAGE_FOR_IDE
    ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/ci/calculate_test_coverage.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/ci/old_coverage
)

add_executable(arg_router_test_coverage EXCLUDE_FROM_ALL
    ${TEST_HEADERS} ${TEST_SRCS} ${TEST_COVERAGE_FOR_IDE})
add_dependencies(arg_router_test_coverage clangformat_test arg_router)

target_compile_features(arg_router_test_coverage PUBLIC cxx_std_17)
set_target_properties(arg_router_test_coverage PROPERTIES CXX_EXTENSIONS OFF)
target_compile_definitions(arg_router_test_coverage PRIVATE UNIT_TEST_BUILD)

configure_test_build(arg_router_test_coverage --coverage)

target_include_directories(arg_router_test_coverage
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(arg_router_test_coverage
    PUBLIC Boost::unit_test_framework
    PUBLIC Boost::filesystem
    PUBLIC Threads::Threads
)

target_link_options(arg_router_test_coverage
    PRIVATE --coverage
)
