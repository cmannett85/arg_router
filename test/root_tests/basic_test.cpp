// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/policy/description.hpp"
#include "arg_router/policy/validator.hpp"

#include "test_helpers.hpp"

#include <optional>

using namespace arg_router;
using namespace std::string_literals;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(root_suite)

BOOST_AUTO_TEST_CASE(anonymous_mode_single_flag_parse_test)
{
    auto router_hit = false;
    const auto r = root(mode(flag(policy::long_name<AR_STRING("hello")>,
                                  policy::description<AR_STRING("Hello description")>),
                             policy::router{[&](bool) { router_hit = true; }}),
                        policy::validation::default_validator);

    auto args = std::vector{"foo", "--hello"};
    r.parse(static_cast<int>(static_cast<int>(args.size())), const_cast<char**>(args.data()));
    BOOST_CHECK(router_hit);
}

BOOST_AUTO_TEST_CASE(anonymous_mode_single_arg_parse_test)
{
    auto result = std::optional<int>{};
    const auto r = root(mode(arg<int>(policy::long_name<AR_STRING("hello")>,
                                      policy::required,
                                      policy::description<AR_STRING("Hello description")>),
                             policy::router{[&](auto value) {
                                 BOOST_CHECK(!result);
                                 result = value;
                             }}),
                        policy::validation::default_validator);

    auto args = std::vector{"foo", "--hello", "42"};
    r.parse(static_cast<int>(static_cast<int>(args.size())), const_cast<char**>(args.data()));
    BOOST_REQUIRE(!!result);
    BOOST_CHECK_EQUAL(*result, 42);
}

BOOST_AUTO_TEST_CASE(required_arg_parse_test)
{
    const auto r = root(mode(flag(policy::long_name<AR_STRING("hello")>,
                                  policy::description<AR_STRING("Hello description")>),
                             arg<int>(policy::long_name<AR_STRING("arg")>,
                                      policy::required,
                                      policy::description<AR_STRING("Arg description")>),
                             policy::router{[&](auto, auto) {
                                 BOOST_CHECK_MESSAGE(false, "Router should not be called");
                             }}),
                        policy::validation::default_validator);

    auto args = std::vector{"foo", "--hello"};
    BOOST_CHECK_EXCEPTION(
        r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data())),
        parse_exception,
        [](const auto& e) { return e.what() == "Missing required argument: --arg"s; });
}

BOOST_AUTO_TEST_CASE(anonymous_mode_single_arg_default_parse_test)
{
    auto router_hit = false;
    auto result = std::tuple<bool, int, int>{};
    const auto r = root(mode(flag(policy::long_name<AR_STRING("hello")>,
                                  policy::description<AR_STRING("Hello description")>),
                             arg<int>(policy::long_name<AR_STRING("arg1")>,
                                      policy::default_value{42},
                                      policy::description<AR_STRING("Arg1 description")>),
                             arg<int>(policy::long_name<AR_STRING("arg2")>,
                                      policy::required,
                                      policy::description<AR_STRING("Arg2 description")>),
                             policy::router{[&](auto hello, auto arg1, auto arg2) {
                                 result = {hello, arg1, arg2};
                                 router_hit = true;
                             }}),
                        policy::validation::default_validator);

    auto f = [&](auto args, auto expected_value) {
        result = decltype(result){};
        router_hit = false;

        r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
        BOOST_CHECK(router_hit);

        BOOST_CHECK_EQUAL(std::get<0>(result), std::get<0>(expected_value));
        BOOST_CHECK_EQUAL(std::get<1>(result), std::get<1>(expected_value));
        BOOST_CHECK_EQUAL(std::get<2>(result), std::get<2>(expected_value));
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector{"foo", "--arg2", "84"}, std::tuple{false, 42, 84}},
            std::tuple{std::vector{"foo", "--arg2", "3", "--arg1", "19"}, std::tuple{false, 19, 3}},
            std::tuple{std::vector{"foo", "--hello", "--arg2", "14"}, std::tuple{true, 42, 14}},
        });
}

