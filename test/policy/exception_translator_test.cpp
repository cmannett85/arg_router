// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/policy/exception_translator.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace std::string_literals;

namespace
{
struct empty_translations {
    using error_code_translations = std::tuple<>;
};
}  // namespace

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(exception_translator_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(
        policy::is_policy_v<policy::exception_translator_t<default_error_code_translations, void>>,
        "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(traits_test)
{
    static_assert(traits::has_translate_exception_method_v<
                      policy::exception_translator_t<default_error_code_translations, void>>,
                  "Policy test has failed");
    static_assert(traits::has_error_code_translations_type_v<default_error_code_translations>,
                  "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(default_test)
{
    const auto et = policy::exception_translator<default_error_code_translations, void>;
    auto f = [&](const auto& ml_e, auto expected_message) {
        try {
            et.translate_exception(ml_e);
            BOOST_FAIL("Should not reach here");
        } catch (const parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), expected_message);
        }
    };

    test::data_set(
        f,
        {
            std::tuple{test::create_exception(error_code::unknown_argument, {"--foo"}),
                       "Unknown argument: --foo"},
            std::tuple{
                test::create_exception(error_code::unhandled_arguments, {"--foo", "hello", "-g"}),
                "Unhandled arguments: --foo, hello, -g"},
            std::tuple{test::create_exception(error_code::argument_has_already_been_set, {"--foo"}),
                       "Argument has already been set: --foo"},
            std::tuple{test::create_exception(error_code::failed_to_parse, {"42"}),
                       "Failed to parse: 42"},
            std::tuple{test::create_exception(error_code::no_arguments_passed),
                       "No arguments passed"},
            std::tuple{test::create_exception(error_code::minimum_value_not_reached, {"--foo"}),
                       "Minimum value not reached: --foo"},
            std::tuple{test::create_exception(error_code::maximum_value_exceeded, {"--foo"}),
                       "Maximum value exceeded: --foo"},
            std::tuple{test::create_exception(error_code::minimum_count_not_reached, {"--foo"}),
                       "Minimum count not reached: --foo"},
            std::tuple{test::create_exception(error_code::mode_requires_arguments, {"foo"}),
                       "Mode requires arguments: foo"},
            std::tuple{test::create_exception(error_code::missing_required_argument, {"--foo"}),
                       "Missing required argument: --foo"},
            std::tuple{test::create_exception(error_code::too_few_values_for_alias, {"--foo"}),
                       "Too few values for alias: --foo"},
            std::tuple{test::create_exception(error_code::dependent_argument_missing, {"--foo"}),
                       "Dependent argument missing (needs to be before the requiring token on the "
                       "command line): --foo"},
            std::tuple{test::create_exception(error_code::one_of_selected_type_mismatch, {"--foo"}),
                       "Only one argument from a \"One Of\" can be used at once: --foo"},
            std::tuple{test::create_exception(static_cast<error_code>(1048)),
                       "Untranslated error code (1048)"},
            std::tuple{test::create_exception(static_cast<error_code>(1048), {"--foo"}),
                       "Untranslated error code (1048): --foo"},
        });
}

BOOST_AUTO_TEST_CASE(fallback_test)
{
    const auto et =
        policy::exception_translator<empty_translations, default_error_code_translations>;
    auto f = [&](const auto& ml_e, auto expected_message) {
        try {
            et.translate_exception(ml_e);
            BOOST_FAIL("Should not reach here");
        } catch (const parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), expected_message);
        }
    };

    test::data_set(
        f,
        {
            std::tuple{test::create_exception(error_code::unknown_argument, {"--foo"}),
                       "Unknown argument: --foo"},
            std::tuple{
                test::create_exception(error_code::unhandled_arguments, {"--foo", "hello", "-g"}),
                "Unhandled arguments: --foo, hello, -g"},
            std::tuple{test::create_exception(error_code::argument_has_already_been_set, {"--foo"}),
                       "Argument has already been set: --foo"},
            std::tuple{test::create_exception(error_code::failed_to_parse, {"42"}),
                       "Failed to parse: 42"},
            std::tuple{test::create_exception(error_code::no_arguments_passed),
                       "No arguments passed"},
            std::tuple{test::create_exception(error_code::minimum_value_not_reached, {"--foo"}),
                       "Minimum value not reached: --foo"},
            std::tuple{test::create_exception(error_code::maximum_value_exceeded, {"--foo"}),
                       "Maximum value exceeded: --foo"},
            std::tuple{test::create_exception(error_code::minimum_count_not_reached, {"--foo"}),
                       "Minimum count not reached: --foo"},
            std::tuple{test::create_exception(error_code::mode_requires_arguments, {"foo"}),
                       "Mode requires arguments: foo"},
            std::tuple{test::create_exception(error_code::missing_required_argument, {"--foo"}),
                       "Missing required argument: --foo"},
            std::tuple{test::create_exception(error_code::too_few_values_for_alias, {"--foo"}),
                       "Too few values for alias: --foo"},
            std::tuple{test::create_exception(error_code::dependent_argument_missing, {"--foo"}),
                       "Dependent argument missing (needs to be before the requiring token on the "
                       "command line): --foo"},
            std::tuple{test::create_exception(static_cast<error_code>(1048)),
                       "Untranslated error code (1048)"},
            std::tuple{test::create_exception(static_cast<error_code>(1048), {"--foo"}),
                       "Untranslated error code (1048): --foo"},
        });
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
