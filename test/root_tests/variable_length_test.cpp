// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/forwarding_arg.hpp"
#include "arg_router/multi_arg.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/token_end_marker.hpp"
#include "arg_router/policy/validator.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace std::string_literals;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(root_suite)

BOOST_AUTO_TEST_CASE(variable_length_multi_arg_test)
{
    auto router_hit = false;
    auto result = std::tuple<bool, std::vector<int>>{};
    const auto r1 =
        root(mode(flag(policy::short_name<'f'>, policy::description<AR_STRING("flag description")>),
                  multi_arg<std::vector<int>>(policy::long_name<AR_STRING("arg")>,
                                              policy::min_max_count<1, 3>,
                                              policy::description<AR_STRING("description")>),
                  policy::router{[&](bool flag, std::vector<int> arg) {
                      result = {flag, std::move(arg)};
                      router_hit = true;
                  }}),
             policy::validation::default_validator);

    const auto r2 =
        root(mode(flag(policy::short_name<'f'>, policy::description<AR_STRING("flag description")>),
                  multi_arg<std::vector<int>>(policy::long_name<AR_STRING("arg")>,
                                              policy::min_max_count<1, 3>,
                                              policy::token_end_marker<AR_STRING("--")>,
                                              policy::description<AR_STRING("description")>),
                  policy::router{[&](bool flag, std::vector<int> arg) {
                      result = {flag, std::move(arg)};
                      router_hit = true;
                  }}),
             policy::validation::default_validator);

    auto f = [&](const auto& root, auto args, auto expected_value, std::string fail_message) {
        try {
            result = decltype(result){};
            router_hit = false;

            root.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK(router_hit);

            BOOST_CHECK_EQUAL(std::get<0>(result), std::get<0>(expected_value));
            BOOST_CHECK_EQUAL(std::get<1>(result), std::get<1>(expected_value));
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
        }
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{r1,
                                  std::vector{"foo", "--arg", "84"},
                                  std::tuple{false, std::vector<int>{84}},
                                  ""},
                       std::tuple{r1,
                                  std::vector{"foo", "--arg", "84", "42"},
                                  std::tuple{false, std::vector<int>{84, 42}},
                                  ""},
                       std::tuple{r1,
                                  std::vector{"foo", "-f", "--arg", "84", "42"},
                                  std::tuple{true, std::vector<int>{84, 42}},
                                  ""},
                       std::tuple{r1,
                                  std::vector{"foo", "--arg", "84", "42", "12", "4"},
                                  std::tuple{false, std::vector<int>{}},
                                  "Unknown argument: 4. Did you mean -f?"},
                       std::tuple{r1,
                                  std::vector{"foo", "--arg"},
                                  std::tuple{false, std::vector<int>{}},
                                  "Minimum count not reached: --arg"},
                       std::tuple{r2,
                                  std::vector{"foo", "--arg", "84", "42", "12", "4"},
                                  std::tuple{false, std::vector<int>{}},
                                  "Maximum count exceeded: --arg"},
                   });
}

BOOST_AUTO_TEST_CASE(variable_length_multi_arg_with_positional_arg_test)
{
    auto router_hit = false;
    auto result = std::tuple<bool, std::vector<int>, std::vector<std::string_view>>{};
    const auto r = root(
        mode(
            flag(policy::short_name<'f'>, policy::description<AR_STRING("flag description")>),
            multi_arg<std::vector<int>>(policy::long_name<AR_STRING("arg")>,
                                        policy::min_max_count<1, 3>,
                                        policy::token_end_marker<AR_STRING("--")>,
                                        policy::description<AR_STRING("description")>),
            positional_arg<std::vector<std::string_view>>(policy::display_name<AR_STRING("POS")>),
            policy::router{[&](bool flag, std::vector<int> arg, std::vector<std::string_view> pos) {
                result = {flag, std::move(arg), std::move(pos)};
                router_hit = true;
            }}),
        policy::validation::default_validator);

    auto f = [&](auto args, auto expected_value, std::string fail_message) {
        try {
            result = decltype(result){};
            router_hit = false;

            r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK(router_hit);

            BOOST_CHECK_EQUAL(std::get<0>(result), std::get<0>(expected_value));
            BOOST_CHECK_EQUAL(std::get<1>(result), std::get<1>(expected_value));
            BOOST_CHECK_EQUAL(std::get<2>(result), std::get<2>(expected_value));
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
        }
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::vector{"foo", "--arg", "84"},
                       std::tuple{false, std::vector<int>{84}, std::vector<std::string_view>{}},
                       ""},
            std::tuple{std::vector{"foo", "--arg", "84", "42"},
                       std::tuple{false, std::vector<int>{84, 42}, std::vector<std::string_view>{}},
                       ""},
            std::tuple{std::vector{"foo", "--arg", "84", "42", "--"},
                       std::tuple{false, std::vector<int>{84, 42}, std::vector<std::string_view>{}},
                       ""},
            std::tuple{std::vector{"foo", "--arg", "84", "42", "--", "hello", "world"},
                       std::tuple{false,
                                  std::vector<int>{84, 42},
                                  std::vector<std::string_view>{"hello", "world"}},
                       ""},
            std::tuple{std::vector{"foo", "-f", "--arg", "84", "42", "--", "hello", "world"},
                       std::tuple{true,
                                  std::vector<int>{84, 42},
                                  std::vector<std::string_view>{"hello", "world"}},
                       ""},
            std::tuple{std::vector{"foo", "-f", "--arg", "84", "42", "12", "4"},
                       std::tuple{false, std::vector<int>{}, std::vector<std::string_view>{}},
                       "Maximum count exceeded: --arg"},
        });
}

