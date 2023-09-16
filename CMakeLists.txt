### Copyright (C) 2022-2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.18)

option(BUILD_STATIC "Build a static library, otherwise shared" ON)
option(BUILD_UNIT_TESTS_AND_EXAMPLES "Build the unit tests and examples" ON)
option(ENABLE_CLANG_FORMAT_CHECKS "Enable clang-format checks, fails build in case of formatting error" OFF)
option(ENABLE_CLANG_TIDY "Enable clang-tidy warnings for unit tests and examples" OFF)
option(ENABLE_SANITIZERS "Enable ASan/UBSan for unit tests and examples" OFF)
option(DISABLE_VCPKG "Disable vcpkg, use system libraries" OFF)

# Use the libraries brought in from vcpkg rather than the system ones, unless overridden
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/path_prefixer.cmake")
if(NOT DISABLE_VCPKG)
    path_prefixer(CMAKE_TOOLCHAIN_FILE external/vcpkg/scripts/buildsystems/vcpkg.cmake)
endif()

project(arg_router
        VERSION 1.4.0
        DESCRIPTION "C++ command line argument parsing and routing"
        HOMEPAGE_URL "https://github.com/cmannett85/arg_router"
        LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(MSVC_FRONTEND OFF)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    set(MSVC_FRONTEND ON)
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/build_types/documentation.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/versioning/version.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/copyright_checker.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/clang-tidy.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/sanitizers.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/ccache.cmake")

set(BOOST_VERSION 1.74)
find_package(Boost ${BOOST_VERSION} REQUIRED)

path_prefixer(HEADERS
    include/arg_router/algorithm.hpp
    include/arg_router/arg.hpp
    include/arg_router/arg_router.hpp
    include/arg_router/config.hpp
    include/arg_router/counting_flag.hpp
    include/arg_router/dependency/alias_group.hpp
    include/arg_router/dependency/detail.hpp
    include/arg_router/dependency/one_of.hpp
    include/arg_router/exception.hpp
    include/arg_router/flag.hpp
    include/arg_router/forwarding_arg.hpp
    include/arg_router/help.hpp
    include/arg_router/help_data.hpp
    include/arg_router/list.hpp
    include/arg_router/literals.hpp
    include/arg_router/math.hpp
    include/arg_router/mode.hpp
    include/arg_router/multi_arg.hpp
    include/arg_router/multi_arg_base.hpp
    include/arg_router/multi_lang/iso_locale.hpp
    include/arg_router/multi_lang/root.hpp
    include/arg_router/multi_lang/translation.hpp
    include/arg_router/parsing/dynamic_token_adapter.hpp
    include/arg_router/parsing/global_parser.hpp
    include/arg_router/parsing/parse_target.hpp
    include/arg_router/parsing/parsing.hpp
    include/arg_router/parsing/pre_parse_data.hpp
    include/arg_router/parsing/token_type.hpp
    include/arg_router/parsing/unknown_argument_handling.hpp
    include/arg_router/policy/alias.hpp
    include/arg_router/policy/colour_help_formatter.hpp
    include/arg_router/policy/custom_parser.hpp
    include/arg_router/policy/default_help_formatter.hpp
    include/arg_router/policy/default_value.hpp
    include/arg_router/policy/dependent.hpp
    include/arg_router/policy/description.hpp
    include/arg_router/policy/display_name.hpp
    include/arg_router/policy/error_name.hpp
    include/arg_router/policy/exception_translator.hpp
    include/arg_router/policy/flatten_help.hpp
    include/arg_router/policy/long_name.hpp
    include/arg_router/policy/none_name.hpp
    include/arg_router/policy/min_max_count.hpp
    include/arg_router/policy/min_max_value.hpp
    include/arg_router/policy/multi_stage_value.hpp
    include/arg_router/policy/no_result_value.hpp
    include/arg_router/policy/policy.hpp
    include/arg_router/policy/program_addendum.hpp
    include/arg_router/policy/program_intro.hpp
    include/arg_router/policy/program_name.hpp
    include/arg_router/policy/program_version.hpp
    include/arg_router/policy/required.hpp
    include/arg_router/policy/router.hpp
    include/arg_router/policy/runtime_enable.hpp
    include/arg_router/policy/short_form_expander.hpp
    include/arg_router/policy/short_name.hpp
    include/arg_router/policy/token_end_marker.hpp
    include/arg_router/policy/validator.hpp
    include/arg_router/policy/validator_rule_utilities.hpp
    include/arg_router/policy/value_separator.hpp
    include/arg_router/positional_arg.hpp
    include/arg_router/root.hpp
    include/arg_router/traits.hpp
    include/arg_router/tree_node.hpp
    include/arg_router/tree_node_fwd.hpp
    include/arg_router/utility/always_true.hpp
    include/arg_router/utility/apply.hpp
    include/arg_router/utility/compile_time_string.hpp
    include/arg_router/utility/compile_time_optional.hpp
    include/arg_router/utility/dynamic_string_view.hpp
    include/arg_router/utility/exception_formatter.hpp
    include/arg_router/utility/from_chars.hpp
    include/arg_router/utility/result.hpp
    include/arg_router/utility/string_to_policy.hpp
    include/arg_router/utility/string_view_ops.hpp
    include/arg_router/utility/terminal.hpp
    include/arg_router/utility/tree_recursor.hpp
    include/arg_router/utility/tuple_iterator.hpp
    include/arg_router/utility/type_hash.hpp
    include/arg_router/utility/unsafe_any.hpp
    include/arg_router/utility/utf8.hpp
    include/arg_router/utility/utf8/code_point.hpp
    include/arg_router/utility/utf8/double_width.hpp
    include/arg_router/utility/utf8/grapheme_cluster_break.hpp
    include/arg_router/utility/utf8/levenshtein_distance.hpp
    include/arg_router/utility/utf8/line_break.hpp
    include/arg_router/utility/utf8/whitespace.hpp
    include/arg_router/utility/utf8/zero_width.hpp
    include/arg_router/version.hpp
)

path_prefixer(SOURCES
    src/multi_lang/iso_locale.cpp
    src/parsing/dynamic_token_adapter.cpp
    src/parsing/global_parser.cpp
    src/parsing/token_type.cpp
    src/utility/terminal.cpp
    src/utility/utf8/levenshtein_distance.cpp
)

if(NOT INSTALLATION_ONLY)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/clang-format.cmake")
    create_clangformat_target(
        NAME clangformat
        FORMAT_FILE ${CMAKE_SOURCE_DIR}/.clang-format
        SOURCES ${HEADERS} ${SOURCES}
    )
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/build_types/library.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/package/install.cmake")

if(BUILD_UNIT_TESTS_AND_EXAMPLES)
    enable_testing()
    add_subdirectory(test)
    add_subdirectory(examples)
endif()