BOOST_AUTO_TEST_CASE(anonymous_mode_no_tokens_parse_test)
{
    auto router_hit = false;
    auto result = std::tuple<bool, int, int>{};
    const auto r = root(mode(flag(policy::long_name<AR_STRING("hello")>,
                                  policy::description<AR_STRING("Hello description")>),
                             arg<int>(policy::long_name<AR_STRING("arg1")>,
                                      policy::default_value{42},
                                      policy::description<AR_STRING("Arg1 description")>),
                             arg<int>(policy::long_name<AR_STRING("arg2")>,
                                      policy::default_value{84},
                                      policy::description<AR_STRING("Arg2 description")>),
                             policy::router{[&](auto hello, auto arg1, auto arg2) {
                                 result = {hello, arg1, arg2};
                                 router_hit = true;
                             }}),
                        policy::validation::default_validator);

    result = {};
    router_hit = false;

    auto args = std::vector{"foo"};
    r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
    BOOST_CHECK(router_hit);

    BOOST_CHECK_EQUAL(std::get<0>(result), false);
    BOOST_CHECK_EQUAL(std::get<1>(result), 42);
    BOOST_CHECK_EQUAL(std::get<2>(result), 84);
}

BOOST_AUTO_TEST_CASE(no_tokens_parse_test)
{
    auto router_hit = false;
    const auto r = root(flag(policy::long_name<AR_STRING("hello")>,
                             policy::description<AR_STRING("Hello description")>,
                             policy::router{[&](auto) { router_hit = true; }}),
                        arg<int>(policy::long_name<AR_STRING("arg1")>,
                                 policy::default_value{42},
                                 policy::description<AR_STRING("Arg1 description")>,
                                 policy::router{[&](auto) { router_hit = true; }}),
                        arg<int>(policy::long_name<AR_STRING("arg2")>,
                                 policy::default_value{84},
                                 policy::description<AR_STRING("Arg2 description")>,
                                 policy::router{[&](auto) { router_hit = true; }}),
                        policy::validation::default_validator);

    auto args = std::vector{"foo"};
    BOOST_CHECK_EXCEPTION(  //
        r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data())),
        parse_exception,
        [](const auto& e) { return e.what() == "No arguments passed"s; });
    BOOST_CHECK(!router_hit);
}

BOOST_AUTO_TEST_CASE(multiple_required_arg_parse_test)
{
    const auto r = root(mode(flag(policy::long_name<AR_STRING("hello")>,
                                  policy::description<AR_STRING("Hello description")>),
                             arg<int>(policy::long_name<AR_STRING("arg1")>,
                                      policy::required,
                                      policy::description<AR_STRING("Arg1 description")>),
                             arg<int>(policy::long_name<AR_STRING("arg2")>,
                                      policy::required,
                                      policy::description<AR_STRING("Arg2 description")>),
                             policy::router{[&](auto, auto, auto) {
                                 BOOST_CHECK_MESSAGE(false, "Router should not be called");
                             }}),
                        policy::validation::default_validator);

    auto args = std::vector{"foo", "--hello", "--arg2", "42"};
    BOOST_CHECK_EXCEPTION(
        r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data())),
        parse_exception,
        [](const auto& e) { return e.what() == "Missing required argument: --arg1"s; });
}

