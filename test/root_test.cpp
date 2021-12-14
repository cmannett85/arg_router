#include "arg_router/policy/validator.hpp"

#include "test_helpers.hpp"

#include <bitset>
#include <optional>

using namespace arg_router;
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

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(is_tree_node_v<root_t<default_validator_type>>,
                  "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(validator_type_test)
{
    static_assert(
        std::is_same_v<typename root_t<default_validator_type>::validator_type,
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
    BOOST_CHECK_EXCEPTION(
        r.parse(args.size(), const_cast<char**>(args.data())),
        parse_exception,
        [](const auto& e) { return e.what() == "Unhandled argument: --foo"s; });
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
        result = {};
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
        result = {A{}, B{}, B{}};
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

BOOST_AUTO_TEST_CASE(anonymous_mode_single_flag_parse_test)
{
    auto router_hit = false;
    const auto r = root(mode(flag(policy::long_name<S_("hello")>,
                                  policy::description<S_("Hello description")>),
                             policy::router{[&](bool) { router_hit = true; }}),
                        policy::validation::default_validator);

    auto args = std::vector{"foo", "--hello"};
    r.parse(args.size(), const_cast<char**>(args.data()));
    BOOST_CHECK(router_hit);
}

BOOST_AUTO_TEST_CASE(anonymous_mode_single_arg_parse_test)
{
    auto result = std::optional<int>{};
    const auto r =
        root(mode(arg<int>(policy::long_name<S_("hello")>,
                           policy::required,
                           policy::description<S_("Hello description")>),
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
    const auto r =
        root(mode(flag(policy::long_name<S_("hello")>,
                       policy::description<S_("Hello description")>),
                  arg<int>(policy::long_name<S_("arg")>,
                           policy::required,
                           policy::description<S_("Arg description")>),
                  policy::router{[&](auto, auto) {
                      BOOST_CHECK_MESSAGE(false, "Router should not be called");
                  }}),
             policy::validation::default_validator);

    auto args = std::vector{"foo", "--hello"};
    BOOST_CHECK_EXCEPTION(r.parse(args.size(), const_cast<char**>(args.data())),
                          parse_exception,
                          [](const auto& e) {
                              return e.what() ==
                                     "Missing required argument: --arg"s;
                          });
}

BOOST_AUTO_TEST_CASE(anonymous_mode_single_arg_default_parse_test)
{
    auto router_hit = false;
    auto result = std::tuple<bool, int, int>{};
    const auto r =
        root(mode(flag(policy::long_name<S_("hello")>,
                       policy::description<S_("Hello description")>),
                  arg<int>(policy::long_name<S_("arg1")>,
                           policy::default_value{42},
                           policy::description<S_("Arg1 description")>),
                  arg<int>(policy::long_name<S_("arg2")>,
                           policy::required,
                           policy::description<S_("Arg2 description")>),
                  policy::router{[&](auto hello, auto arg1, auto arg2) {
                      result = {hello, arg1, arg2};
                      router_hit = true;
                  }}),
             policy::validation::default_validator);

    auto f = [&](auto args, auto expected_value) {
        result = {};
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
            std::tuple{std::vector{"foo", "--arg2", "84"},
                       std::tuple{false, 42, 84}},
            std::tuple{std::vector{"foo", "--arg2", "3", "--arg1", "19"},
                       std::tuple{false, 19, 3}},
            std::tuple{std::vector{"foo", "--hello", "--arg2", "14"},
                       std::tuple{true, 42, 14}},
        });
}

BOOST_AUTO_TEST_CASE(anonymous_mode_no_tokens_parse_test)
{
    auto router_hit = false;
    auto result = std::tuple<bool, int, int>{};
    const auto r =
        root(mode(flag(policy::long_name<S_("hello")>,
                       policy::description<S_("Hello description")>),
                  arg<int>(policy::long_name<S_("arg1")>,
                           policy::default_value{42},
                           policy::description<S_("Arg1 description")>),
                  arg<int>(policy::long_name<S_("arg2")>,
                           policy::default_value{84},
                           policy::description<S_("Arg2 description")>),
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

BOOST_AUTO_TEST_CASE(multiple_required_arg_parse_test)
{
    const auto r =
        root(mode(flag(policy::long_name<S_("hello")>,
                       policy::description<S_("Hello description")>),
                  arg<int>(policy::long_name<S_("arg1")>,
                           policy::required,
                           policy::description<S_("Arg1 description")>),
                  arg<int>(policy::long_name<S_("arg2")>,
                           policy::required,
                           policy::description<S_("Arg2 description")>),
                  policy::router{[&](auto, auto, auto) {
                      BOOST_CHECK_MESSAGE(false, "Router should not be called");
                  }}),
             policy::validation::default_validator);

    auto args = std::vector{"foo", "--hello", "--arg2", "42"};
    BOOST_CHECK_EXCEPTION(r.parse(args.size(), const_cast<char**>(args.data())),
                          parse_exception,
                          [](const auto& e) {
                              return e.what() ==
                                     "Missing required argument: --arg1"s;
                          });
}

BOOST_AUTO_TEST_CASE(anonymous_triple_single_flag_parse_test)
{
    auto router_hit = false;
    auto result = std::array<bool, 3>{};
    const auto r =
        root(mode(flag(policy::long_name<S_("flag1")>,
                       policy::description<S_("First description")>),
                  flag(policy::long_name<S_("flag2")>,
                       policy::description<S_("Second description")>),
                  flag(policy::short_name<'t'>,
                       policy::description<S_("Third description")>),
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
            std::tuple{std::vector{"foo", "--flag1"},
                       std::array{true, false, false},
                       ""},
            std::tuple{std::vector{"foo", "--flag2"},
                       std::array{false, true, false},
                       ""},
            std::tuple{std::vector{"foo", "-t"},
                       std::array{false, false, true},
                       ""},
            std::tuple{std::vector{"foo", "--flag1", "-t"},
                       std::array{true, false, true},
                       ""},
            std::tuple{std::vector{"foo", "-t", "--flag1"},
                       std::array{true, false, true},
                       ""},
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
                       "Unknown argument: --foo"},
            std::tuple{std::vector{"foo", "--flag2", "-t", "--flag1", "--foo"},
                       std::array{false, false, false},
                       "Unknown argument: --foo"},
            std::tuple{std::vector{"foo", "--flag1", "--flag1"},
                       std::array{false, false, false},
                       "Argument has already been set: --flag1"},
            std::tuple{std::vector{"foo", "-t", "-t"},
                       std::array{false, false, false},
                       "Argument has already been set: -t"},
            std::tuple{
                std::vector{"foo", "--flag2", "-t", "--flag1", "--flag2"},
                std::array{false, false, false},
                "Argument has already been set: --flag2"},
        });
}

BOOST_AUTO_TEST_CASE(named_single_mode_parse_test)
{
    auto router_hit = false;
    auto result = std::array<bool, 3>{};
    const auto r =
        root(mode(policy::long_name<S_("my-mode")>,
                  flag(policy::long_name<S_("flag1")>,
                       policy::description<S_("First description")>),
                  flag(policy::long_name<S_("flag2")>,
                       policy::description<S_("Second description")>),
                  flag(policy::short_name<'t'>,
                       policy::description<S_("Third description")>),
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
            std::tuple{std::vector{"foo", "my-mode", "-t"},
                       std::array{false, false, true},
                       ""},
            std::tuple{std::vector{"foo", "my-mode", "--flag1", "-t"},
                       std::array{true, false, true},
                       ""},
            std::tuple{std::vector{"foo", "my-mode", "-t", "--flag1"},
                       std::array{true, false, true},
                       ""},
            std::tuple{
                std::vector{"foo", "my-mode", "--flag1", "--flag2", "-t"},
                std::array{true, true, true},
                ""},
            std::tuple{
                std::vector{"foo", "my-mode", "--flag2", "-t", "--flag1"},
                std::array{true, true, true},
                ""},
            std::tuple{std::vector{"foo", "my-mode", "--foo", "--flag2"},
                       std::array{false, false, false},
                       "Unknown argument: --foo"},
            std::tuple{std::vector{"foo", "my-mode", "--flag2", "--foo"},
                       std::array{false, false, false},
                       "Unknown argument: --foo"},
            std::tuple{std::vector{"foo",
                                   "my-mode",
                                   "--flag1",
                                   "--flag2",
                                   "-t",
                                   "--foo"},
                       std::array{false, false, false},
                       "Unknown argument: --foo"},
            std::tuple{std::vector{"foo",
                                   "my-mode",
                                   "--flag2",
                                   "-t",
                                   "--flag1",
                                   "--foo"},
                       std::array{false, false, false},
                       "Unknown argument: --foo"},
            std::tuple{std::vector{"foo", "my-mode", "--flag1", "--flag1"},
                       std::array{false, false, false},
                       "Argument has already been set: --flag1"},
            std::tuple{std::vector{"foo", "my-mode", "-t", "-t"},
                       std::array{false, false, false},
                       "Argument has already been set: -t"},
            std::tuple{std::vector{"foo",
                                   "my-mode",
                                   "--flag2",
                                   "-t",
                                   "--flag1",
                                   "--flag2"},
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

    const auto r =
        root(mode(policy::long_name<S_("mode1")>,
                  flag(policy::long_name<S_("flag1")>,
                       policy::description<S_("First description")>),
                  flag(policy::long_name<S_("flag2")>,
                       policy::description<S_("Second description")>),
                  flag(policy::short_name<'t'>,
                       policy::description<S_("Third description")>),
                  policy::router{[&](bool flag1, bool flag2, bool t) {
                      result1 = {flag1, flag2, t};
                      router_hit1 = true;
                  }}),
             mode(policy::long_name<S_("mode2")>,
                  flag(policy::long_name<S_("flag1")>,
                       policy::description<S_("Other third description")>),
                  flag(policy::short_name<'b'>,
                       policy::description<S_("Fourth description")>),
                  policy::router{[&](bool flag1, bool b) {
                      result2 = {flag1, b};
                      router_hit2 = true;
                  }}),
             policy::validation::default_validator);

    auto f = [&](auto args,
                 auto router_index,
                 auto expected,
                 std::string fail_message) {
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
            std::tuple{std::vector{"foo", "mode2", "--flag1"},
                       1,
                       std::vector{true, false},
                       ""},
            std::tuple{std::vector{"foo", "mode1", "mode2", "--flag1"},
                       0,
                       std::vector{false, false, false},
                       "Unknown argument: mode2"},
            std::tuple{std::vector{"foo", "mode2", "-b"},
                       1,
                       std::vector{false, true},
                       ""},
        });
}

BOOST_AUTO_TEST_CASE(named_multi_mode_using_list_parse_test)
{
    auto router_hit1 = false;
    auto router_hit2 = false;
    auto result1 = std::array<bool, 3>{};
    auto result2 = std::array<bool, 2>{};

    const auto flag1 = list{flag(policy::long_name<S_("flag1")>,
                                 policy::description<S_("First description")>)};

    const auto r =
        root(mode(policy::long_name<S_("mode1")>,
                  flag1,
                  flag(policy::long_name<S_("flag2")>,
                       policy::description<S_("Second description")>),
                  flag(policy::short_name<'t'>,
                       policy::description<S_("Third description")>),
                  policy::router{[&](bool flag1, bool flag2, bool t) {
                      result1 = {flag1, flag2, t};
                      router_hit1 = true;
                  }}),
             mode(policy::long_name<S_("mode2")>,
                  flag1,
                  flag(policy::short_name<'b'>,
                       policy::description<S_("Fourth description")>),
                  policy::router{[&](bool flag1, bool b) {
                      result2 = {flag1, b};
                      router_hit2 = true;
                  }}),
             policy::validation::default_validator);

    auto f = [&](auto args,
                 auto router_index,
                 auto expected,
                 std::string fail_message) {
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
            std::tuple{std::vector{"foo", "mode2", "--flag1"},
                       1,
                       std::vector{true, false},
                       ""},
            std::tuple{std::vector{"foo", "mode1", "mode2", "--flag1"},
                       0,
                       std::vector{false, false, false},
                       "Unknown argument: mode2"},
            std::tuple{std::vector{"foo", "mode2", "-b"},
                       1,
                       std::vector{false, true},
                       ""},
        });
}

BOOST_AUTO_TEST_CASE(alias_flag_parse_test)
{
    auto router_hit = false;
    auto result = std::array<bool, 3>{};
    const auto r =
        root(mode(flag(policy::long_name<S_("flag1")>,
                       policy::description<S_("First description")>),
                  flag(policy::long_name<S_("flag2")>,
                       policy::description<S_("Second description")>),
                  flag(policy::long_name<S_("flag3")>,
                       policy::description<S_("Third description")>),
                  flag(policy::short_name<'a'>,
                       policy::alias(policy::long_name<S_("flag1")>,
                                     policy::long_name<S_("flag3")>),
                       policy::description<S_("Alias description")>),
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

    test::data_set(f,
                   {
                       std::tuple{std::vector{"foo", "--flag1"},
                                  std::array{true, false, false},
                                  ""},
                       std::tuple{std::vector{"foo", "--flag2"},
                                  std::array{false, true, false},
                                  ""},
                       std::tuple{std::vector{"foo", "--flag3"},
                                  std::array{false, false, true},
                                  ""},
                       std::tuple{std::vector{"foo", "-a"},
                                  std::array{true, false, true},
                                  ""},
                       std::tuple{std::vector{"foo", "-a", "--flag2"},
                                  std::array{true, true, true},
                                  ""},
                       std::tuple{std::vector{"foo", "-a", "--flag1"},
                                  std::array{true, true, true},
                                  "Argument has already been set: --flag1"},
                   });
}

BOOST_AUTO_TEST_CASE(alias_arg_parse_test)
{
    auto router_hit = false;
    auto result = std::tuple<bool, int, int>{};
    const auto r =
        root(mode(arg<bool>(policy::long_name<S_("arg1")>,
                            policy::required,
                            policy::description<S_("First description")>),
                  arg<int>(policy::long_name<S_("arg2")>,
                           policy::default_value(42),
                           policy::description<S_("Second description")>),
                  arg<int>(policy::long_name<S_("arg3")>,
                           policy::default_value(84),
                           policy::description<S_("Third description")>),
                  arg<int>(policy::short_name<'a'>,
                           policy::alias(policy::long_name<S_("arg2")>,
                                         policy::long_name<S_("arg3")>),
                           policy::description<S_("Alias description")>),
                  policy::router{[&](bool arg1, int arg2, int arg3) {
                      result = {arg1, arg2, arg3};
                      router_hit = true;
                  }}),
             policy::validation::default_validator);

    auto f = [&](auto args, auto expected, std::string fail_message) {
        result = {};
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
            std::tuple{std::vector{"foo", "--arg1", "true"},
                       std::tuple{true, 42, 84},
                       ""},
            std::tuple{std::vector{"foo", "--arg1", "false", "-a", "9"},
                       std::tuple{false, 9, 9},
                       ""},
            std::tuple{std::vector{"foo", "--arg2", "13", "-a", "9"},
                       std::tuple{false, 9, 9},
                       "Argument has already been set: --arg2"},
            std::tuple{std::vector{"foo", "-a", "9"},
                       std::tuple{false, 9, 9},
                       "Missing required argument: --arg1"},
        });
}

BOOST_AUTO_TEST_CASE(single_positional_arg_parse_test)
{
    auto router_hit = false;
    auto result = std::tuple<bool, int, std::vector<std::string_view>>{};
    const auto r =
        root(mode(flag(policy::long_name<S_("flag1")>,
                       policy::description<S_("First description")>),
                  arg<int>(policy::long_name<S_("arg1")>,
                           policy::default_value(42),
                           policy::description<S_("Second description")>),
                  positional_arg<std::vector<std::string_view>>(
                      policy::long_name<S_("pos_args")>,
                      policy::description<S_("Third description")>,
                      policy::min_count<2>),
                  policy::router{[&](bool flag1,
                                     int arg1,
                                     std::vector<std::string_view> pos_args) {
                      result = {flag1, arg1, pos_args};
                      router_hit = true;
                  }}),
             policy::validation::default_validator);

    auto f = [&](auto args, auto expected, std::string fail_message) {
        result = {};
        router_hit = false;

        try {
            r.parse(args.size(), const_cast<char**>(args.data()));
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK(router_hit);
            BOOST_CHECK_EQUAL(std::get<0>(result), std::get<0>(expected));
            BOOST_CHECK_EQUAL(std::get<1>(result), std::get<1>(expected));
            BOOST_CHECK(std::get<2>(result) == std::get<2>(expected));
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
                                  std::vector<std::string_view>{"one", "two"}},
                       ""},
            std::tuple{std::vector{"foo", "--flag1", "one", "two"},
                       std::tuple{true,
                                  42,
                                  std::vector<std::string_view>{"one", "two"}},
                       ""},
            std::tuple{std::vector{"foo", "--arg1", "14", "one", "two"},
                       std::tuple{false,
                                  14,
                                  std::vector<std::string_view>{"one", "two"}},
                       ""},
            std::tuple{
                std::vector{"foo", "--arg1", "14", "--flag1", "one", "two"},
                std::tuple{true,
                           14,
                           std::vector<std::string_view>{"one", "two"}},
                ""},
            std::tuple{std::vector{"foo", "--flag1"},
                       std::tuple{true, 42, std::vector<std::string_view>{}},
                       "Minimum count not reached: --pos_args"},
            std::tuple{std::vector{"foo", "--flag1", "--arg1", "9"},
                       std::tuple{true, 42, std::vector<std::string_view>{}},
                       "Minimum count not reached: --pos_args"},
        });
}

BOOST_AUTO_TEST_CASE(two_positional_arg_parse_test)
{
    auto router_hit = false;
    auto result = std::
        tuple<bool, int, std::vector<std::string_view>, std::vector<double>>{};
    const auto r =
        root(mode(flag(policy::long_name<S_("flag1")>,
                       policy::description<S_("First description")>),
                  arg<int>(policy::long_name<S_("arg1")>,
                           policy::default_value(42),
                           policy::description<S_("Second description")>),
                  positional_arg<std::vector<std::string_view>>(
                      policy::long_name<S_("pos_args1")>,
                      policy::description<S_("Third description")>,
                      policy::count<2>),
                  positional_arg<std::vector<double>>(
                      policy::long_name<S_("pos_args2")>,
                      policy::description<S_("Fourth description")>),
                  policy::router{[&](bool flag1,
                                     int arg1,
                                     std::vector<std::string_view> pos_args1,
                                     std::vector<double> pos_args2) {
                      result = {flag1, arg1, pos_args1, pos_args2};
                      router_hit = true;
                  }}),
             policy::validation::default_validator);

    auto f = [&](auto args, auto expected, std::string fail_message) {
        result = {};
        router_hit = false;

        try {
            r.parse(args.size(), const_cast<char**>(args.data()));
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
            std::tuple{std::vector{"foo", "one", "two", "three"},
                       std::tuple{false,
                                  42,
                                  std::vector<std::string_view>{},
                                  std::vector<double>{}},
                       "Failed to parse: three"},
            std::tuple{
                std::vector{"foo", "one", "--flag1", "two", "--arg1", "5"},
                std::tuple{false,
                           42,
                           std::vector<std::string_view>{},
                           std::vector<double>{}},
                "Failed to parse: two"},
        });
}

BOOST_AUTO_TEST_CASE(nested_mode_test)
{
    auto router_hit = std::bitset<6>{};
    auto result =
        std::variant<std::tuple<bool>,
                     std::tuple<int>,
                     std::tuple<bool, double, bool>,
                     std::tuple<int, bool, std::vector<std::string_view>>,
                     std::tuple<bool, bool>,
                     std::tuple<bool, double>>{};

    const auto r = root(
        flag(policy::long_name<S_("top-flag")>,
             policy::description<S_("Description")>,
             policy::router{[&](bool v) {
                 router_hit[0] = true;
                 result = std::tuple{v};
             }}),
        arg<int>(policy::long_name<S_("top-arg")>,
                 policy::description<S_("Description")>,
                 policy::router{[&](int v) {
                     router_hit[1] = true;
                     result = std::tuple{v};
                 }}),
        mode(policy::long_name<S_("mode1")>,
             flag(policy::long_name<S_("flag1")>,
                  policy::description<S_("Description")>),
             arg<double>(policy::long_name<S_("arg1")>,
                         policy::description<S_("Description")>,
                         policy::default_value{3.14}),
             flag(policy::long_name<S_("flag2")>,
                  policy::short_name<'t'>,
                  policy::description<S_("Description")>),
             policy::router{[&](bool f1, double a1, bool f2) {
                 router_hit[2] = true;
                 result = std::tuple{f1, a1, f2};
             }},
             mode(policy::long_name<S_("mode2")>,
                  arg<int>(policy::long_name<S_("arg1")>,
                           policy::description<S_("Description")>,
                           policy::required),
                  flag(policy::long_name<S_("flag1")>,
                       policy::short_name<'b'>,
                       policy::description<S_("Description")>),
                  positional_arg<std::vector<std::string_view>>(
                      policy::long_name<S_("pos_args")>,
                      policy::description<S_("Description")>),
                  policy::router{[&](int a1,
                                     bool f1,
                                     std::vector<std::string_view> pos_args) {
                      router_hit[3] = true;
                      result = std::tuple{a1, f1, std::move(pos_args)};
                  }}),
             mode(policy::long_name<S_("mode3")>,
                  flag(policy::long_name<S_("flag1")>,
                       policy::description<S_("Description")>),
                  flag(policy::long_name<S_("flag2")>,
                       policy::short_name<'b'>,
                       policy::description<S_("Description")>),
                  policy::router{[&](bool f1, bool f2) {
                      router_hit[4] = true;
                      result = std::tuple{f1, f2};
                  }})),
        mode(flag(policy::long_name<S_("flag1")>,
                  policy::description<S_("Description")>),
             arg<double>(policy::long_name<S_("arg1")>,
                         policy::default_value{4.2},
                         policy::description<S_("Description")>),
             policy::router{[&](bool f1, double a1) {
                 router_hit[5] = true;
                 result = std::tuple{f1, a1};
             }}),
        policy::validation::default_validator);

    auto f = [&](auto args,
                 auto expected_index,
                 auto expected,
                 std::string fail_message) {
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
                        BOOST_CHECK_MESSAGE(false,
                                            "Unexpected router arg type");
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
            std::tuple{std::vector{"foo", "--top-flag"},
                       0,
                       std::tuple{true},
                       ""},
            std::tuple{std::vector{"foo", "--top-arg", "42"},
                       1,
                       std::tuple{42},
                       ""},
            std::tuple{std::vector{"foo"}, 5, std::tuple{false, 4.2}, ""},
            std::tuple{std::vector{"foo", "--arg1", "13"},
                       5,
                       std::tuple{false, 13.0},
                       ""},
            std::tuple{std::vector{"foo", "mode1", "-t"},
                       2,
                       std::tuple{false, 3.14, true},
                       ""},
            std::tuple{std::vector{"foo", "mode1", "--arg1", "5.6", "--flag1"},
                       2,
                       std::tuple{true, 5.6, false},
                       ""},
            std::tuple{std::vector{"foo", "mode1", "mode2", "--arg1", "89"},
                       3,
                       std::tuple{89, false, std::vector<std::string_view>{}},
                       ""},
            std::tuple{
                std::vector{"foo", "mode1", "mode2", "-b", "--arg1", "4"},
                3,
                std::tuple{4, true, std::vector<std::string_view>{}},
                ""},
            std::tuple{std::vector{"foo", "mode1", "mode3", "-b"},
                       4,
                       std::tuple{false, true},
                       ""},
            std::tuple{
                std::vector{"foo",
                            "mode1",
                            "mode2",
                            "--arg1",
                            "8",
                            "hello",
                            "goodbye"},
                3,
                std::tuple{8,
                           false,
                           std::vector<std::string_view>{"hello", "goodbye"}},
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

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(must_have_validator_policy_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"

int main() {
        arg_router::root_t<
            arg_router::flag_t<
                arg_router::policy::short_name_t<
                    arg_router::traits::integral_constant<'a'>>,
                arg_router::policy::long_name_t<S_("test")>,
                arg_router::policy::router<std::less<>>>>();
    return 0;
}
    )",
        "Root must have a validator policy, use "
        "policy::validation::default_validator unless you have created a "
        "custom one");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
