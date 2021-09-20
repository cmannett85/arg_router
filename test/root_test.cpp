#include "arg_router/policy/validator.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace std::string_literals;

namespace
{
using default_validator_type =
    std::decay_t<decltype(policy::validation::default_validator)>;
}

BOOST_AUTO_TEST_SUITE(root_suite)

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(is_tree_node_v<root<default_validator_type>>,
                  "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(validator_type_test)
{
    static_assert(
        std::is_same_v<typename root<default_validator_type>::validator_type,
                       default_validator_type>,
        "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(constructor_validation_test)
{
    const auto r = root{
        policy::validation::default_validator,
        flag{policy::long_name<S_("hello")>,
             policy::description<S_("This is a hello")>,
             policy::short_name<'h'>,
             policy::router{[]() {}}},
        flag{policy::long_name<S_("goodbye")>,
             policy::description<S_("This is a goodbye flag")>,
             policy::short_name<'g'>,
             policy::router{[]() {}}},
    };

    BOOST_CHECK_EQUAL(std::get<0>(r.children()).long_name(), "hello");
    BOOST_CHECK_EQUAL(std::get<1>(r.children()).long_name(), "goodbye");
}

BOOST_AUTO_TEST_CASE(no_children_parse_test)
{
    const auto r = root{policy::validation::default_validator};

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
    const auto r = root{flag{policy::long_name<S_("hello")>,
                             policy::description<S_("Hello description")>,
                             policy::router{[&](bool) { router_hit = true; }}},
                        policy::validation::default_validator};

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
    const auto r = root{flag{policy::long_name<S_("hello")>,
                             policy::description<S_("Hello description")>,
                             policy::router{[&](bool) { router_hit = true; }}},
                        policy::validation::default_validator};

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
    const auto r = root{flag{policy::long_name<S_("hello")>,
                             policy::description<S_("Hello description")>,
                             policy::router{[&](bool) { router_hit = true; }}},
                        policy::validation::default_validator};

    auto args = std::vector{"foo", "--hello"};
    r.parse(args.size(), const_cast<char**>(args.data()));
    BOOST_CHECK(router_hit);
}

BOOST_AUTO_TEST_CASE(triple_flag_parse_test)
{
    auto result = std::array<bool, 3>{};
    const auto r = root{flag{policy::long_name<S_("flag1")>,
                             policy::description<S_("First description")>,
                             policy::router{[&](bool) { result[0] = true; }}},
                        flag{policy::long_name<S_("flag2")>,
                             policy::description<S_("Second description")>,
                             policy::router{[&](bool) { result[1] = true; }}},
                        flag{policy::short_name<'t'>,
                             policy::description<S_("Third description")>,
                             policy::router{[&](bool) { result[2] = true; }}},
                        policy::validation::default_validator};

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

BOOST_AUTO_TEST_CASE(anonymous_mode_single_flag_parse_test)
{
    auto router_hit = false;
    const auto r = root{mode{flag{policy::long_name<S_("hello")>,
                                  policy::description<S_("Hello description")>},
                             policy::router{[&](bool) { router_hit = true; }}},
                        policy::validation::default_validator};

    auto args = std::vector{"foo", "--hello"};
    r.parse(args.size(), const_cast<char**>(args.data()));
    BOOST_CHECK(router_hit);
}

BOOST_AUTO_TEST_CASE(anonymous_triple_single_flag_parse_test)
{
    auto router_hit = false;
    auto result = std::array<bool, 3>{};
    const auto r =
        root{mode{flag{policy::long_name<S_("flag1")>,
                       policy::description<S_("First description")>},
                  flag{policy::long_name<S_("flag2")>,
                       policy::description<S_("Second description")>},
                  flag{policy::short_name<'t'>,
                       policy::description<S_("Third description")>},
                  policy::router{[&](bool flag1, bool flag2, bool t) {
                      result = {flag1, flag2, t};
                      router_hit = true;
                  }}},
             policy::validation::default_validator};

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
                       "Flag has already been set: --flag1"},
            std::tuple{std::vector{"foo", "-t", "-t"},
                       std::array{false, false, false},
                       "Flag has already been set: -t"},
            std::tuple{
                std::vector{"foo", "--flag2", "-t", "--flag1", "--flag2"},
                std::array{false, false, false},
                "Flag has already been set: --flag2"},
        });
}

BOOST_AUTO_TEST_CASE(named_single_mode_parse_test)
{
    auto router_hit = false;
    auto result = std::array<bool, 3>{};
    const auto r =
        root{mode{policy::long_name<S_("my-mode")>,
                  flag{policy::long_name<S_("flag1")>,
                       policy::description<S_("First description")>},
                  flag{policy::long_name<S_("flag2")>,
                       policy::description<S_("Second description")>},
                  flag{policy::short_name<'t'>,
                       policy::description<S_("Third description")>},
                  policy::router{[&](bool flag1, bool flag2, bool t) {
                      result = {flag1, flag2, t};
                      router_hit = true;
                  }}},
             policy::validation::default_validator};

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
                       "Flag has already been set: --flag1"},
            std::tuple{std::vector{"foo", "my-mode", "-t", "-t"},
                       std::array{false, false, false},
                       "Flag has already been set: -t"},
            std::tuple{std::vector{"foo",
                                   "my-mode",
                                   "--flag2",
                                   "-t",
                                   "--flag1",
                                   "--flag2"},
                       std::array{false, false, false},
                       "Flag has already been set: --flag2"},
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
        root{mode{policy::long_name<S_("mode1")>,
                  flag{policy::long_name<S_("flag1")>,
                       policy::description<S_("First description")>},
                  flag{policy::long_name<S_("flag2")>,
                       policy::description<S_("Second description")>},
                  flag{policy::short_name<'t'>,
                       policy::description<S_("Third description")>},
                  policy::router{[&](bool flag1, bool flag2, bool t) {
                      result1 = {flag1, flag2, t};
                      router_hit1 = true;
                  }}},
             mode{policy::long_name<S_("mode2")>,
                  flag{policy::long_name<S_("flag1")>,
                       policy::description<S_("Other third description")>},
                  flag{policy::short_name<'b'>,
                       policy::description<S_("Fourth description")>},
                  policy::router{[&](bool flag1, bool b) {
                      result2 = {flag1, b};
                      router_hit2 = true;
                  }}},
             policy::validation::default_validator};

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

    const auto flag1 = list{flag{policy::long_name<S_("flag1")>,
                                 policy::description<S_("First description")>}};

    const auto r =
        root{mode{policy::long_name<S_("mode1")>,
                  flag1,
                  flag{policy::long_name<S_("flag2")>,
                       policy::description<S_("Second description")>},
                  flag{policy::short_name<'t'>,
                       policy::description<S_("Third description")>},
                  policy::router{[&](bool flag1, bool flag2, bool t) {
                      result1 = {flag1, flag2, t};
                      router_hit1 = true;
                  }}},
             mode{policy::long_name<S_("mode2")>,
                  flag1,
                  flag{policy::short_name<'b'>,
                       policy::description<S_("Fourth description")>},
                  policy::router{[&](bool flag1, bool b) {
                      result2 = {flag1, b};
                      router_hit2 = true;
                  }}},
             policy::validation::default_validator};

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
        arg_router::root<
            arg_router::flag<
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
