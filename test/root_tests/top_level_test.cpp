// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/literals.hpp"
#include "arg_router/policy/custom_parser.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/flatten_help.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/policy/value_separator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
namespace ard = arg_router::dependency;
using namespace arg_router::literals;
using namespace std::string_literals;
using namespace std::string_view_literals;

namespace
{
using default_validator_type = std::decay_t<decltype(policy::validation::default_validator)>;

struct A {
    explicit A(int v = 0) : value{v} {}
    int value;
};

struct B {
    explicit B(double v = 0) : value{v} {}
    double value;
};
}  // namespace

template <>
struct arg_router::parser<B> {
    static B parse(std::string_view token) { return B{parser<double>::parse(token)}; }
};

BOOST_AUTO_TEST_SUITE(root_suite)

BOOST_AUTO_TEST_SUITE(top_level_suite)

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(
        is_tree_node_v<root_t<
            flag_t<policy::long_name_t<str<"hello">>, policy::router<std::function<void(bool)>>>,
            default_validator_type>>,
        "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(validator_type_test)
{
    static_assert(std::is_same_v<typename root_t<flag_t<policy::long_name_t<str<"hello">>,
                                                        policy::router<std::function<void(bool)>>>,
                                                 default_validator_type>::validator_type,
                                 default_validator_type>,
                  "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(constructor_validation_test)
{
    const auto r = root(policy::validation::default_validator,
                        flag(policy::long_name_t{"hello"_S},
                             policy::description_t{"This is a hello"_S},
                             policy::short_name_t{"h"_S},
                             policy::router{[]() {}}),
                        flag(policy::long_name_t{"goodbye"_S},
                             policy::description_t{"This is a goodbye flag"_S},
                             policy::short_name_t{"g"_S},
                             policy::router{[]() {}}));

    BOOST_CHECK_EQUAL(std::get<0>(r.children()).long_name(), "hello");
    BOOST_CHECK_EQUAL(std::get<1>(r.children()).long_name(), "goodbye");
}

BOOST_AUTO_TEST_CASE(unknown_argument_parse_test)
{
    auto router_hit = false;
    const auto r = root(flag(policy::long_name_t{"hello"_S},
                             policy::description_t{"Hello description"_S},
                             policy::router{[&](bool) { router_hit = true; }}),
                        policy::validation::default_validator);

    auto args = std::vector{"foo", "--foo"};
    BOOST_CHECK_EXCEPTION(r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data())),
                          parse_exception,
                          [](const auto& e) {
                              return e.what() == "Unknown argument: --foo. Did you mean --hello?"s;
                          });
    BOOST_CHECK(!router_hit);
}

BOOST_AUTO_TEST_CASE(unhandled_parse_test)
{
    auto router_hit = false;
    const auto r = root(flag(policy::long_name_t{"hello"_S},
                             policy::description_t{"Hello description"_S},
                             policy::router{[&](bool) { router_hit = true; }}),
                        policy::validation::default_validator);

    auto args = std::vector{"foo", "--hello", "--foo"};
    BOOST_CHECK_EXCEPTION(r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data())),
                          parse_exception,
                          [](const auto& e) { return e.what() == "Unhandled arguments: --foo"s; });
    BOOST_CHECK(!router_hit);
}

BOOST_AUTO_TEST_CASE(single_flag_parse_test)
{
    auto router_hit = false;
    const auto r = root(flag(policy::long_name_t{"こんにちは"_S},
                             policy::description_t{"こんにちは description"_S},
                             policy::router{[&](bool) { router_hit = true; }}),
                        policy::validation::default_validator);

    auto args = std::vector{"foo", "--こんにちは"};
    r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
    BOOST_CHECK(router_hit);
}

BOOST_AUTO_TEST_CASE(single_arg_parse_test)
{
    auto result = std::optional<int>{};
    const auto r = root(arg<int>(policy::long_name_t{"hello"_S},
                                 policy::description_t{"Hello description"_S},
                                 policy::router{[&](int value) {
                                     BOOST_CHECK(!result);
                                     result = value;
                                 }}),
                        policy::validation::default_validator);

    auto args = std::vector{"foo", "--hello", "42"};
    r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
    BOOST_REQUIRE(!!result);
    BOOST_CHECK_EQUAL(*result, 42);
}

