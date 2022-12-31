// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/policy/dependent.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/min_max_value.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/policy/value_separator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"

#include <bitset>
#include <optional>

using namespace arg_router;
namespace ard = arg_router::dependency;
using namespace std::string_literals;
using namespace std::string_view_literals;

namespace
{
using default_validator_type = std::decay_t<decltype(policy::validation::default_validator)>;
}  // namespace

BOOST_AUTO_TEST_SUITE(root_suite)

BOOST_AUTO_TEST_CASE(anonymous_mode_single_flag_parse_test)
{
    auto router_hit = false;
    const auto r = root(mode(flag(policy::long_name<AR_STRING("hello")>,
                                  policy::description<AR_STRING("Hello description")>),
                             policy::router{[&](bool) { router_hit = true; }}),
                        policy::validation::default_validator);

    auto args = std::vector{"foo", "--hello"};
    r.parse(args.size(), const_cast<char**>(args.data()));
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
    r.parse(args.size(), const_cast<char**>(args.data()));
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
        r.parse(args.size(), const_cast<char**>(args.data())),
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

        r.parse(args.size(), const_cast<char**>(args.data()));
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
    r.parse(args.size(), const_cast<char**>(args.data()));
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
        r.parse(args.size(), const_cast<char**>(args.data())),
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
        r.parse(args.size(), const_cast<char**>(args.data())),
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
            r.parse(args.size(), const_cast<char**>(args.data()));
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
                       "Unknown argument: --foo"},
            std::tuple{std::vector{"foo", "--flag2", "--foo"},
                       std::array{false, false, false},
                       "Unknown argument: --foo"},
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
            r.parse(args.size(), const_cast<char**>(args.data()));
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
                       "Unknown argument: --foo"},
            std::tuple{std::vector{"foo", "my-mode", "--flag2", "--foo"},
                       std::array{false, false, false},
                       "Unknown argument: --foo"},
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
                       "Unknown argument: --flag1"},
            std::tuple{std::vector{"foo", "--foo"},
                       std::array{true, false, false},
                       "Unknown argument: --foo"},
        });
}

BOOST_AUTO_TEST_CASE(named_multi_mode_parse_test)
{
    auto router_hit1 = false;
    auto router_hit2 = false;
    auto result1 = std::array<bool, 3>{};
    auto result2 = std::array<bool, 2>{};

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
        policy::validation::default_validator);

    auto f = [&](auto args, auto router_index, auto expected, std::string fail_message) {
        router_hit1 = false;
        router_hit2 = false;
        result1.fill(false);
        result2.fill(false);

        try {
            r.parse(args.size(), const_cast<char**>(args.data()));
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
                       "Unknown argument: mode2"},
            std::tuple{std::vector{"foo", "mode2", "-b"}, 1, std::vector{false, true}, ""},
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
            r.parse(args.size(), const_cast<char**>(args.data()));
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
                       "Unknown argument: mode2"},
            std::tuple{std::vector{"foo", "mode2", "-b"}, 1, std::vector{false, true}, ""},
        });
}

BOOST_AUTO_TEST_CASE(alias_flag_parse_test)
{
    auto router_hit = false;
    auto result = std::array<bool, 3>{};
    const auto r = root(mode(flag(policy::long_name<AR_STRING("flag1")>,
                                  policy::description<AR_STRING("First description")>),
                             flag(policy::long_name<AR_STRING("flag2")>,
                                  policy::description<AR_STRING("Second description")>),
                             flag(policy::long_name<AR_STRING("flag3")>,
                                  policy::description<AR_STRING("Third description")>),
                             flag(policy::short_name<'a'>,
                                  policy::alias(policy::long_name<AR_STRING("flag1")>,
                                                policy::long_name<AR_STRING("flag3")>),
                                  policy::description<AR_STRING("Alias description")>),
                             policy::router{[&](bool flag1, bool flag2, bool flag3) {
                                 result = {flag1, flag2, flag3};
                                 router_hit = true;
                             }}),
                        policy::validation::default_validator);

    auto f = [&](auto args, auto expected, std::string fail_message) {
        result.fill(false);
        router_hit = false;

        try {
            r.parse(args.size(), const_cast<char**>(args.data()));
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
            std::tuple{std::vector{"foo", "--flag3"}, std::array{false, false, true}, ""},
            std::tuple{std::vector{"foo", "-a"}, std::array{true, false, true}, ""},
            std::tuple{std::vector{"foo", "-a", "--flag2"}, std::array{true, true, true}, ""},
            std::tuple{std::vector{"foo", "-a", "--flag1"},
                       std::array{true, false, true},
                       "Argument has already been set: --flag1"},
        });
}