BOOST_AUTO_TEST_CASE(variable_length_forwarding_arg_test)
{
    auto router_hit = false;
    auto result = std::tuple<bool, std::vector<std::string_view>>{};
    const auto r1 =
        root(mode(flag(policy::short_name<'f'>, policy::description<AR_STRING("flag description")>),
                  forwarding_arg(policy::none_name<AR_STRING("--")>,
                                 policy::max_count<3>,
                                 policy::description<AR_STRING("description")>),
                  policy::router{[&](bool flag, std::vector<std::string_view> arg) {
                      result = {flag, std::move(arg)};
                      router_hit = true;
                  }}),
             policy::validation::default_validator);

    const auto r2 =
        root(mode(flag(policy::short_name<'f'>, policy::description<AR_STRING("flag description")>),
                  forwarding_arg(policy::none_name<AR_STRING("--")>,
                                 policy::max_count<3>,
                                 policy::token_end_marker<AR_STRING("--")>,
                                 policy::description<AR_STRING("description")>),
                  policy::router{[&](bool flag, std::vector<std::string_view> arg) {
                      result = {flag, std::move(arg)};
                      router_hit = true;
                  }}),
             policy::validation::default_validator);

    auto f = [&](const auto& root, auto args, auto expected_value, std::string fail_message) {
        try {
            result = decltype(result){};
            router_hit = false;

            root.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK(router_hit);

            BOOST_CHECK_EQUAL(std::get<0>(result), std::get<0>(expected_value));
            BOOST_CHECK_EQUAL(std::get<1>(result), std::get<1>(expected_value));
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
        }
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{r1,
                       std::vector{"foo", "--", "hello"},
                       std::tuple{false, std::vector<std::string_view>{"hello"}},
                       ""},
            std::tuple{r1,
                       std::vector{"foo", "--", "hello", "world"},
                       std::tuple{false, std::vector<std::string_view>{"hello", "world"}},
                       ""},
            std::tuple{r1,
                       std::vector{"foo", "-f", "--", "hello", "world"},
                       std::tuple{true, std::vector<std::string_view>{"hello", "world"}},
                       ""},
            std::tuple{r1,
                       std::vector{"foo", "--", "hello", "world", "goodbye", "Cam"},
                       std::tuple{false, std::vector<std::string_view>{}},
                       "Unknown argument: Cam. Did you mean -f?"},
            std::tuple{r1,
                       std::vector{"foo", "--"},
                       std::tuple{false, std::vector<std::string_view>{}},
                       ""},
            std::tuple{r2,
                       std::vector{"foo", "--", "hello", "world", "goodbye", "Cam"},
                       std::tuple{false, std::vector<std::string_view>{}},
                       "Maximum count exceeded: --"},
        });
}

BOOST_AUTO_TEST_CASE(variable_length_forwarding_arg_with_positional_arg_test)
{
    auto router_hit = false;
    auto result = std::tuple<bool, std::vector<std::string_view>, std::vector<int>>{};
    const auto r = root(
        mode(
            flag(policy::short_name<'f'>, policy::description<AR_STRING("flag description")>),
            forwarding_arg(policy::none_name<AR_STRING("--")>,
                           policy::max_count<3>,
                           policy::token_end_marker<AR_STRING("--")>,
                           policy::description<AR_STRING("description")>),
            positional_arg<std::vector<int>>(policy::display_name<AR_STRING("POS")>),
            policy::router{[&](bool flag, std::vector<std::string_view> arg, std::vector<int> pos) {
                result = {flag, std::move(arg), std::move(pos)};
                router_hit = true;
            }}),
        policy::validation::default_validator);

    auto f = [&](auto args, auto expected_value, std::string fail_message) {
        try {
            result = decltype(result){};
            router_hit = false;

            r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK(router_hit);

            BOOST_CHECK_EQUAL(std::get<0>(result), std::get<0>(expected_value));
            BOOST_CHECK_EQUAL(std::get<1>(result), std::get<1>(expected_value));
            BOOST_CHECK_EQUAL(std::get<2>(result), std::get<2>(expected_value));
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
        }
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{
                std::vector{"foo", "--", "hello"},
                std::tuple{false, std::vector<std::string_view>{"hello"}, std::vector<int>{}},
                ""},
            std::tuple{std::vector{"foo", "--", "hello", "world"},
                       std::tuple{false,
                                  std::vector<std::string_view>{"hello", "world"},
                                  std::vector<int>{}},
                       ""},
            std::tuple{std::vector{"foo", "--", "hello", "world", "--"},
                       std::tuple{false,
                                  std::vector<std::string_view>{"hello", "world"},
                                  std::vector<int>{}},
                       ""},
            std::tuple{std::vector{"foo", "--", "hello", "world", "--", "84", "42"},
                       std::tuple{false,
                                  std::vector<std::string_view>{"hello", "world"},
                                  std::vector<int>{84, 42}},
                       ""},
            std::tuple{std::vector{"foo", "-f", "--", "hello", "world", "--", "84", "42"},
                       std::tuple{true,
                                  std::vector<std::string_view>{"hello", "world"},
                                  std::vector<int>{84, 42}},
                       ""},
            std::tuple{std::vector{"foo", "-f", "--", "hello", "world", "goodbye", "me"},
                       std::tuple{false, std::vector<std::string_view>{}, std::vector<int>{}},
                       "Maximum count exceeded: --"},
        });
}

BOOST_AUTO_TEST_SUITE_END()