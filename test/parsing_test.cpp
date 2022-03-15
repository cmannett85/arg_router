/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/parsing.hpp"
#include "arg_router/arg.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/policy/value_separator.hpp"
#include "arg_router/root.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(parsing_suite)

BOOST_AUTO_TEST_CASE(default_match_test)
{
    {
        const auto f =
            flag(policy::long_name<S_("hello")>, policy::short_name<'H'>);
        const auto result = parsing::default_match<std::decay_t<decltype(f)>>(
            {parsing::prefix_type::LONG, "hello"});
        BOOST_CHECK(result);
    }

    {
        const auto f =
            flag(policy::long_name<S_("hello")>, policy::short_name<'H'>);
        const auto result = parsing::default_match<std::decay_t<decltype(f)>>(
            {parsing::prefix_type::SHORT, "H"});
        BOOST_CHECK(result);
    }

    {
        const auto f =
            flag(policy::long_name<S_("hello")>, policy::short_name<'H'>);
        const auto result = parsing::default_match<std::decay_t<decltype(f)>>(
            {parsing::prefix_type::LONG, "foo"});
        BOOST_CHECK(!result);
    }

    {
        const auto f = flag(policy::long_name<S_("hello")>);
        const auto result = parsing::default_match<std::decay_t<decltype(f)>>(
            {parsing::prefix_type::LONG, "hello"});
        BOOST_CHECK(result);
    }

    {
        const auto f = flag(policy::long_name<S_("hello")>);
        const auto result = parsing::default_match<std::decay_t<decltype(f)>>(
            {parsing::prefix_type::LONG, "foo"});
        BOOST_CHECK(!result);
    }

    {
        const auto f = flag(policy::short_name<'H'>);
        const auto result = parsing::default_match<std::decay_t<decltype(f)>>(
            {parsing::prefix_type::SHORT, "H"});
        BOOST_CHECK(result);
    }

    {
        const auto f = flag(policy::short_name<'H'>);
        const auto result = parsing::default_match<std::decay_t<decltype(f)>>(
            {parsing::prefix_type::SHORT, "a"});
        BOOST_CHECK(!result);
    }

    {
        const auto a = arg<int>(policy::long_name<S_("arg")>,
                                policy::value_separator<'='>);
        const auto result = parsing::default_match<std::decay_t<decltype(a)>>(
            {parsing::prefix_type::LONG, "arg"});
        BOOST_CHECK(result);
    }

    {
        const auto a = arg<int>(policy::long_name<S_("arg")>,
                                policy::value_separator<'='>);
        const auto result = parsing::default_match<std::decay_t<decltype(a)>>(
            {parsing::prefix_type::LONG, "arg=42"});
        BOOST_CHECK(result);
    }
}

BOOST_AUTO_TEST_CASE(get_token_type_test)
{
    auto f = [](auto token, auto expected_token) {
        const auto result = parsing::get_token_type(token);
        BOOST_CHECK_EQUAL(result, expected_token);
    };

    test::data_set(
        f,
        {std::tuple{"--hello",
                    parsing::token_type{parsing::prefix_type::LONG, "hello"}},
         std::tuple{"-h",
                    parsing::token_type{parsing::prefix_type::SHORT, "h"}},
         std::tuple{"hello",
                    parsing::token_type{parsing::prefix_type::NONE, "hello"}},
         std::tuple{"", parsing::token_type{parsing::prefix_type::NONE, ""}}});
}

BOOST_AUTO_TEST_CASE(string_from_prefix_test)
{
    auto f = [](auto prefix, auto expected) {
        const auto result = parsing::to_string(prefix);
        BOOST_CHECK_EQUAL(result, expected);
    };

    test::data_set(f,
                   {
                       std::tuple{parsing::prefix_type::LONG, "--"},
                       std::tuple{parsing::prefix_type::SHORT, "-"},
                       std::tuple{parsing::prefix_type::NONE, ""},
                   });
}

BOOST_AUTO_TEST_SUITE_END()