BOOST_AUTO_TEST_CASE(alias_arg_parse_test)
{
    auto router_hit = false;
    auto result = std::tuple<bool, int, int>{};
    const auto r = root(mode(arg<bool>(policy::long_name<AR_STRING("arg1")>,
                                       policy::required,
                                       policy::description<AR_STRING("First description")>),
                             arg<int>(policy::long_name<AR_STRING("arg2")>,
                                      policy::default_value(42),
                                      policy::description<AR_STRING("Second description")>),
                             arg<int>(policy::long_name<AR_STRING("arg3")>,
                                      policy::value_separator<'='>,
                                      policy::default_value(84),
                                      policy::description<AR_STRING("Third description")>),
                             arg<int>(policy::short_name<'a'>,
                                      policy::alias(policy::long_name<AR_STRING("arg2")>,
                                                    policy::long_name<AR_STRING("arg3")>),
                                      policy::description<AR_STRING("Alias description")>),
                             policy::router{[&](bool arg1, int arg2, int arg3) {
                                 result = decltype(result){arg1, arg2, arg3};
                                 router_hit = true;
                             }}),
                        policy::validation::default_validator);

    auto f = [&](auto args, auto expected, std::string fail_message) {
        result = decltype(result){};
        router_hit = false;

        try {
            r.parse(args.size(), const_cast<char**>(args.data()));
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK(router_hit);
            BOOST_CHECK_EQUAL(std::get<0>(result), std::get<0>(expected));
            BOOST_CHECK_EQUAL(std::get<1>(result), std::get<1>(expected));
            BOOST_CHECK_EQUAL(std::get<2>(result), std::get<2>(expected));
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(fail_message, e.what());
            BOOST_CHECK(!router_hit);
        }
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector{"foo", "--arg1", "true"}, std::tuple{true, 42, 84}, ""},
            std::tuple{std::vector{"foo", "--arg1", "false", "-a", "9"},
                       std::tuple{false, 9, 9},
                       ""},
            std::tuple{std::vector{"foo", "--arg1", "false", "--arg3=9"},
                       std::tuple{false, 42, 9},
                       ""},
            std::tuple{std::vector{"foo", "--arg2", "13", "-a", "9"},
                       std::tuple{false, 0, 0},
                       "Argument has already been set: --arg2"},
            std::tuple{std::vector{"foo", "--arg3=13", "-a", "9"},
                       std::tuple{false, 0, 0},
                       "Argument has already been set: --arg3"},
            std::tuple{std::vector{"foo", "-a", "9"},
                       std::tuple{false, 9, 9},
                       "Missing required argument: --arg1"},
        });
}

