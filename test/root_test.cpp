#include "arg_router/policy/validator.hpp"

#include "test_helpers.hpp"

#include <optional>

using namespace arg_router;
using namespace std::string_literals;
using namespace std::string_view_literals;

namespace
{
using default_validator_type =
    std::decay_t<decltype(policy::validation::default_validator)>;
}

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

BOOST_AUTO_TEST_CASE(no_children_parse_test)
{
    const auto r = root(policy::validation::default_validator);

    auto args = std::vector{"foo"};
    BOOST_CHECK_EXCEPTION(r.parse(args.size(), const_cast<char**>(args.data())),
                          parse_exception,
                          [](const auto& e) {
                              return e.what() ==
                                     "Default value support not added yet!"s;
                          });
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
                                     "Missing required argument: arg"s;
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
                      router_hit = true;
                      std::get<0>(result) = hello;
                      std::get<1>(result) = arg1;
                      std::get<2>(result) = arg2;
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
                                     "Missing required argument: arg1"s;
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
