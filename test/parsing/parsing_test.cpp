// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

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

BOOST_AUTO_TEST_CASE(match_test)
{
    {
        [[maybe_unused]] const auto f =
            flag(policy::long_name<AR_STRING("hello")>, policy::short_name<'H'>);
        const auto result =
            parsing::match<std::decay_t<decltype(f)>>({parsing::prefix_type::long_, "hello"});
        BOOST_CHECK(result);
    }

    {
        [[maybe_unused]] const auto f =
            flag(policy::long_name<AR_STRING("hello")>, policy::short_name<'H'>);
        const auto result =
            parsing::match<std::decay_t<decltype(f)>>({parsing::prefix_type::short_, "H"});
        BOOST_CHECK(result);
    }

    {
        [[maybe_unused]] const auto f =
            flag(policy::long_name<AR_STRING("hello")>, policy::short_name<'H'>);
        const auto result =
            parsing::match<std::decay_t<decltype(f)>>({parsing::prefix_type::long_, "foo"});
        BOOST_CHECK(!result);
    }

    {
        [[maybe_unused]] const auto f = flag(policy::long_name<AR_STRING("hello")>);
        const auto result =
            parsing::match<std::decay_t<decltype(f)>>({parsing::prefix_type::long_, "hello"});
        BOOST_CHECK(result);
    }

    {
        [[maybe_unused]] const auto f = flag(policy::long_name<AR_STRING("hello")>);
        const auto result =
            parsing::match<std::decay_t<decltype(f)>>({parsing::prefix_type::long_, "foo"});
        BOOST_CHECK(!result);
    }

    {
        [[maybe_unused]] const auto f = flag(policy::short_name<'H'>);
        const auto result =
            parsing::match<std::decay_t<decltype(f)>>({parsing::prefix_type::short_, "H"});
        BOOST_CHECK(result);
    }

    {
        [[maybe_unused]] const auto f = flag(policy::short_name<'H'>);
        const auto result =
            parsing::match<std::decay_t<decltype(f)>>({parsing::prefix_type::short_, "a"});
        BOOST_CHECK(!result);
    }

    {
        [[maybe_unused]] const auto a =
            arg<int>(policy::long_name<AR_STRING("arg")>, policy::value_separator<'='>);
        const auto result =
            parsing::match<std::decay_t<decltype(a)>>({parsing::prefix_type::long_, "arg"});
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
        {std::tuple{"--hello", parsing::token_type{parsing::prefix_type::long_, "hello"}},
         std::tuple{"-h", parsing::token_type{parsing::prefix_type::short_, "h"}},
         std::tuple{"hello", parsing::token_type{parsing::prefix_type::none, "hello"}},
         std::tuple{"", parsing::token_type{parsing::prefix_type::none, ""}}});
}

BOOST_AUTO_TEST_CASE(string_from_prefix_test)
{
    auto f = [](auto prefix, auto expected) {
        const auto result = parsing::to_string(prefix);
        BOOST_CHECK_EQUAL(result, expected);
    };

    test::data_set(f,
                   {
                       std::tuple{parsing::prefix_type::long_, "--"},
                       std::tuple{parsing::prefix_type::short_, "-"},
                       std::tuple{parsing::prefix_type::none, ""},
                   });
}

BOOST_AUTO_TEST_SUITE_END()
