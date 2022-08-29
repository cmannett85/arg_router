### Copyright (C) 2022 by Camden Mannett.  All rights reserved.

cmake_minimum_required(VERSION 3.18)

include(CMakeDependentOption)

option(DOCS_ONLY "Only build the documentation" OFF)
cmake_dependent_option(BUILD_UNIT_TESTS_AND_EXAMPLES "Build the unit tests and examples" ON
                       "NOT DOCS_ONLY" OFF)
cmake_dependent_option(ENABLE_CLANG_TIDY "Enable clang-tidy warnings for unit tests and examples" OFF
                      "NOT DOCS_ONLY" OFF)
cmake_dependent_option(ENABLE_SANITIZERS "Enable ASan/UBSan for unit tests and examples" OFF
                       "NOT DOCS_ONLY" OFF)
cmake_dependent_option(DISABLE_VCPKG "Disable vcpkg, use system libraries" OFF
                       "NOT DOCS_ONLY" OFF)

# Use the libraries brought in from vcpkg rather than the system ones
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/path_prefixer.cmake")
if((NOT DOCS_ONLY) AND (NOT DISABLE_VCPKG))
    path_prefixer(CMAKE_TOOLCHAIN_FILE vcpkg/scripts/buildsystems/vcpkg.cmake)
endif()

project(arg_router
        VERSION 0.1.0
        DESCRIPTION "C++ command line argument parsing and routing"
        HOMEPAGE_URL "https://github.com/cmannett85/arg_router"
        LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(MSVC_FRONTEND OFF)
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    set(MSVC_FRONTEND ON)
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/versioning/version.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/copyright_checker.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/build_types/documentation.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/clang-tidy.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/sanitizers.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/package/install.cmake")

# Only find packages if we're not doing a Docs-only build
if(NOT DOCS_ONLY)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/ccache.cmake")

    find_package(span-lite)
    if (NOT span-lite_FOUND)
        message(WARNING "span-lite package not found, you will need to compile against a C++20 supporting compiler")
    else()
        include_directories(BEFORE SYSTEM ${span-lite_INCLUDE_DIRS})
    endif()

    set(BOOST_VERSION 1.77)
    find_package(Boost ${BOOST_VERSION} REQUIRED)
    include_directories(BEFORE SYSTEM ${Boost_INCLUDE_DIRS})
endif()

path_prefixer(HEADERS
    include/arg_router/algorithm.hpp
    include/arg_router/arg.hpp
    include/arg_router/arg_router.hpp
    include/arg_router/basic_types.hpp
    include/arg_router/config.hpp
    include/arg_router/counting_flag.hpp
    include/arg_router/dependency/alias_group.hpp
    include/arg_router/dependency/detail.hpp
    include/arg_router/dependency/one_of.hpp
    include/arg_router/exception.hpp
    include/arg_router/flag.hpp
    include/arg_router/help.hpp
    include/arg_router/list.hpp
    include/arg_router/math.hpp
    include/arg_router/mode.hpp
    include/arg_router/parsing/dynamic_token_adapter.hpp
    include/arg_router/parsing/global_parser.hpp
    include/arg_router/parsing/parse_target.hpp
    include/arg_router/parsing/parsing.hpp
    include/arg_router/parsing/pre_parse_data.hpp
    include/arg_router/parsing/token_type.hpp
    include/arg_router/policy/alias.hpp
    include/arg_router/policy/custom_parser.hpp
    include/arg_router/policy/default_value.hpp
    include/arg_router/policy/dependent.hpp
    include/arg_router/policy/description.hpp
    include/arg_router/policy/display_name.hpp
    include/arg_router/policy/flatten_help.hpp
    include/arg_router/policy/long_name.hpp
    include/arg_router/policy/none_name.hpp
    include/arg_router/policy/min_max_count.hpp
    include/arg_router/policy/min_max_value.hpp
    include/arg_router/policy/multi_stage_value.hpp
    include/arg_router/policy/no_result_value.hpp
    include/arg_router/policy/policy.hpp
    include/arg_router/policy/program_intro.hpp
    include/arg_router/policy/program_name.hpp
    include/arg_router/policy/program_version.hpp
    include/arg_router/policy/required.hpp
    include/arg_router/policy/router.hpp
    include/arg_router/policy/short_form_expander.hpp
    include/arg_router/policy/short_name.hpp
    include/arg_router/policy/validator.hpp
    include/arg_router/policy/validator_rule_utilities.hpp
    include/arg_router/policy/value_separator.hpp
    include/arg_router/positional_arg.hpp
    include/arg_router/root.hpp
    include/arg_router/traits.hpp
    include/arg_router/tree_node.hpp
    include/arg_router/tree_node_fwd.hpp
    include/arg_router/utility/compile_time_string.hpp
    include/arg_router/utility/compile_time_optional.hpp
    include/arg_router/utility/result.hpp
    include/arg_router/utility/string_view_ops.hpp
    include/arg_router/utility/terminal.hpp
    include/arg_router/utility/tree_recursor.hpp
    include/arg_router/utility/tuple_iterator.hpp
    include/arg_router/utility/unsafe_any.hpp
    include/arg_router/utility/utf8.hpp
    include/arg_router/utility/utf8/code_point.hpp
    include/arg_router/utility/utf8/double_width.hpp
    include/arg_router/utility/utf8/grapheme_cluster_break.hpp
    include/arg_router/utility/utf8/line_break.hpp
    include/arg_router/utility/utf8/whitespace.hpp
    include/arg_router/utility/utf8/zero_width.hpp
)

path_prefixer(FOR_IDE
    .clang-format
    .clang-tidy
    .clangd
    .github/workflows/docs_pusher.yml
    .github/workflows/merge_checker.yml
    .github/workflows/pr_checker.yml
    .gitignore
    cmake/package/arg_router-config.cmake.in
    cmake/versioning/version.cmake
    cmake/versioning/version.hpp.in
    LICENSE
    README.md
    scripts/ci/create_badge_url.sh
    scripts/ci/calculate_test_coverage.sh
    scripts/ci/docker/base.sh
    scripts/ci/docker/clang-9.sh
    scripts/ci/docker/clang++-9.sh
    scripts/ci/docker/cmake.sh
    scripts/ci/docker/Dockerfile
    scripts/ci/docker/g++-9.sh
    scripts/ci/docker/gcc-9.sh
    scripts/ci/docker/README.md
    scripts/copyright_checker.py
    scripts/unicode_table_generators.py
    test/.clang-tidy
    vcpkg.json
)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/clang-format.cmake")
create_clangformat_target(
    NAME clangformat
    FORMAT_FILE ${CMAKE_SOURCE_DIR}/.clang-format
    SOURCES ${HEADERS}
)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/build_types/library.cmake")

if(BUILD_UNIT_TESTS_AND_EXAMPLES)
    enable_testing()
    add_subdirectory(test)

    add_subdirectory(examples)
endif()
