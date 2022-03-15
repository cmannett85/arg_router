/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/mode.hpp"
#include "arg_router/arg.hpp"
#include "arg_router/counting_flag.hpp"
#include "arg_router/dependency/alias_group.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/list.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/display_name.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/positional_arg.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
namespace ard = arg_router::dependency;
using namespace std::string_literals;

namespace
{
template <typename ChildA, typename ChildB>
constexpr void check_tree()
{
    static_assert(
        std::is_same_v<typename ChildA::label, typename ChildB::label>);
    static_assert(std::is_same_v<typename ChildA::description,
                                 typename ChildB::description>);
    static_assert(std::tuple_size_v<typename ChildA::children> ==
                  std::tuple_size_v<typename ChildB::children>);

    utility::tuple_type_iterator<typename ChildA::children>([](auto i) {
        check_tree<std::tuple_element_t<i, typename ChildA::children>,
                   std::tuple_element_t<i, typename ChildB::children>>();
    });
}

template <typename Label, typename Description, typename Children>
struct test_help_data {
    using label = Label;
    using description = Description;
    using children = Children;
};
}  // namespace

BOOST_AUTO_TEST_SUITE(mode_suite)

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(
        is_tree_node_v<
            arg_router::mode_t<flag_t<policy::long_name_t<S_("hello")>>>>,
        "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(anonymous_test)
{
    static_assert(!arg_router::mode_t<
                      policy::none_name_t<S_("mode")>,
                      flag_t<policy::long_name_t<S_("hello")>>>::is_anonymous,
                  "Fail");
    static_assert(arg_router::mode_t<
                      flag_t<policy::long_name_t<S_("hello")>>>::is_anonymous,
                  "Fail");
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
    BOOST_CHECK(tokens.pending_view().empty());

    result = false;
    tokens = parsing::token_list{{parsing::prefix_type::SHORT, "l"}};
    m.parse(tokens);
    BOOST_CHECK(result);
    BOOST_CHECK(tokens.pending_view().empty());

    result = false;
    tokens = parsing::token_list{{parsing::prefix_type::SHORT, "r"}};
    BOOST_CHECK_EXCEPTION(  //
        m.parse(tokens),
        parse_exception,
        [](const auto& e) { return e.what() == "Unknown argument: -r"s; });
    BOOST_CHECK(!result);
    BOOST_CHECK_EQUAL(tokens.pending_view().size(), 1);
}

BOOST_AUTO_TEST_CASE(anonymous_single_positional_single_count_arg_parse_test)
{
    auto result = 0;
    const auto m =
        mode(positional_arg<int>(policy::display_name<S_("hello")>,
                                 policy::description<S_("Hello arg")>,
                                 policy::fixed_count<1>),
             policy::router([&](int f1) { result = f1; }));

    auto tokens = parsing::token_list{{parsing::prefix_type::NONE, "42"}};
    m.parse(tokens);
    BOOST_CHECK_EQUAL(result, 42);
    BOOST_CHECK(tokens.pending_view().empty());

    result = 0;
    tokens = parsing::token_list{{parsing::prefix_type::NONE, "42"},
                                 {parsing::prefix_type::NONE, "84"}};
    BOOST_CHECK_EXCEPTION(  //
        m.parse(tokens),
        parse_exception,
        [](const auto& e) { return e.what() == "Unknown argument: 84"s; });
    BOOST_CHECK_EQUAL(result, 0);
    BOOST_CHECK_EQUAL(
        tokens.pending_view(),
        (parsing::token_list{{parsing::prefix_type::NONE, "84"}}));
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
    const auto m = mode(policy::none_name<S_("my-mode")>,
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
    const auto m = mode(policy::none_name<S_("my-mode")>,
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
        policy::none_name<S_("my-mode")>,
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
        {std::tuple{parsing::token_type{parsing::prefix_type::NONE, "my-mode"},
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
        mode(policy::none_name<S_("my-mode")>,
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
    const auto m = mode(policy::none_name<S_("my-mode")>, list1, list2);

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
                    false},
         std::tuple{parsing::token_type{parsing::prefix_type::SHORT, "b"},
                    false}});
}

BOOST_AUTO_TEST_CASE(nested_modes_parse_test)
{
    auto result = std::optional<std::tuple<bool, int>>{};
    const auto m = mode(policy::none_name<S_("mode1")>,
                        mode(policy::none_name<S_("mode2")>,
                             mode(policy::none_name<S_("mode3")>,
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

BOOST_AUTO_TEST_CASE(no_missing_phase_test)
{
    {
        auto result = 42;
        const auto m = mode(arg<int>(policy::long_name<S_("hello")>),
                            policy::router([&](int arg1) { result = arg1; }));

        auto tokens = parsing::token_list{};
        m.parse(tokens);
        BOOST_CHECK_EQUAL(result, 0);
        BOOST_CHECK(tokens.pending_view().empty());
    }

    {
        auto result = 3.14;
        const auto m =
            mode(arg<int>(policy::long_name<S_("hello")>),
                 policy::router([&](double arg1) { result = arg1; }));

        auto tokens = parsing::token_list{};
        m.parse(tokens);
        BOOST_CHECK_EQUAL(result, 0.0);
        BOOST_CHECK(tokens.pending_view().empty());
    }

    {
        auto result = std::vector<int>{3, 4, 5};
        const auto m = mode(
            positional_arg<std::vector<int>>(policy::display_name<S_("hello")>),
            policy::router([&](std::vector<int> arg1) { result = arg1; }));

        auto tokens = parsing::token_list{};
        m.parse(tokens);
        BOOST_CHECK_EQUAL(result, std::vector<int>{});
        BOOST_CHECK(tokens.pending_view().empty());
    }
}

BOOST_AUTO_TEST_CASE(help_test)
{
    auto f = [](const auto& node, auto flatten, auto expected) {
        using node_type = std::decay_t<decltype(node)>;
        using expected_type = std::decay_t<decltype(expected)>;

        using help_data = typename node_type::template help_data_type<flatten>;

        check_tree<help_data, expected_type>();
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{
                mode(flag(policy::long_name<S_("hello")>,
                          policy::short_name<'h'>,
                          policy::description<S_("Hello desc")>),
                     policy::router([](bool) {})),
                std::true_type{},
                test_help_data<S_(""),
                               S_(""),
                               std::tuple<test_help_data<S_("--hello,-h"),
                                                         S_("Hello desc"),
                                                         std::tuple<>>>>{}},
            std::tuple{
                mode(flag(policy::long_name<S_("hello")>,
                          policy::short_name<'h'>,
                          policy::description<S_("Hello desc")>),
                     policy::router([](bool) {})),
                std::false_type{},
                test_help_data<S_(""),
                               S_(""),
                               std::tuple<test_help_data<S_("--hello,-h"),
                                                         S_("Hello desc"),
                                                         std::tuple<>>>>{}},
            std::tuple{
                mode(flag(policy::long_name<S_("hello")>,
                          policy::short_name<'h'>,
                          policy::description<S_("Hello desc")>),
                     flag(policy::long_name<S_("flag1")>,
                          policy::short_name<'a'>,
                          policy::description<S_("Flag1 desc")>),
                     policy::router([](bool, bool) {})),
                std::true_type{},
                test_help_data<S_(""),
                               S_(""),
                               std::tuple<test_help_data<S_("--hello,-h"),
                                                         S_("Hello desc"),
                                                         std::tuple<>>,
                                          test_help_data<S_("--flag1,-a"),
                                                         S_("Flag1 desc"),
                                                         std::tuple<>>>>{}},
            std::tuple{
                mode(policy::none_name<S_("mode1")>,
                     policy::description<S_("Mode desc")>,
                     flag(policy::long_name<S_("hello")>,
                          policy::short_name<'h'>,
                          policy::description<S_("Hello desc")>),
                     flag(policy::long_name<S_("flag1")>,
                          policy::short_name<'a'>,
                          policy::description<S_("Flag1 desc")>),
                     policy::router([](bool, bool) {})),
                std::false_type{},
                test_help_data<S_("mode1"), S_("Mode desc"), std::tuple<>>{}},
            std::tuple{
                mode(policy::none_name<S_("mode1")>,
                     policy::description<S_("Mode desc")>,
                     flag(policy::long_name<S_("hello")>,
                          policy::short_name<'h'>,
                          policy::description<S_("Hello desc")>),
                     flag(policy::long_name<S_("flag1")>,
                          policy::short_name<'a'>,
                          policy::description<S_("Flag1 desc")>),
                     policy::router([](bool, bool) {})),
                std::true_type{},
                test_help_data<S_("mode1"),
                               S_("Mode desc"),
                               std::tuple<test_help_data<S_("--hello,-h"),
                                                         S_("Hello desc"),
                                                         std::tuple<>>,
                                          test_help_data<S_("--flag1,-a"),
                                                         S_("Flag1 desc"),
                                                         std::tuple<>>>>{}},
            std::tuple{
                mode(policy::none_name<S_("mode1")>,
                     policy::description<S_("Mode1 desc")>,
                     flag(policy::long_name<S_("hello")>,
                          policy::short_name<'h'>,
                          policy::description<S_("Hello desc")>),
                     mode(policy::none_name<S_("mode2")>,
                          policy::description<S_("Mode2 desc")>,
                          flag(policy::long_name<S_("goodbye")>,
                               policy::short_name<'g'>,
                               policy::description<S_("Goodbye desc")>),
                          flag(policy::long_name<S_("flag2")>,
                               policy::short_name<'b'>,
                               policy::description<S_("Flag2 desc")>)),
                     policy::router([](bool) {})),
                std::true_type{},
                test_help_data<
                    S_("mode1"),
                    S_("Mode1 desc"),
                    std::tuple<
                        test_help_data<S_("--hello,-h"),
                                       S_("Hello desc"),
                                       std::tuple<>>,
                        test_help_data<
                            S_("mode2"),
                            S_("Mode2 desc"),
                            std::tuple<test_help_data<S_("--goodbye,-g"),
                                                      S_("Goodbye desc"),
                                                      std::tuple<>>,
                                       test_help_data<S_("--flag2,-b"),
                                                      S_("Flag2 desc"),
                                                      std::tuple<>>>>>>{}},
            std::tuple{
                mode(policy::none_name<S_("mode1")>,
                     policy::description<S_("Mode1 desc")>,
                     flag(policy::long_name<S_("hello")>,
                          policy::short_name<'h'>,
                          policy::description<S_("Hello desc")>),
                     mode(policy::none_name<S_("mode2")>,
                          policy::description<S_("Mode2 desc")>,
                          flag(policy::long_name<S_("goodbye")>,
                               policy::short_name<'g'>,
                               policy::description<S_("Goodbye desc")>),
                          flag(policy::long_name<S_("flag2")>,
                               policy::short_name<'b'>,
                               policy::description<S_("Flag2 desc")>)),
                     policy::router([](bool) {})),
                std::false_type{},
                test_help_data<S_("mode1"), S_("Mode1 desc"), std::tuple<>>{}},
        });
}

BOOST_AUTO_TEST_CASE(multi_stage_alias_group_test)
{
    auto result = 0;
    const auto m =
        mode(ard::alias_group(arg<int>(policy::long_name<S_("arg")>),
                              counting_flag<int>(policy::short_name<'a'>),
                              policy::required),
             policy::router([&](int value) { result = value; }));

    auto tokens = parsing::token_list{
        parsing::token_type{parsing::prefix_type::LONG, "arg"},
        parsing::token_type{parsing::prefix_type::NONE, "5"},
        parsing::token_type{parsing::prefix_type::SHORT, "a"}};

    m.parse(tokens);

    BOOST_CHECK_EQUAL(result, 6);
    BOOST_CHECK(tokens.pending_view().empty());
}

BOOST_AUTO_TEST_CASE(multi_stage_validated_alias_group_test)
{
    auto result = 0;
    const auto m =
        mode(ard::alias_group(arg<int>(policy::long_name<S_("arg")>),
                              counting_flag<int>(policy::short_name<'a'>),
                              policy::min_max_value{1, 3},
                              policy::required),
             policy::router([&](int value) { result = value; }));

    auto f = [&](auto tokens, auto expected_result, auto fail_message) {
        result = 0;
        try {
            m.parse(tokens);
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK_EQUAL(result, expected_result);
            BOOST_CHECK(tokens.pending_view().empty());
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
        }
    };

    test::data_set(
        f,
        {
            std::tuple{
                parsing::token_list{
                    parsing::token_type{parsing::prefix_type::LONG, "arg"},
                    parsing::token_type{parsing::prefix_type::NONE, "1"}},
                1,
                ""s},
            std::tuple{
                parsing::token_list{
                    parsing::token_type{parsing::prefix_type::LONG, "arg"},
                    parsing::token_type{parsing::prefix_type::NONE, "3"}},
                3,
                ""s},
            std::tuple{
                parsing::token_list{
                    parsing::token_type{parsing::prefix_type::LONG, "arg"},
                    parsing::token_type{parsing::prefix_type::NONE, "0"}},
                0,
                "Minimum value not reached: Alias Group: --arg,-a"s},
            std::tuple{
                parsing::token_list{
                    parsing::token_type{parsing::prefix_type::LONG, "arg"},
                    parsing::token_type{parsing::prefix_type::NONE, "5"}},
                0,
                "Maximum value exceeded: Alias Group: --arg,-a"s},
            std::tuple{
                parsing::token_list{
                    parsing::token_type{parsing::prefix_type::SHORT, "a"}},
                1,
                ""s},
            std::tuple{
                parsing::token_list{
                    parsing::token_type{parsing::prefix_type::SHORT, "a"},
                    parsing::token_type{parsing::prefix_type::SHORT, "a"}},
                2,
                ""s},
            std::tuple{parsing::token_list{},
                       0,
                       "Missing required argument: Alias Group: --arg,-a"s},
            std::tuple{
                parsing::token_list{
                    parsing::token_type{parsing::prefix_type::SHORT, "a"},
                    parsing::token_type{parsing::prefix_type::SHORT, "a"},
                    parsing::token_type{parsing::prefix_type::SHORT, "a"},
                    parsing::token_type{parsing::prefix_type::SHORT, "a"}},
                0,
                "Maximum value exceeded: Alias Group: --arg,-a"s},
            std::tuple{
                parsing::token_list{
                    parsing::token_type{parsing::prefix_type::LONG, "arg"},
                    parsing::token_type{parsing::prefix_type::NONE, "2"},
                    parsing::token_type{parsing::prefix_type::SHORT, "a"},
                    parsing::token_type{parsing::prefix_type::SHORT, "a"}},
                0,
                "Maximum value exceeded: Alias Group: --arg,-a"s},
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
        "Mode must have at least one child node");
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

BOOST_AUTO_TEST_CASE(must_not_have_a_long_name_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    const auto m = mode(policy::long_name<S_("my-mode")>,
                        flag(policy::long_name<S_("hello")>));
    return 0;
}
    )",
        "Mode must not have a long name policy");
}

BOOST_AUTO_TEST_CASE(must_not_have_a_short_name_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    const auto m = mode(policy::short_name<'l'>,
                        flag(policy::long_name<S_("hello")>));
    return 0;
}
    )",
        "Mode must not have a short name policy");
}

BOOST_AUTO_TEST_CASE(must_not_have_a_display_name_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/display_name.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    const auto m = mode(policy::display_name<S_("mode")>,
                        flag(policy::long_name<S_("hello")>));
    return 0;
}
    )",
        "Mode must not have a display name policy");
}

