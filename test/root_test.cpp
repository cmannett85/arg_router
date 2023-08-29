// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/literals.hpp"
#include "arg_router/policy/dependent.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/min_max_value.hpp"
#include "arg_router/policy/runtime_enable.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/policy/value_separator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

#include <bitset>
#include <optional>

using namespace arg_router;
using namespace arg_router::literals;
namespace ard = arg_router::dependency;
using namespace std::string_literals;
using namespace std::string_view_literals;

namespace
{
using default_validator_type = std::decay_t<decltype(policy::validation::default_validator)>;
}  // namespace

BOOST_AUTO_TEST_SUITE(root_suite)

BOOST_AUTO_TEST_CASE(alias_flag_parse_test)
{
    auto router_hit = false;
    auto result = std::array<bool, 3>{};
    const auto r = root(
        mode(flag(policy::long_name_t{"flag1"_S}, policy::description_t{"First description"_S}),
             flag(policy::long_name_t{"flag2"_S}, policy::description_t{"Second description"_S}),
             flag(policy::long_name_t{"flag3"_S}, policy::description_t{"Third description"_S}),
             flag(policy::short_name_t{"a"_S},
                  policy::alias(policy::long_name_t{"flag1"_S}, policy::long_name_t{"flag3"_S}),
                  policy::description_t{"Alias description"_S}),
             policy::router{[&](bool flag1, bool flag2, bool flag3) {
                 result = {flag1, flag2, flag3};
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
    const auto r = root(
        mode(arg<bool>(policy::long_name_t{"arg1"_S},
                       policy::required,
                       policy::description_t{"First description"_S}),
             arg<int>(policy::long_name_t{"arg2"_S},
                      policy::default_value(42),
                      policy::description_t{"Second description"_S}),
             arg<int>(policy::long_name_t{"arg3"_S},
                      policy::value_separator_t{"="_S},
                      policy::default_value(84),
                      policy::description_t{"Third description"_S}),
             arg<int>(policy::short_name_t{"a"_S},
                      policy::alias(policy::long_name_t{"arg2"_S}, policy::long_name_t{"arg3"_S}),
                      policy::description_t{"Alias description"_S}),
             policy::router{[&](bool arg1, int arg2, int arg3) {
                 result = decltype(result){arg1, arg2, arg3};
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
        root(flag(policy::long_name_t{"top-flag"_S},
                  policy::description_t{"Description"_S},
                  policy::router{[&](bool v) {
                      router_hit[0] = true;
                      result = std::tuple{v};
                  }}),
             arg<int>(policy::long_name_t{"top-arg"_S},
                      policy::description_t{"Description"_S},
                      policy::router{[&](int v) {
                          router_hit[1] = true;
                          result = std::tuple{v};
                      }}),
             mode(policy::none_name_t{"mode1"_S},
                  flag(policy::long_name_t{"flag1"_S}, policy::description_t{"Description"_S}),
                  arg<double>(policy::long_name_t{"arg1"_S},
                              policy::description_t{"Description"_S},
                              policy::default_value{3.14}),
                  flag(policy::long_name_t{"flag2"_S},
                       policy::short_name_t{"t"_S},
                       policy::description_t{"Description"_S}),
                  policy::router{[&](bool f1, double a1, bool f2) {
                      router_hit[2] = true;
                      result = std::tuple{f1, a1, f2};
                  }},
                  mode(policy::none_name_t{"mode2"_S},
                       arg<int>(policy::long_name_t{"arg1"_S},
                                policy::description_t{"Description"_S},
                                policy::required),
                       flag(policy::long_name_t{"flag1"_S},
                            policy::short_name_t{"b"_S},
                            policy::description_t{"Description"_S}),
                       positional_arg<std::vector<std::string_view>>(
                           policy::display_name_t{"pos_args"_S},
                           policy::description_t{"Description"_S}),
                       policy::router{[&](int a1, bool f1, std::vector<std::string_view> pos_args) {
                           router_hit[3] = true;
                           result = std::tuple{a1, f1, std::move(pos_args)};
                       }}),
                  mode(policy::none_name_t{"mode3"_S},
                       flag(policy::long_name_t{"flag1"_S}, policy::description_t{"Description"_S}),
                       flag(policy::long_name_t{"flag2"_S},
                            policy::short_name_t{"b"_S},
                            policy::description_t{"Description"_S}),
                       policy::router{[&](bool f1, bool f2) {
                           router_hit[4] = true;
                           result = std::tuple{f1, f2};
                       }})),
             mode(flag(policy::long_name_t{"flag1"_S}, policy::description_t{"Description"_S}),
                  arg<double>(policy::long_name_t{"arg1"_S},
                              policy::default_value{4.2},
                              policy::description_t{"Description"_S}),
                  policy::router{[&](bool f1, double a1) {
                      router_hit[5] = true;
                      result = std::tuple{f1, a1};
                  }}),
             policy::validation::default_validator);

    auto f = [&](auto args, auto expected_index, auto expected, std::string fail_message) {
        result = std::tuple{false};
        router_hit.reset();

        try {
            r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
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
            std::tuple{std::vector{"foo", "--top-arg", "-42"}, 1, std::tuple{-42}, ""},
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
                       "Unknown argument: --foo2. Did you mean --flag1?"},
            std::tuple{std::vector{"foo", "mode1", "--foo2"},
                       0,
                       std::tuple{true},
                       "Unknown argument: --foo2. Did you mean --flag2?"},
        });
}

BOOST_AUTO_TEST_CASE(one_of_required_test)
{
    auto f = [&](auto args, auto arg1_expected, auto of_expected, std::string fail_message) {
        auto result = std::optional<std::tuple<int, std::variant<bool, int, std::string_view>>>{};

        const auto r =
            root(mode(arg<int>(policy::long_name_t{"arg1"_S}, policy::default_value{42}),
                      ard::one_of(flag(policy::short_name_t{"f"_S}),
                                  arg<int>(policy::long_name_t{"arg2"_S}),
                                  arg<std::string_view>(policy::long_name_t{"arg3"_S}),
                                  policy::required),
                      policy::router{[&](int arg1, std::variant<bool, int, std::string_view> of) {
                          result = std::tuple{arg1, std::move(of)};
                      }}),
                 policy::validation::default_validator);

        try {
            r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
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
            root(mode(arg<int>(policy::long_name_t{"arg1"_S}, policy::default_value{42}),
                      ard::one_of(flag(policy::short_name_t{"f"_S}),
                                  arg<int>(policy::long_name_t{"arg2"_S}),
                                  arg<std::string_view>(policy::long_name_t{"arg3"_S}),
                                  policy::default_value{"goodbye"sv}),
                      policy::router{[&](int arg1, std::variant<bool, int, std::string_view> of) {
                          result = std::tuple{arg1, std::move(of)};
                      }}),
                 policy::validation::default_validator);

        try {
            r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
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
    auto r = root(mode(policy::none_name_t{"mode1"_S},
                       arg<double>(policy::long_name_t{"arg1"_S}, policy::default_value{3.14}),
                       flag(policy::short_name_t{"a"_S}),
                       flag(policy::short_name_t{"b"_S}),
                       counting_flag<std::size_t>(policy::short_name_t{"c"_S}),
                       policy::router{[&](double arg1, bool a, bool b, std::size_t c) {
                           result = std::tuple{arg1, a, b, c};
                       }}),
                  mode(policy::none_name_t{"mode2"_S},
                       counting_flag<std::size_t>(policy::short_name_t{"a"_S},
                                                  policy::alias(policy::short_name_t{"b"_S})),
                       counting_flag<std::size_t>(policy::short_name_t{"b"_S},
                                                  policy::min_max_value<2u, 5u>()),
                       policy::router{[&](std::size_t b) { result = std::tuple{b}; }}),
                  mode(policy::none_name_t{"mode3"_S},
                       flag(policy::short_name_t{"a"_S}),
                       counting_flag<std::size_t>(policy::short_name_t{"b"_S},
                                                  policy::dependent(policy::short_name_t{"a"_S})),
                       policy::router{[&](bool a, std::size_t b) {
                           result = std::tuple{a, b};
                       }}),
                  policy::validation::default_validator);

    auto f = [&](auto args, auto expected_result, std::string fail_message) {
        result.reset();

        try {
            r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
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

BOOST_AUTO_TEST_CASE(parse_overloads_test)
{
    auto router_hit = false;
    auto result = std::array<bool, 3>{};
    const auto r = root(
        mode(flag(policy::long_name_t{"flag1"_S}, policy::description_t{"First description"_S}),
             flag(policy::long_name_t{"flag2"_S}, policy::description_t{"Second description"_S}),
             flag(policy::short_name_t{"t"_S}, policy::description_t{"Third description"_S}),
             policy::router{[&](bool flag1, bool flag2, bool t) {
                 result = {flag1, flag2, t};
                 router_hit = true;
             }}),
        policy::validation::default_validator);

    const auto parse_invocations =
        std::vector<std::pair<std::string_view, std::function<void(std::vector<const char*>)>>>{
            {"vector<parsing::token_type> overload",
             [&](std::vector<const char*> args) {
                 args.erase(args.begin());
                 auto tt = std::vector<parsing::token_type>{};
                 for (auto arg : args) {
                     tt.emplace_back(parsing::prefix_type::none, arg);
                 }
                 r.parse(std::move(tt));
             }},
            {"int, char** overload",
             [&](std::vector<const char*> args) {
                 r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
             }},
            {"Iter, Iter overload",
             [&](std::vector<const char*> args) {  //
                 args.erase(args.begin());
                 r.parse(args.begin(), args.end());
             }},
            {"Container overload",
             [&](std::vector<const char*> args) {  //
                 args.erase(args.begin());
                 r.parse(args);
             }},
            {"Container overload (with strings)",  //
             [&](std::vector<const char*> args) {
                 args.erase(args.begin());
                 auto strings = std::vector<std::string>{};
                 for (auto arg : args) {
                     strings.push_back(arg);
                 }
                 r.parse(std::move(strings));
             }}};

    auto f = [&](auto args, auto expected, std::string fail_message) {
        result.fill(false);
        router_hit = false;

        for (const auto& [name, invoc] : parse_invocations) {
            BOOST_TEST_MESSAGE("\t" << name);
            try {
                invoc(args);
                BOOST_CHECK(fail_message.empty());
                BOOST_CHECK(router_hit);
                BOOST_CHECK_EQUAL(result[0], expected[0]);
                BOOST_CHECK_EQUAL(result[1], expected[1]);
                BOOST_CHECK_EQUAL(result[2], expected[2]);
            } catch (parse_exception& e) {
                BOOST_CHECK_EQUAL(fail_message, e.what());
                BOOST_CHECK(!router_hit);
            }
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
            std::tuple{std::vector{"foo"}, std::array{false, false, false}, ""},
        });
}

BOOST_AUTO_TEST_CASE(runtime_enable_test)
{
    auto f =
        [&](auto args, auto expected, auto enable_flag, auto enable_arg, std::string fail_message) {
            auto router_hit = false;
            auto result = std::tuple{false, false, 0};
            const auto r = root(mode(flag(policy::long_name_t{"flag1"_S},
                                          policy::description_t{"First description"_S},
                                          policy::runtime_enable{enable_flag}),
                                     flag(policy::long_name_t{"flag2"_S},
                                          policy::description_t{"Second description"_S}),
                                     arg<int>(policy::long_name_t{"arg"_S},
                                              policy::description_t{"Arg description"_S},
                                              policy::runtime_enable_required<int>{enable_arg, 42}),
                                     policy::router{[&](bool flag1, bool flag2, int arg) {
                                         result = {flag1, flag2, arg};
                                         router_hit = true;
                                     }}),
                                policy::validation::default_validator);

            try {
                r.parse(args);
                BOOST_CHECK(fail_message.empty());
                BOOST_CHECK(router_hit);
                BOOST_CHECK_EQUAL(std::get<0>(result), std::get<0>(expected));
                BOOST_CHECK_EQUAL(std::get<1>(result), std::get<1>(expected));
            } catch (parse_exception& e) {
                BOOST_CHECK_EQUAL(fail_message, e.what());
                BOOST_CHECK(!router_hit);
            }
        };

    test::data_set(
        f,
        {
            std::tuple{std::vector{"--flag1"},
                       std::tuple{true, false, 42},
                       true,
                       true,
                       "Missing required argument: --arg"},
            std::tuple{std::vector{"--flag1", "--arg", "32"},
                       std::tuple{true, false, 42},
                       true,
                       true,
                       ""},
            std::tuple{std::vector{"--flag2", "--arg", "32"},
                       std::tuple{false, true, 42},
                       true,
                       true,
                       ""},
            std::tuple{std::vector{"--arg", "32"}, std::tuple{false, false, 32}, true, true, ""},
            std::tuple{std::vector{"--flag1"},
                       std::tuple{false, false, 0},
                       false,
                       true,
                       "Unknown argument: --flag1. Did you mean --flag2?"},
            std::tuple{std::vector{"--arg", "32"},
                       std::tuple{false, false, 0},
                       true,
                       false,
                       "Unknown argument: --arg. Did you mean --flag1?"},
            std::tuple{std::vector{"--flag2"}, std::tuple{false, true, 42}, false, false, ""},
        });
}

BOOST_AUTO_TEST_CASE(runtime_enable_mode_parse_test)
{
    auto f = [&](auto args,
                 auto router_index,
                 auto expected,
                 auto enable_mode1,
                 auto enable_mode2,
                 std::string fail_message) {
        auto router_hit1 = false;
        auto router_hit2 = false;
        auto result1 = std::array<bool, 3>{};
        auto result2 = std::array<bool, 2>{};

        const auto r = root(
            mode(
                policy::none_name_t{"mode1"_S},
                flag(policy::long_name_t{"flag1"_S}, policy::description_t{"First description"_S}),
                flag(policy::long_name_t{"flag2"_S}, policy::description_t{"Second description"_S}),
                flag(policy::short_name_t{"t"_S}, policy::description_t{"Third description"_S}),
                policy::runtime_enable{enable_mode1},
                policy::router{[&](bool flag1, bool flag2, bool t) {
                    result1 = {flag1, flag2, t};
                    router_hit1 = true;
                }}),
            mode(policy::none_name_t{"mode2"_S},
                 flag(policy::long_name_t{"flag1"_S},
                      policy::description_t{"Other third description"_S}),
                 flag(policy::short_name_t{"b"_S}, policy::description_t{"Fourth description"_S}),
                 policy::runtime_enable{enable_mode2},
                 policy::router{[&](bool flag1, bool b) {
                     result2 = {flag1, b};
                     router_hit2 = true;
                 }}),
            policy::validation::default_validator);

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

    test::data_set(f,
                   {
                       std::tuple{std::vector{"foo", "mode1", "--flag1"},
                                  0,
                                  std::vector{true, false, false},
                                  true,
                                  true,
                                  ""},
                       std::tuple{std::vector{"foo", "mode2", "--flag1"},
                                  1,
                                  std::vector{true, false},
                                  true,
                                  true,
                                  ""},
                       std::tuple{std::vector{"foo", "mode1", "mode2", "--flag1"},
                                  0,
                                  std::vector{false, false, false},
                                  true,
                                  true,
                                  "Unknown argument: mode2. Did you mean --flag2?"},
                       std::tuple{std::vector{"foo", "mode2", "-b"},
                                  1,
                                  std::vector{false, true},
                                  true,
                                  true,
                                  ""},
                       std::tuple{std::vector{"foo", "mode1", "--flag1"},
                                  0,
                                  std::vector{true, false, false},
                                  true,
                                  false,
                                  ""},
                       std::tuple{std::vector{"foo", "mode1", "--flag1"},
                                  0,
                                  std::vector{true, false, false},
                                  false,
                                  true,
                                  "Unknown argument: mode1. Did you mean mode2?"},
                       std::tuple{std::vector{"foo", "mode2", "--flag1"},
                                  0,
                                  std::vector{true, false, false},
                                  true,
                                  false,
                                  "Unknown argument: mode2. Did you mean mode1?"},
                       std::tuple{std::vector{"foo", "mode1", "--flag1"},
                                  0,
                                  std::vector{true, false, false},
                                  false,
                                  false,
                                  "Unknown argument: mode1"},
                   });
}

BOOST_AUTO_TEST_SUITE_END()