BOOST_AUTO_TEST_CASE(nested_mode_test)
{
    auto router_hit = std::bitset<6>{};
    auto result = std::variant<std::tuple<bool>,
                               std::tuple<int>,
                               std::tuple<bool, double, bool>,
                               std::tuple<int, bool, std::vector<std::string_view>>,
                               std::tuple<bool, bool>,
                               std::tuple<bool, double>>{};

    const auto r =
        root(flag(policy::long_name<AR_STRING("top-flag")>,
                  policy::description<AR_STRING("Description")>,
                  policy::router{[&](bool v) {
                      router_hit[0] = true;
                      result = std::tuple{v};
                  }}),
             arg<int>(policy::long_name<AR_STRING("top-arg")>,
                      policy::description<AR_STRING("Description")>,
                      policy::router{[&](int v) {
                          router_hit[1] = true;
                          result = std::tuple{v};
                      }}),
             mode(policy::none_name<AR_STRING("mode1")>,
                  flag(policy::long_name<AR_STRING("flag1")>,
                       policy::description<AR_STRING("Description")>),
                  arg<double>(policy::long_name<AR_STRING("arg1")>,
                              policy::description<AR_STRING("Description")>,
                              policy::default_value{3.14}),
                  flag(policy::long_name<AR_STRING("flag2")>,
                       policy::short_name<'t'>,
                       policy::description<AR_STRING("Description")>),
                  policy::router{[&](bool f1, double a1, bool f2) {
                      router_hit[2] = true;
                      result = std::tuple{f1, a1, f2};
                  }},
                  mode(policy::none_name<AR_STRING("mode2")>,
                       arg<int>(policy::long_name<AR_STRING("arg1")>,
                                policy::description<AR_STRING("Description")>,
                                policy::required),
                       flag(policy::long_name<AR_STRING("flag1")>,
                            policy::short_name<'b'>,
                            policy::description<AR_STRING("Description")>),
                       positional_arg<std::vector<std::string_view>>(
                           policy::display_name<AR_STRING("pos_args")>,
                           policy::description<AR_STRING("Description")>),
                       policy::router{[&](int a1, bool f1, std::vector<std::string_view> pos_args) {
                           router_hit[3] = true;
                           result = std::tuple{a1, f1, std::move(pos_args)};
                       }}),
                  mode(policy::none_name<AR_STRING("mode3")>,
                       flag(policy::long_name<AR_STRING("flag1")>,
                            policy::description<AR_STRING("Description")>),
                       flag(policy::long_name<AR_STRING("flag2")>,
                            policy::short_name<'b'>,
                            policy::description<AR_STRING("Description")>),
                       policy::router{[&](bool f1, bool f2) {
                           router_hit[4] = true;
                           result = std::tuple{f1, f2};
                       }})),
             mode(flag(policy::long_name<AR_STRING("flag1")>,
                       policy::description<AR_STRING("Description")>),
                  arg<double>(policy::long_name<AR_STRING("arg1")>,
                              policy::default_value{4.2},
                              policy::description<AR_STRING("Description")>),
                  policy::router{[&](bool f1, double a1) {
                      router_hit[5] = true;
                      result = std::tuple{f1, a1};
                  }}),
             policy::validation::default_validator);

    auto f = [&](auto args, auto expected_index, auto expected, std::string fail_message) {
        result = std::tuple{false};
        router_hit.reset();

        try {
            r.parse(args.size(), const_cast<char**>(args.data()));
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK_EQUAL(router_hit.count(), 1);
            BOOST_CHECK(router_hit[expected_index]);

            std::visit(
                [&](const auto& result_tuple) {
                    using result_type = std::decay_t<decltype(result_tuple)>;
                    using expected_type = std::decay_t<decltype(expected)>;

                    if constexpr (std::is_same_v<result_type, expected_type>) {
                        BOOST_CHECK(result_tuple == expected);
                    } else {
                        BOOST_CHECK_MESSAGE(false, "Unexpected router arg type");
                    }
                },
                result);
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(fail_message, e.what());
            BOOST_CHECK(router_hit.none());
        }
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::vector{"foo", "--top-flag"}, 0, std::tuple{true}, ""},
            std::tuple{std::vector{"foo", "--top-arg", "42"}, 1, std::tuple{42}, ""},
            std::tuple{std::vector{"foo"}, 5, std::tuple{false, 4.2}, ""},
            std::tuple{std::vector{"foo", "--arg1", "13"}, 5, std::tuple{false, 13.0}, ""},
            std::tuple{std::vector{"foo", "mode1", "-t"}, 2, std::tuple{false, 3.14, true}, ""},
            std::tuple{std::vector{"foo", "mode1", "--arg1", "5.6", "--flag1"},
                       2,
                       std::tuple{true, 5.6, false},
                       ""},
            std::tuple{std::vector{"foo", "mode1", "mode2", "--arg1", "89"},
                       3,
                       std::tuple{89, false, std::vector<std::string_view>{}},
                       ""},
            std::tuple{std::vector{"foo", "mode1", "mode2", "-b", "--arg1", "4"},
                       3,
                       std::tuple{4, true, std::vector<std::string_view>{}},
                       ""},
            std::tuple{std::vector{"foo", "mode1", "mode3", "-b"}, 4, std::tuple{false, true}, ""},
            std::tuple{std::vector{"foo", "mode1", "mode2", "--arg1", "8", "hello", "goodbye"},
                       3,
                       std::tuple{8, false, std::vector<std::string_view>{"hello", "goodbye"}},
                       ""},
            std::tuple{std::vector{"foo", "--foo2"},
                       0,
                       std::tuple{true},
                       "Unknown argument: --foo2"},
            std::tuple{std::vector{"foo", "mode1", "--foo2"},
                       0,
                       std::tuple{true},
                       "Unknown argument: --foo2"},
        });
}

