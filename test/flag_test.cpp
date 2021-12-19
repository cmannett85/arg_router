﻿#include "arg_router/flag.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(flag_suite)

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(is_tree_node_v<flag_t<policy::long_name_t<S_("hello")>>>,
                  "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(policies_test)
{
    auto f = flag(policy::long_name<S_("hello")>, policy::short_name<'H'>);
    static_assert(f.long_name() == "hello"sv, "Long name test fail");
    static_assert(f.short_name() == "H", "Short name test fail");
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
            std::tuple{flag(policy::long_name<S_("hello")>),
                       parsing::token_type{parsing::prefix_type::LONG, "hello"},
                       true},
            std::tuple{flag(policy::short_name<'h'>),
                       parsing::token_type{parsing::prefix_type::SHORT, "h"},
                       true},
            std::tuple{
                flag(policy::long_name<S_("hello")>, policy::short_name<'h'>),
                parsing::token_type{parsing::prefix_type::SHORT, "h"},
                true},
            std::tuple{
                flag(policy::long_name<S_("hello")>, policy::short_name<'h'>),
                parsing::token_type{parsing::prefix_type::LONG, "hello"},
                true},
            std::tuple{flag(policy::long_name<S_("goodbye")>),
                       parsing::token_type{parsing::prefix_type::LONG, "hello"},
                       false}});
}

BOOST_AUTO_TEST_CASE(parse_test)
{
    auto router_hit = false;
    auto f = [&](auto node,
                 auto tokens,
                 auto expected_tokens,
                 auto expected_result,
                 auto expected_router_hit,
                 auto... parents) {
        router_hit = false;
        const auto result = node.parse(tokens, parents...);
        BOOST_CHECK_EQUAL(result, expected_result);
        BOOST_CHECK_EQUAL(tokens, expected_tokens);
        BOOST_CHECK_EQUAL(router_hit, expected_router_hit);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{flag(policy::short_name<'h'>),
                       parsing::token_list{{parsing::prefix_type::SHORT, "h"}},
                       parsing::token_list{},
                       true,
                       false},
            std::tuple{flag(policy::short_name<'a'>,
                            policy::alias(policy::short_name<'h'>,
                                          policy::short_name<'g'>)),
                       parsing::token_list{{parsing::prefix_type::SHORT, "a"}},
                       parsing::token_list{
                           {parsing::prefix_type::SHORT, "h"},
                           {parsing::prefix_type::SHORT, "g"},
                       },
                       true,
                       false,
                       mode(flag(policy::short_name<'a'>,
                                 policy::alias(policy::short_name<'h'>,
                                               policy::short_name<'g'>)),
                            flag(policy::short_name<'h'>),
                            flag(policy::short_name<'g'>))},
            std::tuple{flag(policy::short_name<'a'>,  //
                            policy::router{[&](bool result) {
                                BOOST_CHECK(result);
                                router_hit = true;
                            }}),
                       parsing::token_list{{parsing::prefix_type::SHORT, "a"}},
                       parsing::token_list{},
                       true,
                       true}});
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(policies_only_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto f = flag(
        policy::long_name<S_("hello")>,
        flag(policy::short_name<'b'>),
        policy::short_name<'H'>
    );
    return 0;
}
    )",
        "Flags must only contain policies (not other nodes)");
}

BOOST_AUTO_TEST_CASE(no_custom_parser_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/policy/custom_parser.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto f = flag(
        policy::long_name<S_("hello")>,
        policy::custom_parser<bool>{[](std::string_view) { return false; }}
    );

    auto tokens = parsing::token_list{{parsing::prefix_type::LONG, "hello"}};
    f.parse(tokens);
    return 0;
}
    )",
        "Flag cannot have a custom parser");
}

BOOST_AUTO_TEST_CASE(must_be_named_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"

using namespace arg_router;

int main() {
    auto f = flag();
    return 0;
}
    )",
        "Flag must have a long and/or short name policy");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()