BOOST_AUTO_TEST_CASE(anonymous_triple_flag_parse_test)
{
    auto router_hit = false;
    auto result = std::array<bool, 3>{};
    const auto r = root(
        mode(flag(policy::long_name<AR_STRING("flag1")>,
                  policy::description<AR_STRING("First description")>),
             flag(policy::long_name<AR_STRING("flag2")>,
                  policy::description<AR_STRING("Second description")>),
             flag(policy::short_name<'t'>, policy::description<AR_STRING("Third description")>),
             policy::router{[&](bool flag1, bool flag2, bool t) {
                 result = {flag1, flag2, t};
                 router_hit = true;
             }}),
        policy::validation::default_validator);

    auto f = [&](auto args, auto expected, std::string fail_message) {
        result.fill(false);
        router_hit = false;

        try {
            r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK(router_hit);
            BOOST_CHECK_EQUAL(result[0], expected[0]);
            BOOST_CHECK_EQUAL(result[1], expected[1]);
            BOOST_CHECK_EQUAL(result[2], expected[2]);
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(fail_message, e.what());
            BOOST_CHECK(!router_hit);
        }
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector{"foo", "--flag1"}, std::array{true, false, false}, ""},
            std::tuple{std::vector{"foo", "--flag2"}, std::array{false, true, false}, ""},
            std::tuple{std::vector{"foo", "-t"}, std::array{false, false, true}, ""},
            std::tuple{std::vector{"foo", "--flag1", "-t"}, std::array{true, false, true}, ""},
            std::tuple{std::vector{"foo", "-t", "--flag1"}, std::array{true, false, true}, ""},
            std::tuple{std::vector{"foo", "--flag1", "--flag2", "-t"},
                       std::array{true, true, true},
                       ""},
            std::tuple{std::vector{"foo", "--flag2", "-t", "--flag1"},
                       std::array{true, true, true},
                       ""},
            std::tuple{std::vector{"foo", "--foo", "--flag2"},
                       std::array{false, false, false},
                       "Unknown argument: --foo. Did you mean -t?"},
            std::tuple{std::vector{"foo", "--flag2", "--foo"},
                       std::array{false, false, false},
                       "Unknown argument: --foo. Did you mean -t?"},
            std::tuple{std::vector{"foo", "--flag1", "--flag2", "-t", "--foo"},
                       std::array{false, false, false},
                       "Unhandled arguments: --foo"},
            std::tuple{std::vector{"foo", "--flag2", "-t", "--flag1", "--foo"},
                       std::array{false, false, false},
                       "Unhandled arguments: --foo"},
            std::tuple{std::vector{"foo", "--flag1", "--flag1"},
                       std::array{false, false, false},
                       "Argument has already been set: --flag1"},
            std::tuple{std::vector{"foo", "-t", "-t"},
                       std::array{false, false, false},
                       "Argument has already been set: -t"},
            std::tuple{std::vector{"foo", "--flag2", "-t", "--flag1", "--flag2"},
                       std::array{false, false, false},
                       "Argument has already been set: --flag2"},
        });
}

BOOST_AUTO_TEST_CASE(named_single_mode_parse_test)
{
    auto router_hit = false;
    auto result = std::array<bool, 3>{};
    const auto r = root(
        mode(policy::none_name<AR_STRING("my-mode")>,
             flag(policy::long_name<AR_STRING("flag1")>,
                  policy::description<AR_STRING("First description")>),
             flag(policy::long_name<AR_STRING("flag2")>,
                  policy::description<AR_STRING("Second description")>),
             flag(policy::short_name<'t'>, policy::description<AR_STRING("Third description")>),
             policy::router{[&](bool flag1, bool flag2, bool t) {
                 result = {flag1, flag2, t};
                 router_hit = true;
             }}),
        policy::validation::default_validator);

    auto f = [&](auto args, auto expected, std::string fail_message) {
        result.fill(false);
        router_hit = false;

        try {
            r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK(router_hit);
            BOOST_CHECK_EQUAL(result[0], expected[0]);
            BOOST_CHECK_EQUAL(result[1], expected[1]);
            BOOST_CHECK_EQUAL(result[2], expected[2]);
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(fail_message, e.what());
            BOOST_CHECK(!router_hit);
        }
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector{"foo", "my-mode", "--flag1"},
                       std::array{true, false, false},
                       ""},
            std::tuple{std::vector{"foo", "my-mode", "--flag2"},
                       std::array{false, true, false},
                       ""},
            std::tuple{std::vector{"foo", "my-mode", "-t"}, std::array{false, false, true}, ""},
            std::tuple{std::vector{"foo", "my-mode", "--flag1", "-t"},
                       std::array{true, false, true},
                       ""},
            std::tuple{std::vector{"foo", "my-mode", "-t", "--flag1"},
                       std::array{true, false, true},
                       ""},
            std::tuple{std::vector{"foo", "my-mode", "--flag1", "--flag2", "-t"},
                       std::array{true, true, true},
                       ""},
            std::tuple{std::vector{"foo", "my-mode", "--flag2", "-t", "--flag1"},
                       std::array{true, true, true},
                       ""},
            std::tuple{std::vector{"foo", "my-mode", "--foo", "--flag2"},
                       std::array{false, false, false},
                       "Unknown argument: --foo. Did you mean -t?"},
            std::tuple{std::vector{"foo", "my-mode", "--flag2", "--foo"},
                       std::array{false, false, false},
                       "Unknown argument: --foo. Did you mean -t?"},
            std::tuple{std::vector{"foo", "my-mode", "--flag1", "--flag2", "-t", "--foo"},
                       std::array{false, false, false},
                       "Unhandled arguments: --foo"},
            std::tuple{std::vector{"foo", "my-mode", "--flag2", "-t", "--flag1", "--foo"},
                       std::array{false, false, false},
                       "Unhandled arguments: --foo"},
            std::tuple{std::vector{"foo", "my-mode", "--flag1", "--flag1"},
                       std::array{false, false, false},
                       "Argument has already been set: --flag1"},
            std::tuple{std::vector{"foo", "my-mode", "-t", "-t"},
                       std::array{false, false, false},
                       "Argument has already been set: -t"},
            std::tuple{std::vector{"foo", "my-mode", "--flag2", "-t", "--flag1", "--flag2"},
                       std::array{false, false, false},
                       "Argument has already been set: --flag2"},
            std::tuple{std::vector{"foo", "--flag1"},
                       std::array{true, false, false},
                       "Unknown argument: --flag1. Did you mean my-mode --flag1?"},
            std::tuple{std::vector{"foo", "--foo"},
                       std::array{true, false, false},
                       "Unknown argument: --foo. Did you mean my-mode -t?"},
        });
}