BOOST_AUTO_TEST_CASE(one_of_required_test)
{
    auto f = [&](auto args, auto arg1_expected, auto of_expected, std::string fail_message) {
        auto result = std::optional<std::tuple<int, std::variant<bool, int, std::string_view>>>{};

        const auto r =
            root(mode(arg<int>(policy::long_name<AR_STRING("arg1")>, policy::default_value{42}),
                      ard::one_of(flag(policy::short_name<'f'>),
                                  arg<int>(policy::long_name<AR_STRING("arg2")>),
                                  arg<std::string_view>(policy::long_name<AR_STRING("arg3")>),
                                  policy::required),
                      policy::router{[&](int arg1, std::variant<bool, int, std::string_view> of) {
                          result = std::tuple{arg1, std::move(of)};
                      }}),
                 policy::validation::default_validator);

        try {
            r.parse(args.size(), const_cast<char**>(args.data()));
            BOOST_CHECK(fail_message.empty());
            BOOST_REQUIRE(!!result);

            BOOST_CHECK_EQUAL(std::get<0>(*result), arg1_expected);
            BOOST_CHECK_EQUAL(std::get<decltype(of_expected)>(std::get<1>(*result)), of_expected);
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(fail_message, e.what());
            BOOST_CHECK(!result);
        }
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{std::vector{"foo", "-f"}, 42, true, ""},
                       std::tuple{std::vector{"foo", "--arg1", "13", "-f"}, 13, true, ""},
                       std::tuple{std::vector{"foo", "--arg3", "hello"}, 42, "hello"sv, ""},
                       std::tuple{std::vector{"foo"},
                                  42,
                                  true,
                                  "Missing required argument: One of: -f,--arg2,--arg3"},
                   });
}

BOOST_AUTO_TEST_CASE(one_of_default_value_test)
{
    auto f = [&](auto args, auto arg1_expected, auto of_expected, std::string fail_message) {
        auto result = std::optional<std::tuple<int, std::variant<bool, int, std::string_view>>>{};

        const auto r =
            root(mode(arg<int>(policy::long_name<AR_STRING("arg1")>, policy::default_value{42}),
                      ard::one_of(flag(policy::short_name<'f'>),
                                  arg<int>(policy::long_name<AR_STRING("arg2")>),
                                  arg<std::string_view>(policy::long_name<AR_STRING("arg3")>),
                                  policy::default_value{"goodbye"sv}),
                      policy::router{[&](int arg1, std::variant<bool, int, std::string_view> of) {
                          result = std::tuple{arg1, std::move(of)};
                      }}),
                 policy::validation::default_validator);

        try {
            r.parse(args.size(), const_cast<char**>(args.data()));
            BOOST_CHECK(fail_message.empty());
            BOOST_REQUIRE(!!result);

            BOOST_CHECK_EQUAL(std::get<0>(*result), arg1_expected);
            BOOST_CHECK_EQUAL(std::get<decltype(of_expected)>(std::get<1>(*result)), of_expected);
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(fail_message, e.what());
            BOOST_CHECK(!result);
        }
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{std::vector{"foo", "-f"}, 42, true, ""},
                       std::tuple{std::vector{"foo", "--arg1", "13", "-f"}, 13, true, ""},
                       std::tuple{std::vector{"foo", "--arg3", "hello"}, 42, "hello"sv, ""},
                       std::tuple{std::vector{"foo"}, 42, "goodbye"sv, ""},
                   });
}