BOOST_AUTO_TEST_CASE(must_not_have_value_separator_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/policy/value_separator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto f = mode(policy::none_name<S_("mode")>,
                  policy::value_separator<'='>,
                  flag(policy::long_name<S_("hello")>));
    return 0;
}
    )",
        "Mode must not have a value separator policy");
}

BOOST_AUTO_TEST_CASE(anonymous_child_mode_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
namespace
{
template <typename... Params>
class stub_node : public tree_node<Params...>
{
public:
    constexpr explicit stub_node(Params... params) :
        tree_node<Params...>{std::move(params)...}
    {
    }

    void parse(parsing::token_list& tokens) const
    {
        std::get<0>(this->children()).parse(tokens, *this);
    }
};
} // namespace

int main() {
    const auto m = stub_node(
                        mode(policy::none_name<S_("mode")>,
                             mode(flag(policy::long_name<S_("hello")>),
                                  policy::router([&](bool) {}))));

    auto tokens = parsing::token_list{{parsing::prefix_type::LONG, "hello"}};
    m.parse(tokens);
    return 0;
}
    )",
        "Anonymous modes can only exist under the root");
}

BOOST_AUTO_TEST_CASE(anonymous_mode_cannot_have_a_child_mode_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    [[maybe_unused]] const auto m = mode(
                    flag(policy::long_name<S_("flag")>),
                    mode(policy::none_name<S_("mode")>,
                         flag(policy::long_name<S_("hello")>)));
    return 0;
}
    )",
        "Anonymous mode cannot have a child mode");
}

