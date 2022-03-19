/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
namespace ard = arg_router::dependency;
using namespace std::string_literals;
using namespace std::string_view_literals;

namespace
{
using default_validator_type =
    std::decay_t<decltype(policy::validation::default_validator)>;

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
    static B parse(std::string_view token)
    {
        return B{parser<double>::parse(token)};
    }
};

BOOST_AUTO_TEST_SUITE(root_suite)

BOOST_AUTO_TEST_SUITE(single_tests)

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(
        is_tree_node_v<root_t<flag_t<policy::long_name_t<S_("hello")>,
                                     policy::router<std::function<void(bool)>>>,
                              default_validator_type>>,
        "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(validator_type_test)
{
    static_assert(
        std::is_same_v<
            typename root_t<flag_t<policy::long_name_t<S_("hello")>,
                                   policy::router<std::function<void(bool)>>>,
                            default_validator_type>::validator_type,
            default_validator_type>,
        "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(constructor_validation_test)
{
    const auto r = root(policy::validation::default_validator,
                        flag(policy::long_name<S_("hello")>,
                             policy::description<S_("This is a hello")>,
                             policy::short_name<'h'>,
                             policy::router{[]() {}}),
                        flag(policy::long_name<S_("goodbye")>,
                             policy::description<S_("This is a goodbye flag")>,
                             policy::short_name<'g'>,
                             policy::router{[]() {}}));

    BOOST_CHECK_EQUAL(std::get<0>(r.children()).long_name(), "hello");
    BOOST_CHECK_EQUAL(std::get<1>(r.children()).long_name(), "goodbye");
}

BOOST_AUTO_TEST_CASE(unknown_argument_parse_test)
{
    auto router_hit = false;
    const auto r = root(flag(policy::long_name<S_("hello")>,
                             policy::description<S_("Hello description")>,
                             policy::router{[&](bool) { router_hit = true; }}),
                        policy::validation::default_validator);

    auto args = std::vector{"foo", "--foo"};
    BOOST_CHECK_EXCEPTION(
        r.parse(args.size(), const_cast<char**>(args.data())),
        parse_exception,
        [](const auto& e) { return e.what() == "Unknown argument: --foo"s; });
    BOOST_CHECK(!router_hit);
}

BOOST_AUTO_TEST_CASE(unhandled_parse_test)
{
    auto router_hit = false;
    const auto r = root(flag(policy::long_name<S_("hello")>,
                             policy::description<S_("Hello description")>,
                             policy::router{[&](bool) { router_hit = true; }}),
                        policy::validation::default_validator);

    auto args = std::vector{"foo", "--hello", "--foo"};
    BOOST_CHECK_EXCEPTION(r.parse(args.size(), const_cast<char**>(args.data())),
                          parse_exception,
                          [](const auto& e) {
                              return e.what() == "Unhandled arguments: --foo"s;
                          });
    BOOST_CHECK(!router_hit);
}

BOOST_AUTO_TEST_CASE(single_flag_parse_test)
{
    auto router_hit = false;
    const auto r = root(flag(policy::long_name<S_("hello")>,
                             policy::description<S_("Hello description")>,
                             policy::router{[&](bool) { router_hit = true; }}),
                        policy::validation::default_validator);

    auto args = std::vector{"foo", "--hello"};
    r.parse(args.size(), const_cast<char**>(args.data()));
    BOOST_CHECK(router_hit);
}

BOOST_AUTO_TEST_CASE(single_arg_parse_test)
{
    auto result = std::optional<int>{};
    const auto r = root(arg<int>(policy::long_name<S_("hello")>,
                                 policy::description<S_("Hello description")>,
                                 policy::router{[&](int value) {
                                     BOOST_CHECK(!result);
                                     result = value;
                                 }}),
                        policy::validation::default_validator);

    auto args = std::vector{"foo", "--hello", "42"};
    r.parse(args.size(), const_cast<char**>(args.data()));
    BOOST_REQUIRE(!!result);
    BOOST_CHECK_EQUAL(*result, 42);
}

BOOST_AUTO_TEST_CASE(single_arg_separator_parse_test)
{
    auto result = std::optional<int>{};
    const auto r = root(arg<int>(policy::long_name<S_("hello")>,
                                 policy::description<S_("Hello description")>,
                                 policy::value_separator<'='>,
                                 policy::router{[&](int value) {
                                     BOOST_CHECK(!result);
                                     result = value;
                                 }}),
                        policy::validation::default_validator);

    auto f = [&](auto args, auto expected, auto fail_message) {
        result.reset();

        try {
            r.parse(args.size(), const_cast<char**>(args.data()));
            BOOST_CHECK(fail_message.empty());
            BOOST_REQUIRE(!!result);
            BOOST_CHECK_EQUAL(*result, expected);
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
        }
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector{"foo", "--hello=42"}, 42, ""s},
            std::tuple{std::vector{"foo", "--hello", "42"},
                       0,
                       "Expected to find value separator '=' in \"--hello\""s},
            std::tuple{std::vector{"foo", "--hello="},
                       0,
                       "Unable to find value after separator: --hello="s},
        });
}

BOOST_AUTO_TEST_CASE(single_string_arg_parse_test)
{
    auto result = std::optional<std::string_view>{};
    const auto r =
        root(arg<std::string_view>(policy::long_name<S_("hello")>,
                                   policy::description<S_("Hello description")>,
                                   policy::router{[&](std::string_view value) {
                                       BOOST_CHECK(!result);
                                       result = value;
                                   }}),
             policy::validation::default_validator);

    auto f = [&](auto args, auto expected) {
        result.reset();

        r.parse(args.size(), const_cast<char**>(args.data()));
        BOOST_REQUIRE(!!result);
        BOOST_CHECK_EQUAL(*result, expected);
    };

    test::data_set(
        f,
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
    const auto r = root(flag(policy::long_name<S_("flag1")>,
                             policy::description<S_("First description")>,
                             policy::router{[&](bool) { result[0] = true; }}),
                        flag(policy::long_name<S_("flag2")>,
                             policy::description<S_("Second description")>,
                             policy::router{[&](bool) { result[1] = true; }}),
                        flag(policy::short_name<'t'>,
                             policy::description<S_("Third description")>,
                             policy::router{[&](bool) { result[2] = true; }}),
                        policy::validation::default_validator);

    auto f = [&](auto args, auto expected) {
        result.fill(false);

        r.parse(args.size(), const_cast<char**>(args.data()));
        BOOST_CHECK_EQUAL(result[0], expected[0]);
        BOOST_CHECK_EQUAL(result[1], expected[1]);
        BOOST_CHECK_EQUAL(result[2], expected[2]);
    };

    test::data_set(f,
                   {
                       std::tuple{std::vector{"foo", "--flag1"},
                                  std::array{true, false, false}},
                       std::tuple{std::vector{"foo", "--flag2"},
                                  std::array{false, true, false}},
                       std::tuple{std::vector{"foo", "-t"},
                                  std::array{false, false, true}},
                   });
}

BOOST_AUTO_TEST_CASE(triple_arg_parse_test)
{
    auto result = std::tuple<int, double, std::string_view>{};
    auto hit = std::array<bool, 3>{};

    const auto r =
        root(arg<int>(policy::long_name<S_("flag1")>,
                      policy::description<S_("First description")>,
                      policy::router{[&](auto value) {
                          std::get<0>(result) = value;
                          hit[0] = true;
                      }}),
             arg<double>(policy::long_name<S_("flag2")>,
                         policy::description<S_("Second description")>,
                         policy::router{[&](auto value) {
                             std::get<1>(result) = value;
                             hit[1] = true;
                         }}),
             arg<std::string_view>(policy::short_name<'t'>,
                                   policy::description<S_("Third description")>,
                                   policy::router{[&](auto value) {
                                       std::get<2>(result) = value;
                                       hit[2] = true;
                                   }}),
             policy::validation::default_validator);

    auto f = [&](auto args, auto expected_hit, auto expected_value) {
        result = decltype(result){};
        hit.fill(false);

        r.parse(args.size(), const_cast<char**>(args.data()));
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

    const auto r = root(
        arg<A>(policy::long_name<S_("arg1")>,
               policy::description<S_("First description")>,
               policy::custom_parser<A>{[](auto token) -> A {
                   return A{parser<int>::parse(token)};
               }},
               policy::router{[&](auto arg1) { std::get<0>(result) = arg1; }}),
        arg<B>(policy::long_name<S_("arg2")>,
               policy::description<S_("Second description")>,
               policy::custom_parser<B>{[&](auto token) -> B {
                   parser_hit = true;
                   return B{parser<double>::parse(token)};
               }},
               policy::router{[&](auto arg2) { std::get<1>(result) = arg2; }}),
        arg<B>(policy::long_name<S_("arg3")>,
               policy::description<S_("Third description")>,
               policy::router{[&](auto arg3) { std::get<2>(result) = arg3; }}),
        policy::validation::default_validator);

    auto f = [&](auto args, auto expected_hit, auto expected_value) {
        result = decltype(result){A{}, B{}, B{}};
        parser_hit = false;

        r.parse(args.size(), const_cast<char**>(args.data()));
        BOOST_CHECK_EQUAL(parser_hit, expected_hit);

        BOOST_CHECK_EQUAL(std::get<0>(result).value,
                          std::get<0>(expected_value).value);
        BOOST_CHECK_EQUAL(std::get<1>(result).value,
                          std::get<1>(expected_value).value);
        BOOST_CHECK_EQUAL(std::get<2>(result).value,
                          std::get<2>(expected_value).value);
    };

    test::data_set(f,
                   {
                       std::tuple{std::vector{"foo", "--arg1", "42"},
                                  false,
                                  std::tuple{A{42}, B{}, B{}}},
                       std::tuple{std::vector{"foo", "--arg2", "3.14"},
                                  true,
                                  std::tuple{A{}, B{3.14}, B{}}},
                       std::tuple{std::vector{"foo", "--arg3", "3.3"},
                                  false,
                                  std::tuple{A{}, B{}, B{3.3}}},
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
            std::tuple{root(flag(policy::long_name<S_("flag1")>,
                                 policy::short_name<'a'>,
                                 policy::description<S_("Flag1 description")>,
                                 policy::router{[](bool) {}}),
                            flag(policy::long_name<S_("flag2")>,
                                 policy::router{[](bool) {}}),
                            flag(policy::short_name<'b'>,
                                 policy::description<S_("b description")>,
                                 policy::router{[](bool) {}}),
                            policy::validation::default_validator),
                       ""s},
            std::tuple{
                root(flag(policy::long_name<S_("flag1")>,
                          policy::short_name<'a'>,
                          policy::description<S_("Flag1 description")>,
                          policy::router{[](bool) {}}),
                     flag(policy::long_name<S_("flag2")>,
                          policy::router{[](bool) {}}),
                     flag(policy::short_name<'b'>,
                          policy::description<S_("b description")>,
                          policy::router{[](bool) {}}),
                     arg<int>(policy::long_name<S_("arg1")>,
                              policy::value_separator<'='>,
                              policy::router{[](int) {}}),
                     help(policy::long_name<S_("help")>,
                          policy::short_name<'h'>,
                          policy::description<S_("Help output")>,
                          policy::program_name<S_("foo")>,
                          policy::program_version<S_("v3.14")>,
                          policy::program_intro<S_("My foo is good for you")>),
                     policy::validation::default_validator),
                R"(foo v3.14

My foo is good for you

    --flag1,-a        Flag1 description
    --flag2
    -b                b description
    --arg1=<Value>
    --help,-h         Help output
)"s},
            std::tuple{
                root(help(policy::long_name<S_("help")>,
                          policy::short_name<'h'>,
                          policy::description<S_("Help output")>,
                          policy::program_name<S_("foo")>,
                          policy::program_version<S_("v3.14")>,
                          policy::program_intro<S_("My foo is good for you")>),
                     mode(flag(policy::long_name<S_("flag1")>,
                               policy::short_name<'a'>,
                               policy::description<S_("Flag1 description")>),
                          flag(policy::long_name<S_("flag2")>),
                          arg<int>(policy::long_name<S_("arg1")>,
                                   policy::value_separator<'='>,
                                   policy::description<S_("Arg1 description")>),
                          flag(policy::short_name<'b'>,
                               policy::description<S_("b description")>),
                          policy::router{[](bool, bool, bool) {}}),
                     policy::validation::default_validator),
                R"(foo v3.14

My foo is good for you

    --help,-h             Help output
        --flag1,-a        Flag1 description
        --flag2
        --arg1=<Value>    Arg1 description
        -b                b description
)"s}});
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