BOOST_AUTO_TEST_CASE(counting_flag_test)
{
    auto result = std::optional<  //
        std::variant<std::tuple<double, bool, bool, std::size_t>,
                     std::tuple<std::size_t>,
                     std::tuple<bool, std::size_t>>>{};
    auto r = root(
        mode(policy::none_name<AR_STRING("mode1")>,
             arg<double>(policy::long_name<AR_STRING("arg1")>, policy::default_value{3.14}),
             flag(policy::short_name<'a'>),
             flag(policy::short_name<'b'>),
             counting_flag<std::size_t>(policy::short_name<'c'>),
             policy::router{[&](double arg1, bool a, bool b, std::size_t c) {
                 result = std::tuple{arg1, a, b, c};
             }}),
        mode(policy::none_name<AR_STRING("mode2")>,
             counting_flag<std::size_t>(policy::short_name<'a'>,
                                        policy::alias(policy::short_name<'b'>)),
             counting_flag<std::size_t>(policy::short_name<'b'>, policy::min_max_value<2u, 5u>()),
             policy::router{[&](std::size_t b) { result = std::tuple{b}; }}),
        mode(policy::none_name<AR_STRING("mode3")>,
             flag(policy::short_name<'a'>),
             counting_flag<std::size_t>(policy::short_name<'b'>,
                                        policy::dependent(policy::short_name<'a'>)),
             policy::router{[&](bool a, std::size_t b) {
                 result = std::tuple{a, b};
             }}),
        policy::validation::default_validator);

    auto f = [&](auto args, auto expected_result, std::string fail_message) {
        result.reset();

        try {
            r.parse(args.size(), const_cast<char**>(args.data()));
            BOOST_CHECK(fail_message.empty());

            BOOST_REQUIRE(!!result);
            BOOST_CHECK(std::get<std::decay_t<decltype(expected_result)>>(*result) ==
                        expected_result);
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(fail_message, e.what());
            BOOST_CHECK(!result);
        }
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::vector{"foo", "mode1"},
                       std::tuple{3.14, false, false, std::size_t{0}},
                       ""},
            std::tuple{std::vector{"foo", "mode1", "-c"},
                       std::tuple{3.14, false, false, std::size_t{1}},
                       ""},
            std::tuple{std::vector{"foo", "mode1", "-c", "-a", "-c", "-b", "-c", "-c"},
                       std::tuple{3.14, true, true, std::size_t{4}},
                       ""},
            std::tuple{std::vector{"foo", "mode1", "-ccc"},
                       std::tuple{3.14, false, false, std::size_t{3}},
                       ""},
            std::tuple{std::vector{"foo", "mode1", "-cacbcc"},
                       std::tuple{3.14, true, true, std::size_t{4}},
                       ""},
            std::tuple{std::vector{"foo", "mode1", "-c", "--arg1", "9.2", "-bcc"},
                       std::tuple{9.2, false, true, std::size_t{3}},
                       ""},
            std::tuple{std::vector{"foo", "mode2", "-aba"}, std::tuple{std::size_t{3}}, ""},
            std::tuple{std::vector{"foo", "mode2", "-b"},
                       std::tuple{std::size_t{1}},
                       "Minimum value not reached: -b"},
            std::tuple{std::vector{"foo", "mode2", "-abababab"},
                       std::tuple{std::size_t{8}},
                       "Maximum value exceeded: -b"},
            std::tuple{std::vector{"foo", "mode3", "-bbab"},
                       std::tuple{true, std::size_t{3}},
                       "Dependent argument missing (needs to be before the "
                       "requiring token on the command line): -a"},
            std::tuple{std::vector{"foo", "mode3", "-abbb"}, std::tuple{true, std::size_t{3}}, ""},
            std::tuple{std::vector{"foo", "mode3", "-a"}, std::tuple{true, std::size_t{0}}, ""},
            std::tuple{std::vector{"foo", "mode3", "-bbb"},
                       std::tuple{false, std::size_t{0}},
                       "Dependent argument missing (needs to be before the "
                       "requiring token on the command line): -a"},
        });
}

BOOST_AUTO_TEST_SUITE_END()