BOOST_AUTO_TEST_CASE(mode_has_router_or_all_children_are_modes_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
namespace
{
template <typename... Params>
class stub_node : public tree_node<Params...>
{
public:
    constexpr explicit stub_node(Params... params) :
        tree_node<Params...>{std::move(params)...}
    {
    }

    void parse(parsing::token_list& tokens) const
    {
        std::get<0>(this->children()).parse(tokens, *this);
    }
};
} // namespace

int main() {
    const auto m = stub_node(
                        mode(policy::none_name<S_("mode")>,
                             flag(policy::long_name<S_("f1")>),
                             mode(flag(policy::long_name<S_("f2")>),
                                  policy::router([&](bool) {}))));
                             

    auto tokens = parsing::token_list{{parsing::prefix_type::LONG, "hello"}};
    m.parse(tokens);
    return 0;
}
    )",
        "Mode must have a router or all its children are modes");
}

BOOST_AUTO_TEST_CASE(non_mode_children_cannot_have_children_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    const auto m = mode(flag(policy::long_name<S_("hello")>,
                             policy::router([&](bool) {})),
                        policy::router([&](bool) {}));
    return 0;
}
    )",
        "Non-mode children cannot have routing");
}

BOOST_AUTO_TEST_CASE(pre_parse_phase_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    const auto m = mode(policy::alias(policy::long_name<S_("other-mode")>),
                        flag(policy::long_name<S_("hello")>));
    return 0;
}
    )",
        "Mode does not support policies with pre-parse, parse, validation, "
        "or missing phases; as it delegates those to its children");
}

BOOST_AUTO_TEST_CASE(parse_phase_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/custom_parser.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    const auto m = mode(policy::custom_parser<int>{[](auto) { return false; }},
                        flag(policy::long_name<S_("hello")>));
    return 0;
}
    )",
        "Mode does not support policies with pre-parse, parse, validation, "
        "or missing phases; as it delegates those to its children");
}

BOOST_AUTO_TEST_CASE(validation_phase_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    const auto m = mode(policy::min_max_value{1, 3},
                        flag(policy::long_name<S_("hello")>));
    return 0;
}
    )",
        "Mode does not support policies with pre-parse, parse, validation, "
        "or missing phases; as it delegates those to its children");
}

BOOST_AUTO_TEST_CASE(missing_phase_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    const auto m = mode(policy::required,
                        flag(policy::long_name<S_("hello")>));
    return 0;
}
    )",
        "Mode does not support policies with pre-parse, parse, validation, "
        "or missing phases; as it delegates those to its children");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