BOOST_AUTO_TEST_CASE(single_arg_separator_parse_test)
{
    auto result = std::optional<int>{};
    const auto r = root(arg<int>(policy::long_name_t{"hello"_S},
                                 policy::description_t{"Hello description"_S},
                                 policy::value_separator_t{"="_S},
                                 policy::router{[&](int value) {
                                     BOOST_CHECK(!result);
                                     result = value;
                                 }}),
                        policy::validation::default_validator);

    auto f = [&](auto args, auto expected, std::string fail_message) {
        result.reset();

        try {
            r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
            BOOST_CHECK(fail_message.empty());
            BOOST_REQUIRE(!!result);
            BOOST_CHECK_EQUAL(*result, expected);
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
        }
    };

    test::data_set(f,
                   {
                       std::tuple{std::vector{"foo", "--hello=42"}, 42, ""},
                       std::tuple{std::vector{"foo", "--hello", "42"},
                                  0,
                                  "Expected a value separator: --hello"},
                       std::tuple{std::vector{"foo", "--hello="},
                                  0,
                                  "Unknown argument: --hello=. Did you mean --hello?"},
                   });
}

BOOST_AUTO_TEST_CASE(single_string_arg_parse_test)
{
    auto result = std::optional<std::string_view>{};
    const auto r = root(arg<std::string_view>(policy::long_name_t{"hello"_S},
                                              policy::description_t{"Hello description"_S},
                                              policy::router{[&](std::string_view value) {
                                                  BOOST_CHECK(!result);
                                                  result = value;
                                              }}),
                        policy::validation::default_validator);

    auto f = [&](auto args, auto expected) {
        result.reset();

        r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
        BOOST_REQUIRE(!!result);
        BOOST_CHECK_EQUAL(*result, expected);
    };

    test::data_set(f,
                   {
                       std::tuple{std::vector{"foo", "--hello", "hello"}, "hello"sv},
                       std::tuple{std::vector{"foo", "--hello", "-h"}, "-h"sv},
                       std::tuple{std::vector{"foo", "--hello", "-hello"}, "-hello"sv},
                       std::tuple{std::vector{"foo", "--hello", "--hello"}, "--hello"sv},
                   });
}

BOOST_AUTO_TEST_CASE(triple_flag_parse_test)
{
    auto result = std::array<bool, 3>{};
    const auto r = root(flag(policy::long_name_t{"flag1"_S},
                             policy::description_t{"First description"_S},
                             policy::router{[&](bool) { result[0] = true; }}),
                        flag(policy::long_name_t{"flag2"_S},
                             policy::description_t{"Second description"_S},
                             policy::router{[&](bool) { result[1] = true; }}),
                        flag(policy::short_name_t{"t"_S},
                             policy::description_t{"Third description"_S},
                             policy::router{[&](bool) { result[2] = true; }}),
                        policy::validation::default_validator);

    auto f = [&](auto args, auto expected) {
        result.fill(false);

        r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
        BOOST_CHECK_EQUAL(result[0], expected[0]);
        BOOST_CHECK_EQUAL(result[1], expected[1]);
        BOOST_CHECK_EQUAL(result[2], expected[2]);
    };

    test::data_set(f,
                   {
                       std::tuple{std::vector{"foo", "--flag1"}, std::array{true, false, false}},
                       std::tuple{std::vector{"foo", "--flag2"}, std::array{false, true, false}},
                       std::tuple{std::vector{"foo", "-t"}, std::array{false, false, true}},
                   });
}

