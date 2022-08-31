### Copyright (C) 2022 by Camden Mannett.  All rights reserved. 

path_prefixer(TEST_COVERAGE_FOR_IDE
    ../scripts/ci/calculate_test_coverage.sh
    ../scripts/ci/old_coverage
)

add_executable(arg_router_test_coverage EXCLUDE_FROM_ALL
    ${TEST_HEADERS} ${TEST_SRCS} ${TEST_COVERAGE_FOR_IDE})
add_dependencies(arg_router_test_coverage clangformat_test arg_router)

target_compile_features(arg_router_test_coverage PUBLIC cxx_std_17)
set_target_properties(arg_router_test_coverage PROPERTIES CXX_EXTENSIONS OFF)
target_compile_definitions(arg_router_test_coverage PRIVATE
    UNIT_TEST_BUILD
    AR_REPO_PATH="${CMAKE_SOURCE_DIR}"
)

configure_test_build(arg_router_test_coverage --coverage)
add_clangtidy_to_target(arg_router_test_coverage)
add_santizers_to_target(arg_router_test_coverage)

target_include_directories(arg_router_test_coverage
    PUBLIC "${CMAKE_SOURCE_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(arg_router_test_coverage
    PUBLIC Boost::filesystem
    PUBLIC Threads::Threads
)

target_link_options(arg_router_test_coverage
    PRIVATE --coverage
)
