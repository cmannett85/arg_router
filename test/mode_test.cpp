#include "arg_router/mode.hpp"
#include "arg_router/arg.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/list.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace std::string_literals;

BOOST_AUTO_TEST_SUITE(mode_suite)

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(is_tree_node_v<arg_router::mode_t<flag_t<>>>,
                  "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(anonymous_single_flag_match_test)
{
    const auto m = mode(flag(policy::long_name<S_("hello")>,
                             policy::short_name<'l'>,
                             policy::description<S_("Hello arg")>));

    auto f = [&](auto token, auto expected_result) {
        auto visitor_hit = false;
        const auto result = m.match(token, [&](const auto& node) {
            static_assert(std::is_same_v<std::decay_t<decltype(node)>,
                                         std::decay_t<decltype(m)>>,
                          "match fail");
            visitor_hit = true;
        });
        BOOST_CHECK_EQUAL(result, expected_result);
        BOOST_CHECK_EQUAL(visitor_hit, expected_result);
    };

    test::data_set(
        f,
        {
            std::tuple{parsing::token_type{parsing::prefix_type::LONG, "hello"},
                       true},
            std::tuple{
                parsing::token_type{parsing::prefix_type::LONG, "goodbye"},
                true},
        });
}

BOOST_AUTO_TEST_CASE(anonymous_single_flag_parse_test)
{
    auto result = false;
    const auto m = mode(flag(policy::long_name<S_("hello")>,
                             policy::short_name<'l'>,
                             policy::description<S_("Hello arg")>),
                        policy::router([&](bool f1) { result = f1; }));

    auto tokens = parsing::token_list{{parsing::prefix_type::LONG, "hello"}};
    m.parse(tokens);
    BOOST_CHECK(result);
    BOOST_CHECK(tokens.empty());

    result = false;
    tokens = parsing::token_list{{parsing::prefix_type::SHORT, "l"}};
    m.parse(tokens);
    BOOST_CHECK(result);
    BOOST_CHECK(tokens.empty());

    result = false;
    tokens = parsing::token_list{{parsing::prefix_type::SHORT, "r"}};
    BOOST_CHECK_EXCEPTION(  //
        m.parse(tokens),
        parse_exception,
        [](const auto& e) { return e.what() == "Unknown argument: -r"s; });
    BOOST_CHECK(!result);
    BOOST_CHECK_EQUAL(tokens.size(), 1);
}

BOOST_AUTO_TEST_CASE(anonymous_triple_flag_match_test)
{
    const auto m = mode(
        flag(policy::long_name<S_("hello")>,
             policy::short_name<'l'>,
             policy::description<S_("Hello arg")>),
        flag(policy::long_name<S_("foo")>, policy::description<S_("Foo arg")>),
        flag(policy::short_name<'b'>, policy::description<S_("b arg")>));

    auto f = [&](auto token, auto expected_result) {
        auto visitor_hit = false;
        const auto result = m.match(token, [&](const auto& node) {
            static_assert(std::is_same_v<std::decay_t<decltype(node)>,
                                         std::decay_t<decltype(m)>>,
                          "match fail");
            visitor_hit = true;
        });
        BOOST_CHECK_EQUAL(result, expected_result);
        BOOST_CHECK_EQUAL(visitor_hit, expected_result);
    };

    test::data_set(
        f,
        {std::tuple{parsing::token_type{parsing::prefix_type::LONG, "hello"},
                    true},
         std::tuple{parsing::token_type{parsing::prefix_type::LONG, "foo"},
                    true},
         std::tuple{parsing::token_type{parsing::prefix_type::SHORT, "b"},
                    true},
         std::tuple{parsing::token_type{parsing::prefix_type::SHORT, "g"},
                    true}});
}

BOOST_AUTO_TEST_CASE(anonymous_triple_child_parse_test)
{
    auto result = std::optional<std::tuple<bool, int, bool>>{};
    const auto m =
        mode(flag(policy::long_name<S_("hello")>,
                  policy::short_name<'l'>,
                  policy::description<S_("Hello arg")>),
             arg<int>(policy::long_name<S_("foo")>,
                      policy::description<S_("Foo arg")>,
                      policy::default_value{42}),
             flag(policy::short_name<'b'>, policy::description<S_("b arg")>),
             policy::router([&](bool f1, int f2, bool f3) {
                 result = std::tuple{f1, f2, f3};
             }));

    auto f = [&](auto tokens, auto expected_result, auto fail_message) {
        result.reset();
        try {
            m.parse(tokens);
            BOOST_CHECK(fail_message.empty());
            BOOST_REQUIRE(result);
            BOOST_CHECK_EQUAL(std::get<0>(*result),
                              std::get<0>(expected_result));
            BOOST_CHECK_EQUAL(std::get<1>(*result),
                              std::get<1>(expected_result));
            BOOST_CHECK_EQUAL(std::get<2>(*result),
                              std::get<2>(expected_result));
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
            BOOST_CHECK(!result);
        }
    };

    test::data_set(
        f,
        {std::tuple{parsing::token_list{}, std::tuple{false, 42, false}, ""s},
         std::tuple{parsing::token_list{{parsing::prefix_type::LONG, "hello"}},
                    std::tuple{true, 42, false},
                    ""s},
         std::tuple{parsing::token_list{{parsing::prefix_type::SHORT, "l"}},
                    std::tuple{true, 42, false},
                    ""s},
         std::tuple{parsing::token_list{{parsing::prefix_type::LONG, "foo"},
                                        {parsing::prefix_type::NONE, "13"}},
                    std::tuple{false, 13, false},
                    ""s},
         std::tuple{parsing::token_list{{parsing::prefix_type::SHORT, "b"}},
                    std::tuple{false, 42, true},
                    ""s},
         std::tuple{parsing::token_list{{parsing::prefix_type::LONG, "hello"},
                                        {parsing::prefix_type::SHORT, "b"}},
                    std::tuple{true, 42, true},
                    ""s},
         std::tuple{parsing::token_list{{parsing::prefix_type::SHORT, "l"},
                                        {parsing::prefix_type::SHORT, "b"},
                                        {parsing::prefix_type::LONG, "foo"},
                                        {parsing::prefix_type::NONE, "48"}},
                    std::tuple{true, 48, true},
                    ""s},
         std::tuple{parsing::token_list{{parsing::prefix_type::LONG, "hello"},
                                        {parsing::prefix_type::SHORT, "l"}},
                    std::tuple{false, 42, false},
                    "Argument has already been set: -l"s},
         std::tuple{
             parsing::token_list{{parsing::prefix_type::LONG, "goodbye"}},
             std::tuple{false, 42, false},
             "Unknown argument: --goodbye"s}});
}

BOOST_AUTO_TEST_CASE(named_single_flag_match_test)
{
    const auto m = mode(policy::long_name<S_("my-mode")>,
                        flag(policy::long_name<S_("hello")>,
                             policy::short_name<'l'>,
                             policy::description<S_("Hello arg")>));

    auto f = [&](auto token, auto expected_result) {
        auto visitor_hit = false;
        const auto result = m.match(token, [&](const auto& node) {
            static_assert(std::is_same_v<std::decay_t<decltype(node)>,
                                         std::decay_t<decltype(m)>>,
                          "match fail");
            visitor_hit = true;
        });
        BOOST_CHECK_EQUAL(result, expected_result);
        BOOST_CHECK_EQUAL(visitor_hit, expected_result);
    };

    test::data_set(
        f,
        {std::tuple{parsing::token_type{parsing::prefix_type::NONE, "my-mode"},
                    true},
         std::tuple{parsing::token_type{parsing::prefix_type::LONG, "hello"},
                    false},
         std::tuple{parsing::token_type{parsing::prefix_type::SHORT, "l"},
                    false}});
}

BOOST_AUTO_TEST_CASE(named_single_flag_parse_test)
{
    auto result = std::optional<bool>{};
    const auto m = mode(policy::long_name<S_("my-mode")>,
                        flag(policy::long_name<S_("hello")>,
                             policy::short_name<'l'>,
                             policy::description<S_("Hello arg")>),
                        policy::router([&](bool f1) { result = f1; }));

    auto f = [&](auto tokens, auto expected_result, auto fail_message) {
        result.reset();
        try {
            m.parse(tokens);
            BOOST_CHECK(fail_message.empty());
            BOOST_REQUIRE(result);
            BOOST_CHECK_EQUAL(*result, expected_result);
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
            BOOST_CHECK(!result);
        }
    };

    test::data_set(
        f,
        {
            std::tuple{
                parsing::token_list{{parsing::prefix_type::NONE, "my-mode"}},
                std::optional<bool>{false},
                ""s},
            std::tuple{
                parsing::token_list{{parsing::prefix_type::NONE, "my-mode"},
                                    {parsing::prefix_type::LONG, "hello"}},
                std::optional<bool>{true},
                ""s},
            std::tuple{
                parsing::token_list{{parsing::prefix_type::NONE, "my-mode"},
                                    {parsing::prefix_type::SHORT, "l"}},
                std::optional<bool>{true},
                ""s},
            std::tuple{
                parsing::token_list{{parsing::prefix_type::NONE, "my-mode"},
                                    {parsing::prefix_type::LONG, "goodbye"}},
                std::optional<bool>{},
                "Unknown argument: --goodbye"s},
        });
}

BOOST_AUTO_TEST_CASE(named_triple_flag_match_test)
{
    const auto m = mode(
        policy::long_name<S_("my-mode")>,
        flag(policy::long_name<S_("hello")>,
             policy::short_name<'l'>,
             policy::description<S_("Hello arg")>),
        flag(policy::long_name<S_("foo")>, policy::description<S_("Foo arg")>),
        flag(policy::short_name<'b'>, policy::description<S_("b arg")>));

    auto f = [&](auto token, auto expected_result) {
        auto visitor_hit = false;
        const auto result = m.match(token, [&](const auto& node) {
            static_assert(std::is_same_v<std::decay_t<decltype(node)>,
                                         std::decay_t<decltype(m)>>,
                          "match fail");
            visitor_hit = true;
        });
        BOOST_CHECK_EQUAL(result, expected_result);
        BOOST_CHECK_EQUAL(visitor_hit, expected_result);
    };

    test::data_set(
        f,
        {std::tuple{parsing::token_type{parsing::prefix_type::LONG, "my-mode"},
                    true},
         std::tuple{parsing::token_type{parsing::prefix_type::LONG, "hello"},
                    false},
         std::tuple{parsing::token_type{parsing::prefix_type::SHORT, "l"},
                    false},
         std::tuple{parsing::token_type{parsing::prefix_type::SHORT, "b"},
                    false}});
}

BOOST_AUTO_TEST_CASE(named_triple_arg_parse_test)
{
    auto result = std::optional<std::tuple<bool, int, bool>>{};
    const auto m =
        mode(policy::long_name<S_("my-mode")>,
             flag(policy::long_name<S_("hello")>,
                  policy::short_name<'l'>,
                  policy::description<S_("Hello arg")>),
             arg<int>(policy::long_name<S_("foo")>,
                      policy::description<S_("Foo arg")>,
                      policy::default_value{42}),
             flag(policy::short_name<'b'>, policy::description<S_("b arg")>),
             policy::router([&](bool f1, int f2, bool f3) {
                 result = std::tuple{f1, f2, f3};
             }));

    auto f = [&](auto tokens, auto expected_result, auto fail_message) {
        result.reset();
        try {
            m.parse(tokens);
            BOOST_CHECK(fail_message.empty());
            BOOST_REQUIRE(result);
            BOOST_CHECK_EQUAL(std::get<0>(*result),
                              std::get<0>(expected_result));
            BOOST_CHECK_EQUAL(std::get<1>(*result),
                              std::get<1>(expected_result));
            BOOST_CHECK_EQUAL(std::get<2>(*result),
                              std::get<2>(expected_result));
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
            BOOST_CHECK(!result);
        }
    };

    test::data_set(
        f,
        {std::tuple{
             parsing::token_list{{parsing::prefix_type::LONG, "my-mode"}},
             std::tuple{false, 42, false},
             ""s},
         std::tuple{parsing::token_list{{parsing::prefix_type::LONG, "my-mode"},
                                        {parsing::prefix_type::LONG, "hello"}},
                    std::tuple{true, 42, false},
                    ""s},
         std::tuple{parsing::token_list{{parsing::prefix_type::LONG, "my-mode"},
                                        {parsing::prefix_type::SHORT, "l"}},
                    std::tuple{true, 42, false},
                    ""s},
         std::tuple{parsing::token_list{{parsing::prefix_type::LONG, "my-mode"},
                                        {parsing::prefix_type::LONG, "foo"},
                                        {parsing::prefix_type::NONE, "13"}},
                    std::tuple{false, 13, false},
                    ""s},
         std::tuple{parsing::token_list{{parsing::prefix_type::LONG, "my-mode"},
                                        {parsing::prefix_type::SHORT, "b"}},
                    std::tuple{false, 42, true},
                    ""s},
         std::tuple{parsing::token_list{{parsing::prefix_type::LONG, "my-mode"},
                                        {parsing::prefix_type::LONG, "hello"},
                                        {parsing::prefix_type::SHORT, "b"}},
                    std::tuple{true, 42, true},
                    ""s},
         std::tuple{parsing::token_list{{parsing::prefix_type::LONG, "my-mode"},
                                        {parsing::prefix_type::SHORT, "l"},
                                        {parsing::prefix_type::SHORT, "b"},
                                        {parsing::prefix_type::LONG, "foo"},
                                        {parsing::prefix_type::NONE, "48"}},
                    std::tuple{true, 48, true},
                    ""s},
         std::tuple{parsing::token_list{{parsing::prefix_type::LONG, "my-mode"},
                                        {parsing::prefix_type::LONG, "hello"},
                                        {parsing::prefix_type::SHORT, "l"}},
                    std::tuple{false, 42, false},
                    "Argument has already been set: -l"s},
         std::tuple{
             parsing::token_list{{parsing::prefix_type::LONG, "my-mode"},
                                 {parsing::prefix_type::LONG, "goodbye"}},
             std::tuple{false, 42, false},
             "Unknown argument: --goodbye"s}});
}

BOOST_AUTO_TEST_CASE(anonymous_triple_flag_single_list_match_test)
{
    const auto flags = list{
        flag(policy::long_name<S_("hello")>,
             policy::short_name<'l'>,
             policy::description<S_("Hello arg")>),
        flag(policy::long_name<S_("foo")>, policy::description<S_("Foo arg")>),
        flag(policy::short_name<'b'>, policy::description<S_("b arg")>)};
    const auto m = mode(flags);

    auto f = [&](auto token, auto expected_result) {
        auto visitor_hit = false;
        const auto result = m.match(token, [&](const auto& node) {
            static_assert(std::is_same_v<std::decay_t<decltype(node)>,
                                         std::decay_t<decltype(m)>>,
                          "match fail");
            visitor_hit = true;
        });
        BOOST_CHECK_EQUAL(result, expected_result);
        BOOST_CHECK_EQUAL(visitor_hit, expected_result);
    };

    test::data_set(
        f,
        {std::tuple{parsing::token_type{parsing::prefix_type::LONG, "hello"},
                    true},
         std::tuple{parsing::token_type{parsing::prefix_type::LONG, "foo"},
                    true},
         std::tuple{parsing::token_type{parsing::prefix_type::SHORT, "b"},
                    true},
         std::tuple{parsing::token_type{parsing::prefix_type::SHORT, "g"},
                    true}});
}

BOOST_AUTO_TEST_CASE(named_triple_flag_double_list_match_test)
{
    const auto list1 = list{
        flag(policy::long_name<S_("hello")>,
             policy::short_name<'l'>,
             policy::description<S_("Hello arg")>),
        flag(policy::long_name<S_("foo")>, policy::description<S_("Foo arg")>)};
    const auto list2 =
        list{flag(policy::short_name<'b'>, policy::description<S_("b arg")>)};
    const auto m = mode(policy::long_name<S_("my-mode")>, list1, list2);

    auto f = [&](auto token, auto expected_result) {
        auto visitor_hit = false;
        const auto result = m.match(token, [&](const auto& node) {
            static_assert(std::is_same_v<std::decay_t<decltype(node)>,
                                         std::decay_t<decltype(m)>>,
                          "match fail");
            visitor_hit = true;
        });
        BOOST_CHECK_EQUAL(result, expected_result);
        BOOST_CHECK_EQUAL(visitor_hit, expected_result);
    };

    test::data_set(
        f,
        {std::tuple{parsing::token_type{parsing::prefix_type::LONG, "my-mode"},
                    true},
         std::tuple{parsing::token_type{parsing::prefix_type::LONG, "hello"},
                    false},
         std::tuple{parsing::token_type{parsing::prefix_type::SHORT, "l"},
                    false},
         std::tuple{parsing::token_type{parsing::prefix_type::SHORT, "b"},
                    false}});
}

BOOST_AUTO_TEST_CASE(nested_modes_parse_test)
{
    auto result = std::optional<std::tuple<bool, int>>{};
    const auto m = mode(policy::long_name<S_("mode1")>,
                        mode(policy::long_name<S_("mode2")>,
                             mode(policy::long_name<S_("mode3")>,
                                  flag(policy::long_name<S_("hello")>,
                                       policy::short_name<'l'>,
                                       policy::description<S_("Hello arg")>),
                                  arg<int>(policy::long_name<S_("foo")>,
                                           policy::description<S_("Foo arg")>,
                                           policy::default_value{42}),
                                  policy::router([&](bool f1, int f2) {
                                      result = std::tuple{f1, f2};
                                  }))));

    auto f = [&](auto tokens, auto expected_result, auto fail_message) {
        result.reset();
        try {
            m.parse(tokens);
            BOOST_CHECK(fail_message.empty());
            BOOST_REQUIRE(result);
            BOOST_CHECK_EQUAL(std::get<0>(*result),
                              std::get<0>(expected_result));
            BOOST_CHECK_EQUAL(std::get<1>(*result),
                              std::get<1>(expected_result));
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
            BOOST_CHECK(!result);
        }
    };

    test::data_set(
        f,
        {
            std::tuple{
                parsing::token_list{{parsing::prefix_type::NONE, "mode1"}},
                std::tuple{false, 42},
                "Mode requires arguments: mode1"s},
            std::tuple{
                parsing::token_list{{parsing::prefix_type::NONE, "mode1"},
                                    {parsing::prefix_type::NONE, "mode2"}},
                std::tuple{false, 42},
                "Mode requires arguments: mode2"s},
            std::tuple{
                parsing::token_list{{parsing::prefix_type::NONE, "mode1"},
                                    {parsing::prefix_type::NONE, "mode9"}},
                std::tuple{false, 42},
                "Unknown argument: mode9"s},
            std::tuple{
                parsing::token_list{{parsing::prefix_type::NONE, "mode1"},
                                    {parsing::prefix_type::NONE, "mode2"},
                                    {parsing::prefix_type::NONE, "mode3"}},
                std::tuple{false, 42},
                ""s},
            std::tuple{
                parsing::token_list{{parsing::prefix_type::NONE, "mode1"},
                                    {parsing::prefix_type::NONE, "mode2"},
                                    {parsing::prefix_type::NONE, "mode3"},
                                    {parsing::prefix_type::LONG, "hello"}},
                std::tuple{true, 42},
                ""s},
            std::tuple{
                parsing::token_list{{parsing::prefix_type::NONE, "mode1"},
                                    {parsing::prefix_type::NONE, "mode2"},
                                    {parsing::prefix_type::NONE, "mode3"},
                                    {parsing::prefix_type::LONG, "hello"},
                                    {parsing::prefix_type::LONG, "foo"},
                                    {parsing::prefix_type::NONE, "13"}},
                std::tuple{true, 13},
                ""s},
        });
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(no_children_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/mode.hpp"

using namespace arg_router;

int main() {
    const auto m = mode();
    return 0;
}
    )",
        "mode_t must have at least one child node");
}

BOOST_AUTO_TEST_CASE(anonymous_modes_must_have_routing_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto tokens = parsing::token_list{{parsing::prefix_type::LONG, "hello"}};
    const auto m = mode(flag(policy::long_name<S_("hello")>));
    m.parse(tokens);
    return 0;
}
    )",
        "Anonymous modes must have routing");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