BOOST_AUTO_TEST_CASE(triple_arg_parse_test)
{
    auto result = std::tuple<int, double, std::string_view>{};
    auto hit = std::array<bool, 3>{};

    const auto r = root(arg<int>(policy::long_name_t{"flag1"_S},
                                 policy::description_t{"First description"_S},
                                 policy::router{[&](auto value) {
                                     std::get<0>(result) = value;
                                     hit[0] = true;
                                 }}),
                        arg<double>(policy::long_name_t{"flag2"_S},
                                    policy::description_t{"Second description"_S},
                                    policy::router{[&](auto value) {
                                        std::get<1>(result) = value;
                                        hit[1] = true;
                                    }}),
                        arg<std::string_view>(policy::short_name_t{"t"_S},
                                              policy::description_t{"Third description"_S},
                                              policy::router{[&](auto value) {
                                                  std::get<2>(result) = value;
                                                  hit[2] = true;
                                              }}),
                        policy::validation::default_validator);

    auto f = [&](auto args, auto expected_hit, auto expected_value) {
        result = decltype(result){};
        hit.fill(false);

        r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
        BOOST_CHECK_EQUAL(hit[0], expected_hit[0]);
        BOOST_CHECK_EQUAL(hit[1], expected_hit[1]);
        BOOST_CHECK_EQUAL(hit[2], expected_hit[2]);

        BOOST_CHECK_EQUAL(std::get<0>(result), std::get<0>(expected_value));
        BOOST_CHECK_EQUAL(std::get<1>(result), std::get<1>(expected_value));
        BOOST_CHECK_EQUAL(std::get<2>(result), std::get<2>(expected_value));
    };

    test::data_set(f,
                   {
                       std::tuple{std::vector{"foo", "--flag1", "42"},
                                  std::array{true, false, false},
                                  std::tuple{42, 0.0, ""sv}},
                       std::tuple{std::vector{"foo", "--flag2", "3.14"},
                                  std::array{false, true, false},
                                  std::tuple{0, 3.14, ""sv}},
                       std::tuple{std::vector{"foo", "-t", "hello"},
                                  std::array{false, false, true},
                                  std::tuple{0, 0.0, "hello"sv}},
                   });
}

BOOST_AUTO_TEST_CASE(custom_parser_test)
{
    auto result = std::tuple<A, B, B>{};
    auto parser_hit = false;

    const auto r = root(arg<A>(policy::long_name_t{"arg1"_S},
                               policy::description_t{"First description"_S},
                               policy::custom_parser<A>{
                                   [](auto token) -> A { return A{parser<int>::parse(token)}; }},
                               policy::router{[&](auto arg1) { std::get<0>(result) = arg1; }}),
                        arg<B>(policy::long_name_t{"arg2"_S},
                               policy::description_t{"Second description"_S},
                               policy::custom_parser<B>{[&](auto token) -> B {
                                   parser_hit = true;
                                   return B{parser<double>::parse(token)};
                               }},
                               policy::router{[&](auto arg2) { std::get<1>(result) = arg2; }}),
                        arg<B>(policy::long_name_t{"arg3"_S},
                               policy::description_t{"Third description"_S},
                               policy::router{[&](auto arg3) { std::get<2>(result) = arg3; }}),
                        policy::validation::default_validator);

    auto f = [&](auto args, auto expected_hit, auto expected_value) {
        result = decltype(result){A{}, B{}, B{}};
        parser_hit = false;

        r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
        BOOST_CHECK_EQUAL(parser_hit, expected_hit);

        BOOST_CHECK_EQUAL(std::get<0>(result).value, std::get<0>(expected_value).value);
        BOOST_CHECK_EQUAL(std::get<1>(result).value, std::get<1>(expected_value).value);
        BOOST_CHECK_EQUAL(std::get<2>(result).value, std::get<2>(expected_value).value);
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector{"foo", "--arg1", "42"}, false, std::tuple{A{42}, B{}, B{}}},
            std::tuple{std::vector{"foo", "--arg2", "3.14"}, true, std::tuple{A{}, B{3.14}, B{}}},
            std::tuple{std::vector{"foo", "--arg3", "3.3"}, false, std::tuple{A{}, B{}, B{3.3}}},
        });
}

