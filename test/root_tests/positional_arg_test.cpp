// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/literals.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/token_end_marker.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace arg_router::literals;
namespace ard = arg_router::dependency;

BOOST_AUTO_TEST_SUITE(root_suite)

BOOST_AUTO_TEST_SUITE(positional_arg_suite)

BOOST_AUTO_TEST_CASE(single_positional_arg_parse_test)
{
    auto router_hit = false;
    auto result = std::tuple<bool, bool, int, std::vector<std::string_view>>{};
    const auto r =
        root(mode(flag(policy::long_name_t{"flag1"_S},
                       policy::short_name_t{"a"_S},
                       policy::description_t{"First description"_S}),
                  flag(policy::short_name_t{"b"_S}),
                  arg<int>(policy::long_name_t{"arg1"_S},
                           policy::default_value(42),
                           policy::description_t{"Second description"_S}),
                  positional_arg<std::vector<std::string_view>>(
                      policy::display_name_t{"pos_args"_S},
                      policy::description_t{"Third description"_S},
                      policy::required,
                      policy::min_count<2>),
                  policy::router{
                      [&](bool flag1, bool b, int arg1, std::vector<std::string_view> pos_args) {
                          result = decltype(result){flag1, b, arg1, pos_args};
                          router_hit = true;
                      }}),
             policy::validation::default_validator);

    auto f = [&](auto args, auto expected, std::string fail_message) {
        result = decltype(result){};
        router_hit = false;

        try {
            r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK(router_hit);
            BOOST_CHECK_EQUAL(std::get<0>(result), std::get<0>(expected));
            BOOST_CHECK_EQUAL(std::get<1>(result), std::get<1>(expected));
            BOOST_CHECK_EQUAL(std::get<2>(result), std::get<2>(expected));
            BOOST_CHECK(std::get<3>(result) == std::get<3>(expected));
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(fail_message, e.what());
            BOOST_CHECK(!router_hit);
        }
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector{"foo", "one", "two"},
                       std::tuple{false, false, 42, std::vector<std::string_view>{"one", "two"}},
                       ""},
            std::tuple{std::vector{"foo", "--one", "two"},
                       std::tuple{false, false, 42, std::vector<std::string_view>{"--one", "two"}},
                       ""},
            std::tuple{std::vector{"foo", "one", "--two"},
                       std::tuple{false, false, 42, std::vector<std::string_view>{"one", "--two"}},
                       ""},
            std::tuple{std::vector{"foo", "--flag1", "one", "two"},
                       std::tuple{true, false, 42, std::vector<std::string_view>{"one", "two"}},
                       ""},
            std::tuple{std::vector{"foo", "-a", "one", "two"},
                       std::tuple{true, false, 42, std::vector<std::string_view>{"one", "two"}},
                       ""},
            std::tuple{std::vector{"foo", "--flag1", "-b", "one", "two"},
                       std::tuple{true, true, 42, std::vector<std::string_view>{"one", "two"}},
                       ""},
            std::tuple{std::vector{"foo", "--arg1", "14", "one", "two"},
                       std::tuple{false, false, 14, std::vector<std::string_view>{"one", "two"}},
                       ""},
            std::tuple{std::vector{"foo", "--arg1", "14", "--flag1", "one", "two"},
                       std::tuple{true, false, 14, std::vector<std::string_view>{"one", "two"}},
                       ""},
            std::tuple{std::vector{"foo", "--arg1", "14", "--flag1", "one", "-two"},
                       std::tuple{true, false, 14, std::vector<std::string_view>{"one", "-two"}},
                       ""},
            std::tuple{std::vector{"foo", "-ab", "--one", "two"},
                       std::tuple{true, true, 42, std::vector<std::string_view>{"--one", "two"}},
                       ""},
            std::tuple{std::vector{"foo", "--flag1", "hello"},
                       std::tuple{true, false, 42, std::vector<std::string_view>{}},
                       "Minimum count not reached: pos_args"},
            std::tuple{std::vector{"foo", "--flag1", "--arg1", "9", "hello"},
                       std::tuple{true, false, 9, std::vector<std::string_view>{}},
                       "Minimum count not reached: pos_args"},
            std::tuple{std::vector{"foo", "--flag1"},
                       std::tuple{true, false, 42, std::vector<std::string_view>{}},
                       "Missing required argument: pos_args"},
        });
}