BOOST_AUTO_TEST_CASE(anonymous_and_named_multi_mode_parse_test)
{
    auto router_hit1 = false;
    auto router_hit2 = false;
    auto router_hit3 = false;
    auto result1 = std::array<bool, 3>{};
    auto result2 = std::array<bool, 2>{};
    auto result3 = std::array<bool, 2>{};

    const auto r = root(
        mode(policy::none_name<AR_STRING("mode1")>,
             flag(policy::long_name<AR_STRING("flag1")>,
                  policy::description<AR_STRING("First description")>),
             flag(policy::long_name<AR_STRING("flag2")>,
                  policy::description<AR_STRING("Second description")>),
             flag(policy::short_name<'t'>, policy::description<AR_STRING("Third description")>),
             policy::router{[&](bool flag1, bool flag2, bool t) {
                 result1 = {flag1, flag2, t};
                 router_hit1 = true;
             }}),
        mode(policy::none_name<AR_STRING("mode2")>,
             flag(policy::long_name<AR_STRING("flag1")>,
                  policy::description<AR_STRING("Other third description")>),
             flag(policy::short_name<'b'>, policy::description<AR_STRING("Fourth description")>),
             policy::router{[&](bool flag1, bool b) {
                 result2 = {flag1, b};
                 router_hit2 = true;
             }}),
        mode(flag(policy::long_name<AR_STRING("flag3")>,
                  policy::description<AR_STRING("Other third description")>),
             flag(policy::short_name<'c'>, policy::description<AR_STRING("Fourth description")>),
             policy::router{[&](bool flag3, bool c) {
                 result3 = {flag3, c};
                 router_hit3 = true;
             }}),
        policy::validation::default_validator);

    auto f = [&](auto args, auto router_index, auto expected, std::string fail_message) {
        router_hit1 = false;
        router_hit2 = false;
        router_hit3 = false;
        result1.fill(false);
        result2.fill(false);
        result3.fill(false);

        try {
            r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
            BOOST_CHECK(fail_message.empty());

            if (router_index == 0) {
                BOOST_CHECK(router_hit1);
                BOOST_CHECK(!router_hit2);
                BOOST_CHECK_EQUAL(result1[0], expected[0]);
                BOOST_CHECK_EQUAL(result1[1], expected[1]);
                BOOST_CHECK_EQUAL(result1[2], expected[2]);
            } else if (router_index == 1) {
                BOOST_CHECK(!router_hit1);
                BOOST_CHECK(router_hit2);
                BOOST_CHECK_EQUAL(result2[0], expected[0]);
                BOOST_CHECK_EQUAL(result2[1], expected[1]);
            } else if (router_index == 2) {
                BOOST_CHECK(!router_hit1);
                BOOST_CHECK(!router_hit2);
                BOOST_CHECK(router_hit3);
                BOOST_CHECK_EQUAL(result3[0], expected[0]);
                BOOST_CHECK_EQUAL(result3[1], expected[1]);
            }
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(fail_message, e.what());
            BOOST_CHECK(!router_hit1);
            BOOST_CHECK(!router_hit2);
        }
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector{"foo", "mode1", "--flag1"},
                       0,
                       std::vector{true, false, false},
                       ""},
            std::tuple{std::vector{"foo", "mode2", "--flag1"}, 1, std::vector{true, false}, ""},
            std::tuple{std::vector{"foo", "mode1", "mode2", "--flag1"},
                       0,
                       std::vector{false, false, false},
                       "Unknown argument: mode2. Did you mean --flag2?"},
            std::tuple{std::vector{"foo", "mode2", "-b"}, 1, std::vector{false, true}, ""},
            std::tuple{std::vector{"foo", "--flag3"}, 2, std::vector{true, false}, ""},
            std::tuple{std::vector{"foo", "-c"}, 2, std::vector{false, true}, ""},
            std::tuple{std::vector{"foo", "-c", "--flag3"}, 2, std::vector{true, true}, ""},
            std::tuple{std::vector{"foo", "--flag1"},
                       2,
                       std::vector{false, false},
                       "Unknown argument: --flag1. Did you mean --flag3?"},
            std::tuple{std::vector{"foo", "-b"},
                       2,
                       std::vector{false, false},
                       "Unknown argument: -b. Did you mean -c?"},
        });
}