BOOST_AUTO_TEST_CASE(help_test)
{
    auto f = [](const auto& root, const auto& expected_result) {
        const auto result = root.help();
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{root(flag(policy::long_name_t{"flag1"_S},
                                 policy::short_name_t{"a"_S},
                                 policy::description_t{"Flag1 description"_S},
                                 policy::router{[](bool) {}}),
                            flag(policy::long_name_t{"flag2"_S}, policy::router{[](bool) {}}),
                            flag(policy::short_name_t{"b"_S},
                                 policy::description_t{"b description"_S},
                                 policy::router{[](bool) {}}),
                            policy::validation::default_validator),
                       ""s},
            std::tuple{root(flag(policy::long_name_t{"flag1"_S},
                                 policy::short_name_t{"a"_S},
                                 policy::description_t{"Flag1 description"_S},
                                 policy::router{[](bool) {}}),
                            flag(policy::long_name_t{"flag2"_S}, policy::router{[](bool) {}}),
                            flag(policy::short_name_t{"b"_S},
                                 policy::description_t{"b description"_S},
                                 policy::router{[](bool) {}}),
                            arg<int>(policy::long_name_t{"arg1"_S},
                                     policy::value_separator_t{"="_S},
                                     policy::router{[](int) {}}),
                            help(policy::long_name_t{"help"_S},
                                 policy::short_name_t{"h"_S},
                                 policy::description_t{"Help output"_S},
                                 policy::program_name_t{"foo"_S},
                                 policy::program_version_t{"v3.14"_S},
                                 policy::program_intro_t{"My foo is good for you"_S}),
                            policy::validation::default_validator),
                       R"(foo v3.14

My foo is good for you

    --flag1,-a        Flag1 description
    --flag2
    -b                b description
    --arg1=<Value>
    --help,-h         Help output
)"s},
            std::tuple{root(help(policy::long_name_t{"help"_S},
                                 policy::short_name_t{"h"_S},
                                 policy::description_t{"Help output"_S},
                                 policy::program_name_t{"foo"_S},
                                 policy::program_version_t{"v3.14"_S},
                                 policy::program_intro_t{"My foo is good for you"_S}),
                            mode(flag(policy::long_name_t{"flag1"_S},
                                      policy::short_name_t{"a"_S},
                                      policy::description_t{"Flag1 description"_S}),
                                 flag(policy::long_name_t{"flag2"_S}),
                                 arg<int>(policy::long_name_t{"arg1"_S},
                                          policy::description_t{"Arg1 description"_S}),
                                 flag(policy::short_name_t{"b"_S},
                                      policy::description_t{"b description"_S}),
                                 policy::router{[](bool, bool, bool) {}}),
                            policy::validation::default_validator),
                       R"(foo v3.14

My foo is good for you

    --help,-h             Help output
     
        --flag1,-a        Flag1 description
        --flag2
        --arg1 <Value>    Arg1 description
        -b                b description
)"s},
            std::tuple{root(help(policy::long_name_t{"help"_S},
                                 policy::short_name_t{"h"_S},
                                 policy::description_t{"Help output"_S},
                                 policy::program_name_t{"foo"_S},
                                 policy::program_version_t{"v3.14"_S},
                                 policy::program_intro_t{"My foo is good for you"_S}),
                            mode("test"_S,
                                 "Test mode"_S,
                                 flag(policy::long_name_t{"flag1"_S}),
                                 arg<int>(policy::long_name_t{"arg1"_S},
                                          policy::description_t{"Arg1 description"_S}),
                                 policy::router{[](bool, int) {}}),
                            mode(flag(policy::long_name_t{"flag1"_S},
                                      policy::short_name_t{"a"_S},
                                      policy::description_t{"Flag1 description"_S}),
                                 flag(policy::long_name_t{"flag2"_S}),
                                 arg<int>(policy::long_name_t{"arg1"_S},
                                          policy::description_t{"Arg1 description"_S}),
                                 flag(policy::short_name_t{"b"_S},
                                      policy::description_t{"b description"_S}),
                                 policy::router{[](bool, bool, bool) {}}),
                            policy::validation::default_validator),
                       R"(foo v3.14

My foo is good for you

    --help,-h             Help output
    test                  Test mode
     
        --flag1,-a        Flag1 description
        --flag2
        --arg1 <Value>    Arg1 description
        -b                b description
)"s},
            std::tuple{root(help(policy::long_name_t{"help"_S},
                                 policy::short_name_t{"h"_S},
                                 policy::flatten_help,
                                 policy::description_t{"Help output"_S},
                                 policy::program_name_t{"foo"_S},
                                 policy::program_version_t{"v3.14"_S},
                                 policy::program_intro_t{"My foo is good for you"_S}),
                            mode("test"_S,
                                 "Test mode"_S,
                                 flag(policy::long_name_t{"flag1"_S}),
                                 arg<int>(policy::long_name_t{"arg1"_S},
                                          policy::description_t{"Arg1 description"_S}),
                                 policy::router{[](bool, int) {}}),
                            mode(flag(policy::long_name_t{"flag1"_S},
                                      policy::short_name_t{"a"_S},
                                      policy::description_t{"Flag1 description"_S}),
                                 flag(policy::long_name_t{"flag2"_S}),
                                 arg<int>(policy::long_name_t{"arg1"_S},
                                          policy::description_t{"Arg1 description"_S}),
                                 flag(policy::short_name_t{"b"_S},
                                      policy::description_t{"b description"_S}),
                                 policy::router{[](bool, bool, bool) {}}),
                            policy::validation::default_validator),
                       R"(foo v3.14

My foo is good for you

    --help,-h             Help output
    test                  Test mode
        --flag1
        --arg1 <Value>    Arg1 description
     
        --flag1,-a        Flag1 description
        --flag2
        --arg1 <Value>    Arg1 description
        -b                b description
)"s}});
}

BOOST_AUTO_TEST_CASE(help_runtime_test)
{
    auto f = [](const auto& root, const auto& expected_result) {
        const auto result = root.help();
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{root(flag(policy::long_name_t{"flag1"_S},
                                 policy::short_name_t{"a"_S},
                                 policy::description_t{"Flag1 description"_S},
                                 policy::router{[](bool) {}}),
                            flag(policy::long_name_t{"flag2"_S}, policy::router{[](bool) {}}),
                            flag(policy::short_name_t{"b"_S},
                                 policy::description_t{"b description"_S},
                                 policy::runtime_enable{false},
                                 policy::router{[](bool) {}}),
                            policy::validation::default_validator),
                       ""s},
            std::tuple{root(flag(policy::long_name_t{"flag1"_S},
                                 policy::short_name_t{"a"_S},
                                 policy::description_t{"Flag1 description"_S},
                                 policy::router{[](bool) {}}),
                            flag(policy::long_name_t{"flag2"_S}, policy::router{[](bool) {}}),
                            flag(policy::short_name_t{"b"_S},
                                 policy::description_t{"b description"_S},
                                 policy::runtime_enable{true},
                                 policy::router{[](bool) {}}),
                            policy::validation::default_validator),
                       ""s},
            std::tuple{root(flag(policy::long_name_t{"flag1"_S},
                                 policy::short_name_t{"a"_S},
                                 policy::description_t{"Flag1 description"_S},
                                 policy::router{[](bool) {}}),
                            flag(policy::long_name_t{"flag2"_S}, policy::router{[](bool) {}}),
                            flag(policy::short_name_t{"b"_S},
                                 policy::description_t{"b description"_S},
                                 policy::runtime_enable{false},
                                 policy::router{[](bool) {}}),
                            arg<int>(policy::long_name_t{"arg1"_S},
                                     policy::value_separator_t{"="_S},
                                     policy::router{[](int) {}}),
                            help(policy::long_name_t{"help"_S},
                                 policy::short_name_t{"h"_S},
                                 policy::description_t{"Help output"_S},
                                 policy::program_name_t{"foo"_S},
                                 policy::program_version_t{"v3.14"_S},
                                 policy::program_intro_t{"My foo is good for you"_S}),
                            policy::validation::default_validator),
                       R"(foo v3.14

My foo is good for you

    --flag1,-a        Flag1 description
    --flag2
    --arg1=<Value>
    --help,-h         Help output
)"s},
            std::tuple{
                root(help(policy::long_name_t{"help"_S},
                          policy::short_name_t{"h"_S},
                          policy::description_t{"Help output"_S},
                          policy::program_name_t{"foo"_S},
                          policy::program_version_t{"v3.14"_S},
                          policy::program_intro_t{"My foo is good for you"_S}),
                     mode(flag(policy::long_name_t{"flag1"_S},
                               policy::short_name_t{"a"_S},
                               policy::description_t{"Flag1 description"_S}),
                          flag(policy::long_name_t{"flag2"_S}, policy::runtime_enable{false}),
                          arg<int>(policy::long_name_t{"arg1"_S},
                                   policy::description_t{"Arg1 description"_S}),
                          flag(policy::short_name_t{"b"_S},
                               policy::description_t{"b description"_S}),
                          policy::router{[](bool, bool, bool) {}}),
                     policy::validation::default_validator),
                R"(foo v3.14

My foo is good for you

    --help,-h             Help output
     
        --flag1,-a        Flag1 description
        --arg1 <Value>    Arg1 description
        -b                b description
)"s},
            std::tuple{root(help(policy::long_name_t{"help"_S},
                                 policy::short_name_t{"h"_S},
                                 policy::description_t{"Help output"_S},
                                 policy::program_name_t{"foo"_S},
                                 policy::program_version_t{"v3.14"_S},
                                 policy::program_intro_t{"My foo is good for you"_S}),
                            mode("test"_S,
                                 "Test mode"_S,
                                 policy::runtime_enable{false},
                                 flag(policy::long_name_t{"flag1"_S}),
                                 arg<int>(policy::long_name_t{"arg1"_S},
                                          policy::description_t{"Arg1 description"_S}),
                                 policy::router{[](bool, int) {}}),
                            mode(flag(policy::long_name_t{"flag1"_S},
                                      policy::short_name_t{"a"_S},
                                      policy::description_t{"Flag1 description"_S}),
                                 flag(policy::long_name_t{"flag2"_S}),
                                 arg<int>(policy::long_name_t{"arg1"_S},
                                          policy::description_t{"Arg1 description"_S}),
                                 flag(policy::short_name_t{"b"_S},
                                      policy::description_t{"b description"_S}),
                                 policy::router{[](bool, bool, bool) {}}),
                            policy::validation::default_validator),
                       R"(foo v3.14

My foo is good for you

    --help,-h             Help output
     
        --flag1,-a        Flag1 description
        --flag2
        --arg1 <Value>    Arg1 description
        -b                b description
)"s},
            std::tuple{root(help(policy::long_name_t{"help"_S},
                                 policy::short_name_t{"h"_S},
                                 policy::flatten_help,
                                 policy::description_t{"Help output"_S},
                                 policy::program_name_t{"foo"_S},
                                 policy::program_version_t{"v3.14"_S},
                                 policy::program_intro_t{"My foo is good for you"_S}),
                            mode("test"_S,
                                 "Test mode"_S,
                                 flag(policy::long_name_t{"flag1"_S}),
                                 arg<int>(policy::long_name_t{"arg1"_S},
                                          policy::description_t{"Arg1 description"_S},
                                          policy::runtime_enable{false}),
                                 policy::router{[](bool, int) {}}),
                            mode(flag(policy::long_name_t{"flag1"_S},
                                      policy::short_name_t{"a"_S},
                                      policy::description_t{"Flag1 description"_S}),
                                 flag(policy::long_name_t{"flag2"_S}),
                                 arg<int>(policy::long_name_t{"arg1"_S},
                                          policy::description_t{"Arg1 description"_S}),
                                 flag(policy::short_name_t{"b"_S},
                                      policy::description_t{"b description"_S}),
                                 policy::router{[](bool, bool, bool) {}}),
                            policy::validation::default_validator),
                       R"(foo v3.14

My foo is good for you

    --help,-h             Help output
    test                  Test mode
        --flag1
     
        --flag1,-a        Flag1 description
        --flag2
        --arg1 <Value>    Arg1 description
        -b                b description
)"s}});
}

BOOST_AUTO_TEST_CASE(runtime_enable_flag_parse_test)
{
    auto f =
        [&](auto args, auto expected, auto enable_flag1, auto enable_flag2, auto fail_message) {
            auto result = std::array<bool, 2>{};

            try {
                const auto r = root(flag(policy::long_name_t{"flag1"_S},
                                         policy::description_t{"First description"_S},
                                         policy::runtime_enable{enable_flag1},
                                         policy::router{[&](bool) { result[0] = true; }}),
                                    flag(policy::long_name_t{"flag2"_S},
                                         policy::description_t{"Second description"_S},
                                         policy::runtime_enable{enable_flag2},
                                         policy::router{[&](bool) { result[1] = true; }}),
                                    policy::validation::default_validator);

                r.parse(args);
                BOOST_CHECK_EQUAL(result[0], expected[0]);
                BOOST_CHECK_EQUAL(result[1], expected[1]);
                BOOST_CHECK(fail_message.empty());
            } catch (parse_exception& e) {
                BOOST_CHECK_EQUAL(fail_message, e.what());
            }
        };

    test::data_set(
        f,
        {
            std::tuple{std::vector{"--flag1"}, std::array{true, false}, true, true, ""s},
            std::tuple{std::vector{"--flag2"}, std::array{false, true}, true, true, ""s},
            std::tuple{std::vector{"--flag1"}, std::array{true, false}, true, false, ""s},
            std::tuple{std::vector{"--flag2"}, std::array{false, true}, false, true, ""s},
            std::tuple{std::vector{"--flag1"},
                       std::array{false, false},
                       false,
                       true,
                       "Unknown argument: --flag1. Did you mean --flag2?"s},
            std::tuple{std::vector{"--flag2"},
                       std::array{false, false},
                       true,
                       false,
                       "Unknown argument: --flag2. Did you mean --flag1?"s},
        });
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