BOOST_AUTO_TEST_CASE(two_positional_arg_parse_test)
{
    auto router_hit = false;
    auto result = std::tuple<bool, int, std::vector<std::string_view>, std::vector<double>>{};
    const auto r = root(
        mode(flag(policy::long_name_t{"flag1"_S}, policy::description_t{"First description"_S}),
             arg<int>(policy::long_name_t{"arg1"_S},
                      policy::default_value(42),
                      policy::description_t{"Second description"_S}),
             positional_arg<std::vector<std::string_view>>(
                 policy::display_name_t{"pos_args1"_S},
                 policy::description_t{"Third description"_S},
                 policy::fixed_count<2>),
             positional_arg<std::vector<double>>(policy::display_name_t{"pos_args2"_S},
                                                 policy::description_t{"Fourth description"_S}),
             policy::router{[&](bool flag1,
                                int arg1,
                                std::vector<std::string_view> pos_args1,
                                std::vector<double> pos_args2) {
                 result = decltype(result){flag1, arg1, pos_args1, pos_args2};
                 router_hit = true;
             }}),
        policy::validation::default_validator);

    auto f = [&](auto args, auto expected, std::string fail_message) {
        result = decltype(result){};
        router_hit = false;

        try {
            r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK(router_hit);
            BOOST_CHECK_EQUAL(std::get<0>(result), std::get<0>(expected));
            BOOST_CHECK_EQUAL(std::get<1>(result), std::get<1>(expected));
            BOOST_CHECK(std::get<2>(result) == std::get<2>(expected));
            BOOST_CHECK(std::get<3>(result) == std::get<3>(expected));
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(fail_message, e.what());
            BOOST_CHECK(!router_hit);
        }
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector{"foo", "one", "two"},
                       std::tuple{false,
                                  42,
                                  std::vector<std::string_view>{"one", "two"},
                                  std::vector<double>{}},
                       ""},
            std::tuple{std::vector{"foo", "one", "two", "3.14"},
                       std::tuple{false,
                                  42,
                                  std::vector<std::string_view>{"one", "two"},
                                  std::vector<double>{3.14}},
                       ""},
            std::tuple{std::vector{"foo", "one", "two", "3.14", "443.34"},
                       std::tuple{false,
                                  42,
                                  std::vector<std::string_view>{"one", "two"},
                                  std::vector<double>{3.14, 443.34}},
                       ""},
            std::tuple{
                std::vector{"foo", "one", "two", "three"},
                std::tuple{false, 42, std::vector<std::string_view>{}, std::vector<double>{}},
                "Failed to parse: three"},
            std::tuple{
                std::vector{"foo", "one", "--flag1", "two", "--arg1", "5"},
                std::tuple{false, 42, std::vector<std::string_view>{}, std::vector<double>{}},
                "Failed to parse: two"},
        });
}

