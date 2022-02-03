/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/parsing.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/root.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(parsing_suite)

BOOST_AUTO_TEST_CASE(flag_default_match_test)
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

BOOST_AUTO_TEST_CASE(expand_arguments_test)
{
    auto f = [](auto input, auto expected) {
        const auto result = parsing::expand_arguments(input.size(),  //
                                                      input.data());
        BOOST_CHECK_EQUAL(result, expected);
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector{"program name", "--foo", "-g", "-d", "42"},
                       parsing::token_list{{parsing::prefix_type::LONG, "foo"},
                                           {parsing::prefix_type::SHORT, "g"},
                                           {parsing::prefix_type::SHORT, "d"},
                                           {parsing::prefix_type::NONE, "42"}}},
            std::tuple{std::vector{"program name", "-fwed"},
                       parsing::token_list{{parsing::prefix_type::SHORT, "f"},
                                           {parsing::prefix_type::SHORT, "w"},
                                           {parsing::prefix_type::SHORT, "e"},
                                           {parsing::prefix_type::SHORT, "d"}}},
            std::tuple{std::vector{"program name",
                                   "--foo",
                                   "42",
                                   "-venv",
                                   "-d",
                                   "-abc"},
                       parsing::token_list{{parsing::prefix_type::LONG, "foo"},
                                           {parsing::prefix_type::NONE, "42"},
                                           {parsing::prefix_type::SHORT, "v"},
                                           {parsing::prefix_type::SHORT, "e"},
                                           {parsing::prefix_type::SHORT, "n"},
                                           {parsing::prefix_type::SHORT, "v"},
                                           {parsing::prefix_type::SHORT, "d"},
                                           {parsing::prefix_type::SHORT, "a"},
                                           {parsing::prefix_type::SHORT, "b"},
                                           {parsing::prefix_type::SHORT, "c"}}},
        });
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