BOOST_AUTO_TEST_CASE(named_multi_mode_using_list_parse_test)
{
    auto router_hit1 = false;
    auto router_hit2 = false;
    auto result1 = std::array<bool, 3>{};
    auto result2 = std::array<bool, 2>{};

    const auto flag1 = list{flag(policy::long_name<AR_STRING("flag1")>,
                                 policy::description<AR_STRING("First description")>)};

    const auto r = root(
        mode(policy::none_name<AR_STRING("mode1")>,
             flag1,
             flag(policy::long_name<AR_STRING("flag2")>,
                  policy::description<AR_STRING("Second description")>),
             flag(policy::short_name<'t'>, policy::description<AR_STRING("Third description")>),
             policy::router{[&](bool flag1, bool flag2, bool t) {
                 result1 = {flag1, flag2, t};
                 router_hit1 = true;
             }}),
        mode(policy::none_name<AR_STRING("mode2")>,
             flag1,
             flag(policy::short_name<'b'>, policy::description<AR_STRING("Fourth description")>),
             policy::router{[&](bool flag1, bool b) {
                 result2 = {flag1, b};
                 router_hit2 = true;
             }}),
        policy::validation::default_validator);

    auto f = [&](auto args, auto router_index, auto expected, std::string fail_message) {
        router_hit1 = false;
        router_hit2 = false;
        result1.fill(false);
        result2.fill(false);

        try {
            r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
            BOOST_CHECK(fail_message.empty());

            if (router_index == 0) {
                BOOST_CHECK(router_hit1);
                BOOST_CHECK(!router_hit2);
                BOOST_CHECK_EQUAL(result1[0], expected[0]);
                BOOST_CHECK_EQUAL(result1[1], expected[1]);
                BOOST_CHECK_EQUAL(result1[2], expected[2]);
            } else {
                BOOST_CHECK(!router_hit1);
                BOOST_CHECK(router_hit2);
                BOOST_CHECK_EQUAL(result2[0], expected[0]);
                BOOST_CHECK_EQUAL(result2[1], expected[1]);
            }
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(fail_message, e.what());
            BOOST_CHECK(!router_hit1);
            BOOST_CHECK(!router_hit2);
        }
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector{"foo", "mode1", "--flag1"},
                       0,
                       std::vector{true, false, false},
                       ""},
            std::tuple{std::vector{"foo", "mode2", "--flag1"}, 1, std::vector{true, false}, ""},
            std::tuple{std::vector{"foo", "mode1", "mode2", "--flag1"},
                       0,
                       std::vector{false, false, false},
                       "Unknown argument: mode2. Did you mean --flag2?"},
            std::tuple{std::vector{"foo", "mode2", "-b"}, 1, std::vector{false, true}, ""},
        });
}

BOOST_AUTO_TEST_SUITE_END()