BOOST_AUTO_TEST_CASE(two_positional_token_end_marker_arg_parse_test)
{
    auto router_hit = false;
    auto result = std::tuple<bool, int, std::vector<std::string_view>, std::vector<double>>{};
    const auto r = root(
        mode(flag(policy::long_name_t{"flag1"_S}, policy::description_t{"First description"_S}),
             arg<int>(policy::long_name_t{"arg1"_S},
                      policy::default_value(42),
                      policy::description_t{"Second description"_S}),
             positional_arg<std::vector<std::string_view>>(
                 policy::display_name_t{"pos_args1"_S},
                 policy::description_t{"Third description"_S},
                 policy::token_end_marker_t{"--"_S}),
             positional_arg<std::vector<double>>(policy::display_name_t{"pos_args2"_S},
                                                 policy::description_t{"Fourth description"_S}),
             policy::router{[&](bool flag1,
                                int arg1,
                                std::vector<std::string_view> pos_args1,
                                std::vector<double> pos_args2) {
                 result = decltype(result){flag1, arg1, pos_args1, pos_args2};
                 router_hit = true;
             }}),
        policy::validation::default_validator);

    auto f = [&](auto args, auto expected, std::string fail_message) {
        result = decltype(result){};
        router_hit = false;

        try {
            r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK(router_hit);
            BOOST_CHECK_EQUAL(std::get<0>(result), std::get<0>(expected));
            BOOST_CHECK_EQUAL(std::get<1>(result), std::get<1>(expected));
            BOOST_CHECK(std::get<2>(result) == std::get<2>(expected));
            BOOST_CHECK(std::get<3>(result) == std::get<3>(expected));
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(fail_message, e.what());
            BOOST_CHECK(!router_hit);
        }
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector{"foo", "one", "two"},
                       std::tuple{false,
                                  42,
                                  std::vector<std::string_view>{"one", "two"},
                                  std::vector<double>{}},
                       ""},
            std::tuple{std::vector{"foo", "one", "two", "3.14"},
                       std::tuple{false,
                                  42,
                                  std::vector<std::string_view>{"one", "two", "3.14"},
                                  std::vector<double>{}},
                       ""},
            std::tuple{std::vector{"foo", "one", "two", "--", "3.14"},
                       std::tuple{false,
                                  42,
                                  std::vector<std::string_view>{"one", "two"},
                                  std::vector<double>{3.14}},
                       ""},
            std::tuple{std::vector{"foo", "--flag1", "one", "two", "--", "3.14"},
                       std::tuple{true,
                                  42,
                                  std::vector<std::string_view>{"one", "two"},
                                  std::vector<double>{3.14}},
                       ""},
            std::tuple{std::vector{"foo", "--arg1", "5", "one", "two", "--", "3.14"},
                       std::tuple{false,
                                  5,
                                  std::vector<std::string_view>{"one", "two"},
                                  std::vector<double>{3.14}},
                       ""},
            std::tuple{
                std::vector{"foo", "--arg1", "5", "--", "3.14"},
                std::tuple{false, 5, std::vector<std::string_view>{}, std::vector<double>{3.14}},
                ""},
            std::tuple{
                std::vector{"foo", "--", "3.14"},
                std::tuple{false, 42, std::vector<std::string_view>{}, std::vector<double>{3.14}},
                ""},
            std::tuple{
                std::vector{"foo", "--"},
                std::tuple{false, 42, std::vector<std::string_view>{}, std::vector<double>{}},
                ""},
            std::tuple{
                std::vector{"foo"},
                std::tuple{false, 42, std::vector<std::string_view>{}, std::vector<double>{}},
                ""},
            std::tuple{
                std::vector{"foo", "one", "two", "--", "3.14", "three"},
                std::tuple{false, 42, std::vector<std::string_view>{}, std::vector<double>{}},
                "Failed to parse: three"},
        });
}

