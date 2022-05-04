/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/counting_flag.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(counting_flag_suite)

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(
        is_tree_node_v<
            counting_flag_t<std::size_t, policy::long_name_t<S_("hello")>>>,
        "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(match_test)
{
    auto f = [](const auto& test_node,
                const parsing::token_type& token,
                bool expected_result) {
        using test_node_type = std::decay_t<decltype(test_node)>;

        const auto result = test_node.match(token, [&](const auto& node) {
            using node_type = std::decay_t<decltype(node)>;
            static_assert(std::is_same_v<node_type, test_node_type>,
                          "match fail");
        });
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{
                counting_flag<std::size_t>(policy::long_name<S_("hello")>),
                parsing::token_type{parsing::prefix_type::long_, "hello"},
                true},
            std::tuple{counting_flag<std::size_t>(policy::short_name<'h'>),
                       parsing::token_type{parsing::prefix_type::short_, "h"},
                       true},
            std::tuple{
                counting_flag<std::size_t>(policy::long_name<S_("hello")>,
                                           policy::short_name<'h'>),
                parsing::token_type{parsing::prefix_type::short_, "h"},
                true},
            std::tuple{
                counting_flag<std::size_t>(policy::long_name<S_("hello")>,
                                           policy::short_name<'h'>),
                parsing::token_type{parsing::prefix_type::long_, "hello"},
                true},
            std::tuple{
                counting_flag<std::size_t>(policy::long_name<S_("goodbye")>),
                parsing::token_type{parsing::prefix_type::long_, "hello"},
                false}});
}

BOOST_AUTO_TEST_CASE(parse_test)
{
    auto f = [&](auto node,
                 auto tokens,
                 auto expected_tokens,
                 auto expected_result,
                 const auto&... parents) {
        const auto result = node.parse(tokens, parents...);
        BOOST_CHECK_EQUAL(result, expected_result);
        BOOST_CHECK_EQUAL(tokens.pending_view(), expected_tokens);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{counting_flag<std::size_t>(policy::short_name<'h'>),
                       parsing::token_list{{parsing::prefix_type::short_, "h"}},
                       parsing::token_list{},
                       true},
            std::tuple{
                counting_flag<std::size_t>(policy::long_name<S_("foo")>),
                parsing::token_list{{parsing::prefix_type::long_, "foo"}},
                parsing::token_list{},
                true},
        });
}

BOOST_AUTO_TEST_CASE(merge_test)
{
    {
        auto node = counting_flag<std::size_t>(policy::short_name<'h'>);

        auto result = std::optional<std::size_t>{};
        node.merge(result, true);
        BOOST_REQUIRE(!!result);
        BOOST_CHECK_EQUAL(*result, 1);

        node.merge(result, true);
        BOOST_REQUIRE(!!result);
        BOOST_CHECK_EQUAL(*result, 2);

        node.merge(result, true);
        BOOST_REQUIRE(!!result);
        BOOST_CHECK_EQUAL(*result, 3);
    }

    {
        enum class enum_t { A, B, C, D };

        auto node = counting_flag<enum_t>(policy::short_name<'h'>);

        auto result = std::optional<enum_t>{};
        node.merge(result, true);
        BOOST_REQUIRE(!!result);
        BOOST_CHECK(*result == enum_t::B);

        node.merge(result, true);
        BOOST_REQUIRE(!!result);
        BOOST_CHECK(*result == enum_t::C);

        node.merge(result, true);
        BOOST_REQUIRE(!!result);
        BOOST_CHECK(*result == enum_t::D);
    }
}

BOOST_AUTO_TEST_CASE(help_test)
{
    auto f = [](const auto& node,
                auto expected_label,
                auto expected_description) {
        using node_type = std::decay_t<decltype(node)>;

        using help_data = typename node_type::template help_data_type<false>;
        using flattened_help_data =
            typename node_type::template help_data_type<true>;

        static_assert(std::is_same_v<typename help_data::label,
                                     typename flattened_help_data::label>);
        static_assert(
            std::is_same_v<typename help_data::description,
                           typename flattened_help_data::description>);
        static_assert(std::tuple_size_v<typename help_data::children> == 0);
        static_assert(
            std::tuple_size_v<typename flattened_help_data::children> == 0);

        BOOST_CHECK_EQUAL(help_data::label::get(), expected_label);
        BOOST_CHECK_EQUAL(help_data::description::get(), expected_description);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{
                counting_flag<int>(policy::short_name<'h'>,
                                   policy::long_name<S_("hello")>,
                                   policy::description<S_("A counting flag!")>),
                "--hello,-h",
                "A counting flag!"},
            std::tuple{
                counting_flag<int>(policy::long_name<S_("hello")>,
                                   policy::description<S_("A counting flag!")>),
                "--hello",
                "A counting flag!"},
            std::tuple{
                counting_flag<int>(policy::short_name<'h'>,
                                   policy::description<S_("A counting flag!")>),
                "-h",
                "A counting flag!"},
            std::tuple{counting_flag<int>(policy::short_name<'h'>), "-h", ""},
        });
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(policies_only_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/counting_flag.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto f = counting_flag<int>(
        policy::long_name<S_("hello")>,
        counting_flag<int>(policy::short_name<'b'>),
        policy::short_name<'H'>
    );
    return 0;
}
    )",
        "Counting flags must only contain policies (not other nodes)");
}

BOOST_AUTO_TEST_CASE(must_be_named_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/counting_flag.hpp"

using namespace arg_router;

int main() {
    auto f = counting_flag<int>();
    return 0;
}
    )",
        "Counting flag must have a long and/or short name policy");
}

BOOST_AUTO_TEST_CASE(must_not_have_display_name_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/counting_flag.hpp"
#include "arg_router/policy/display_name.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto f = counting_flag<int>(policy::long_name<S_("hello")>,
                                policy::display_name<S_("hello2")>);
    return 0;
}
    )",
        "Counting flag must not have a display name policy");
}

BOOST_AUTO_TEST_CASE(must_not_have_none_name_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/counting_flag.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto f = counting_flag<int>(policy::long_name<S_("hello")>,
                                policy::none_name<S_("hello2")>);
    return 0;
}
    )",
        "Counting flag must not have a none name policy");
}

BOOST_AUTO_TEST_CASE(parse_policy_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/counting_flag.hpp"
#include "arg_router/policy/custom_parser.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto f = counting_flag<int>(
                policy::long_name<S_("hello")>,
                policy::custom_parser<bool>{
                    [](std::string_view) { return true; }});
    return 0;
}
    )",
        "Counting flag does not support policies with parse or routing phases "
        "(e.g. custom_parser)");
}

BOOST_AUTO_TEST_CASE(routing_phase_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/counting_flag.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto f = counting_flag<int>(policy::long_name<S_("hello")>,
                                policy::router{[](int) {}});
    return 0;
}
    )",
        "Counting flag does not support policies with parse or routing phases "
        "(e.g. custom_parser)");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
