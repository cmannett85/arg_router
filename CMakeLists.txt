### Copyright (C) 2022 by Camden Mannett.  All rights reserved. 

cmake_minimum_required(VERSION 3.15)

# Options
option(DOCS_ONLY "Only build the documentation")

# Use the libraries brought in from vcpkg rather than the system ones
if(NOT DOCS_ONLY)
    set(CMAKE_TOOLCHAIN_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake
    )
endif()

project(arg_router
        VERSION 0.0.1
        DESCRIPTION "C++ command line argument parsing and routing"
        HOMEPAGE_URL "https://github.com/cmannett85/arg_router"
        LANGUAGES CXX)

# Only enable LTO and find packages if we're not doing a Docs-only build
if(NOT DOCS_ONLY)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ccache.cmake)

    find_package(Threads REQUIRED)

    set(Boost_USE_MULTITHREADED ON)
    set(BOOST_VERSION 1.75)
    find_package(Boost ${BOOST_VERSION} REQUIRED)
    include_directories(BEFORE SYSTEM ${Boost_INCLUDE_DIRS})
endif()

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/algorithm.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/arg.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/basic_types.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/config.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/counting_flag.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/dependency/alias_group.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/dependency/detail.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/dependency/one_of.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/exception.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/flag.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/global_parser.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/help.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/list.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/math.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/mode.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/parsing.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/policy/alias.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/policy/custom_parser.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/policy/default_value.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/policy/dependent.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/policy/description.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/policy/display_name.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/policy/flatten_help.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/policy/long_name.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/policy/none_name.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/policy/min_max_count.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/policy/min_max_value.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/policy/multi_stage_value.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/policy/no_result_value.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/policy/policy.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/policy/program_intro.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/policy/program_name.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/policy/program_version.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/policy/required.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/policy/router.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/policy/short_name.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/policy/validator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/positional_arg.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/root.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/token_type.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/traits.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/tree_node.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/tree_node_fwd.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/utility/compile_time_string.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/utility/string_view_ops.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/utility/tree_recursor.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/utility/tuple_iterator.hpp
)

set(FOR_IDE
    ${CMAKE_CURRENT_SOURCE_DIR}/.clang-format
    ${CMAKE_CURRENT_SOURCE_DIR}/.github/workflows/docs_pusher.yml
    ${CMAKE_CURRENT_SOURCE_DIR}/.github/workflows/pr_checker.yml
    ${CMAKE_CURRENT_SOURCE_DIR}/.gitignore
    ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile
    ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
    ${CMAKE_CURRENT_SOURCE_DIR}/README.md
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ci/create_badge_url.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ci/calculate_test_coverage.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ci/docker/base.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ci/docker/clang-9.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ci/docker/clang++-9.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ci/docker/cmake.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ci/docker/Dockerfile
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ci/docker/g++-9.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ci/docker/gcc-9.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ci/docker/README.md
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/copyright_checker.py
    ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg.json
)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/versioning/version.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/build_types/documentation.cmake)

if(NOT DOCS_ONLY)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/copyright_checker.cmake)
    create_copyright_checker_target(
        NAME copyright_checker
        SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/scripts/copyright_checker.py
        SOURCES ${HEADERS} ${FOR_IDE}
    )

    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/clang-format.cmake)
    create_clangformat_target(
        NAME clangformat
        FORMAT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/.clang-format
        DEPENDS copyright_checker
        SOURCES ${HEADERS}
    )

    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/build_types/library.cmake)

    enable_testing()
    add_subdirectory(test)
endif()