BOOST_AUTO_TEST_CASE(three_positional_token_end_marker_arg_parse_test)
{
    auto router_hit = false;
    auto result = std::tuple<bool,
                             std::vector<std::string_view>,
                             std::vector<double>,
                             std::vector<std::string_view>>{};
    const auto r = root(
        mode(flag(policy::long_name_t{"flag1"_S}, policy::description_t{"First description"_S}),
             positional_arg<std::vector<std::string_view>>(
                 policy::display_name_t{"pos_args1"_S},
                 policy::description_t{"Third description"_S},
                 policy::token_end_marker_t{"--"_S}),
             positional_arg<std::vector<double>>(policy::display_name_t{"pos_args2"_S},
                                                 policy::description_t{"Fourth description"_S},
                                                 policy::token_end_marker_t{"--"_S}),
             positional_arg<std::vector<std::string_view>>(
                 policy::display_name_t{"pos_args1"_S},
                 policy::description_t{"Fifth description"_S}),
             policy::router{[&](bool flag1,
                                std::vector<std::string_view> pos_args1,
                                std::vector<double> pos_args2,
                                std::vector<std::string_view> pos_args3) {
                 result = decltype(result){flag1, pos_args1, pos_args2, pos_args3};
                 router_hit = true;
             }}),
        policy::validation::default_validator);

    auto f = [&](auto args, auto expected, std::string fail_message) {
        result = decltype(result){};
        router_hit = false;

        try {
            r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK(router_hit);
            BOOST_CHECK_EQUAL(std::get<0>(result), std::get<0>(expected));
            BOOST_CHECK_EQUAL(std::get<1>(result), std::get<1>(expected));
            BOOST_CHECK_EQUAL(std::get<2>(result), std::get<2>(expected));
            BOOST_CHECK_EQUAL(std::get<3>(result), std::get<3>(expected));
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(fail_message, e.what());
            BOOST_CHECK(!router_hit);
        }
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector{"foo", "one", "two"},
                       std::tuple{false,
                                  std::vector<std::string_view>{"one", "two"},
                                  std::vector<double>{},
                                  std::vector<std::string_view>{}},
                       ""},
            std::tuple{std::vector{"foo", "one", "two", "3.14"},
                       std::tuple{false,
                                  std::vector<std::string_view>{"one", "two", "3.14"},
                                  std::vector<double>{},
                                  std::vector<std::string_view>{}},
                       ""},
            std::tuple{std::vector{"foo", "one", "two", "--", "3.14"},
                       std::tuple{false,
                                  std::vector<std::string_view>{"one", "two"},
                                  std::vector<double>{3.14},
                                  std::vector<std::string_view>{}},
                       ""},
            std::tuple{std::vector{"foo", "one", "two", "--", "3.14", "three"},
                       std::tuple{false,
                                  std::vector<std::string_view>{},
                                  std::vector<double>{},
                                  std::vector<std::string_view>{}},
                       "Failed to parse: three"},
            std::tuple{std::vector{"foo", "one", "two", "--", "3.14", "5", "--"},
                       std::tuple{false,
                                  std::vector<std::string_view>{"one", "two"},
                                  std::vector<double>{3.14, 5.0},
                                  std::vector<std::string_view>{}},
                       ""},
            std::tuple{std::vector{"foo", "--flag1", "one", "two", "--", "3.14", "5", "--"},
                       std::tuple{true,
                                  std::vector<std::string_view>{"one", "two"},
                                  std::vector<double>{3.14, 5.0},
                                  std::vector<std::string_view>{}},
                       ""},
            std::tuple{std::vector{"foo", "one", "two", "--", "3.14", "5", "--", "three"},
                       std::tuple{false,
                                  std::vector<std::string_view>{"one", "two"},
                                  std::vector<double>{3.14, 5.0},
                                  std::vector<std::string_view>{"three"}},
                       ""},
            std::tuple{std::vector{"foo", "one", "two", "--", "--", "three"},
                       std::tuple{false,
                                  std::vector<std::string_view>{"one", "two"},
                                  std::vector<double>{},
                                  std::vector<std::string_view>{"three"}},
                       ""},
            std::tuple{std::vector{"foo", "one", "two", "--", "--"},
                       std::tuple{false,
                                  std::vector<std::string_view>{"one", "two"},
                                  std::vector<double>{},
                                  std::vector<std::string_view>{}},
                       ""},
            std::tuple{std::vector{"foo", "one", "two", "--"},
                       std::tuple{false,
                                  std::vector<std::string_view>{"one", "two"},
                                  std::vector<double>{},
                                  std::vector<std::string_view>{}},
                       ""},
            std::tuple{std::vector{"foo", "one", "two"},
                       std::tuple{false,
                                  std::vector<std::string_view>{"one", "two"},
                                  std::vector<double>{},
                                  std::vector<std::string_view>{}},
                       ""},
            std::tuple{std::vector{"foo"},
                       std::tuple{false,
                                  std::vector<std::string_view>{},
                                  std::vector<double>{},
                                  std::vector<std::string_view>{}},
                       ""},
        });
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
