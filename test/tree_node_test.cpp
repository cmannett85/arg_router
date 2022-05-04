/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/tree_node.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/parsing.hpp"
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/default_value.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/short_form_expander.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/policy/value_separator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace std::string_view_literals;

namespace
{
template <typename... Params>
class stub_node : public tree_node<Params...>
{
public:
    using value_type = bool;

    using tree_node<Params...>::parse;

    constexpr explicit stub_node(Params... params) :
        tree_node<Params...>{std::move(params)...}
    {
    }

    template <typename Fn>
    bool match(const parsing::token_type& token, const Fn& visitor) const
    {
        if (parsing::match<stub_node>(token)) {
            visitor(*this);
            return true;
        }

        return false;
    }

    template <typename... Parents>
    bool pre_parse(vector<parsing::token_type>& args,
                   parsing::token_list& tokens,
                   const Parents&... parents) const
    {
        return tree_node<Params...>::pre_parse(args, tokens, *this, parents...);
    }
};
}  // namespace

BOOST_AUTO_TEST_SUITE(tree_node_suite)

BOOST_AUTO_TEST_CASE(mixed_tree_node_types_test)
{
    using tn = tree_node<policy::long_name_t<S_("hello")>,
                         tree_node<policy::long_name_t<S_("child")>>,
                         policy::short_name_t<traits::integral_constant<'A'>>>;
    static_assert(
        std::is_same_v<
            typename tn::parameters_type,
            std::tuple<policy::long_name_t<S_("hello")>,
                       tree_node<policy::long_name_t<S_("child")>>,
                       policy::short_name_t<traits::integral_constant<'A'>>>>,
        "parameters_type does not match");

    static_assert(
        std::is_same_v<
            typename tn::policies_type,
            std::tuple<policy::long_name_t<S_("hello")>,
                       policy::short_name_t<traits::integral_constant<'A'>>>>,
        "policies_type does not match");

    static_assert(
        std::is_same_v<typename tn::children_type,
                       std::tuple<tree_node<policy::long_name_t<S_("child")>>>>,
        "children_type does not match");
}

BOOST_AUTO_TEST_CASE(only_policies_tree_node_types_test)
{
    using tn = tree_node<policy::long_name_t<S_("hello")>,
                         policy::short_name_t<traits::integral_constant<'A'>>>;
    static_assert(
        std::is_same_v<
            typename tn::parameters_type,
            std::tuple<policy::long_name_t<S_("hello")>,
                       policy::short_name_t<traits::integral_constant<'A'>>>>,
        "parameters_type does not match");

    static_assert(
        std::is_same_v<
            typename tn::policies_type,
            std::tuple<policy::long_name_t<S_("hello")>,
                       policy::short_name_t<traits::integral_constant<'A'>>>>,
        "policies_type does not match");

    static_assert(std::is_same_v<typename tn::children_type, std::tuple<>>,
                  "children_type does not match");
}

BOOST_AUTO_TEST_CASE(one_child_tree_node_types_test)
{
    using tn =
        tree_node<flag_t<policy::long_name_t<S_("hello")>,
                         policy::short_name_t<traits::integral_constant<'A'>>>>;
    static_assert(
        std::is_same_v<
            typename tn::parameters_type,
            std::tuple<
                flag_t<policy::long_name_t<S_("hello")>,
                       policy::short_name_t<traits::integral_constant<'A'>>>>>,
        "parameters_type does not match");

    static_assert(std::is_same_v<typename tn::policies_type, std::tuple<>>,
                  "policies_type does not match");

    static_assert(
        std::is_same_v<
            typename tn::children_type,
            std::tuple<
                flag_t<policy::long_name_t<S_("hello")>,
                       policy::short_name_t<traits::integral_constant<'A'>>>>>,
        "children_type does not match");
}

BOOST_AUTO_TEST_CASE(two_children_tree_node_types_test)
{
    using tn =
        tree_node<flag_t<policy::long_name_t<S_("hello")>,
                         policy::short_name_t<traits::integral_constant<'A'>>>,
                  flag_t<policy::short_name_t<traits::integral_constant<'B'>>>>;
    static_assert(
        std::is_same_v<
            typename tn::parameters_type,
            std::tuple<
                flag_t<policy::long_name_t<S_("hello")>,
                       policy::short_name_t<traits::integral_constant<'A'>>>,
                flag_t<policy::short_name_t<traits::integral_constant<'B'>>>>>,
        "parameters_type does not match");

    static_assert(std::is_same_v<typename tn::policies_type, std::tuple<>>,
                  "policies_type does not match");

    static_assert(
        std::is_same_v<
            typename tn::children_type,
            std::tuple<
                flag_t<policy::long_name_t<S_("hello")>,
                       policy::short_name_t<traits::integral_constant<'A'>>>,
                flag_t<policy::short_name_t<traits::integral_constant<'B'>>>>>,
        "children_type does not match");
}

BOOST_AUTO_TEST_CASE(empty_tree_node_types_test)
{
    using tn = tree_node<>;
    static_assert(std::is_same_v<typename tn::parameters_type, std::tuple<>>,
                  "parameters_type does not match");

    static_assert(std::is_same_v<typename tn::policies_type, std::tuple<>>,
                  "policies_type does not match");

    static_assert(std::is_same_v<typename tn::children_type, std::tuple<>>,
                  "children_type does not match");
}

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(!is_tree_node_v<float>, "Should not be a tree_node");
    static_assert(!is_tree_node_v<std::vector<float>>,
                  "Should not be a tree_node");

    static_assert(is_tree_node_v<tree_node<float>>, "Should be a tree_node");
    static_assert(is_tree_node_v<tree_node<float, int, double>>,
                  "Should be a tree_node");
    static_assert(is_tree_node_v<tree_node<float, int, double>>,
                  "Should be a tree_node");
    static_assert(is_tree_node_v<tree_node<policy::long_name_t<S_("hello")>>>,
                  "Should be a tree_node");
    static_assert(is_tree_node_v<tree_node<policy::long_name_t<S_("hello")>,
                                           policy::long_name_t<S_("goodbye")>>>,
                  "Should be a tree_node");
}

BOOST_AUTO_TEST_CASE(priority_ordered_policies_type_test)
{
    using tn = tree_node<
        policy::long_name_t<S_("hello")>,
        policy::alias_t<policy::short_name_t<traits::integral_constant<'A'>>>,
        policy::short_name_t<traits::integral_constant<'A'>>,
        policy::default_value<int>>;

    static_assert(
        std::is_same_v<
            typename tn::priority_ordered_policies_type,
            std::tuple<policy::default_value<int>,
                       policy::alias_t<policy::short_name_t<
                           traits::integral_constant<'A'>>>,
                       policy::long_name_t<S_("hello")>,
                       policy::short_name_t<traits::integral_constant<'A'>>>>,
        "parameters_type does not match");
}

BOOST_AUTO_TEST_CASE(find_test)
{
    auto f = [](const auto& node,
                auto token,
                auto expected_index,
                auto expected_result) {
        using node_type = std::decay_t<decltype(node)>;

        auto visitor_hit = false;
        auto visitor_checker = [&](auto i, const auto& child) {
            using expected_child_type =
                std::tuple_element_t<i, typename node_type::children_type>;

            BOOST_CHECK_EQUAL(i, expected_index);
            BOOST_CHECK((std::is_same_v<expected_child_type,
                                        std::decay_t<decltype(child)>>));
            visitor_hit = true;
        };
        const auto result = node.find(token, visitor_checker);
        BOOST_CHECK_EQUAL(result, expected_result);
        BOOST_CHECK_EQUAL(result, visitor_hit);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{
                stub_node{stub_node{policy::long_name<S_("child1")>},
                          stub_node{policy::long_name<S_("child2")>}},
                parsing::token_type{parsing::prefix_type::long_, "child1"},
                0u,
                true},
            std::tuple{
                stub_node{stub_node{policy::long_name<S_("child1")>},
                          stub_node{policy::long_name<S_("child2")>}},
                parsing::token_type{parsing::prefix_type::long_, "child2"},
                1u,
                true},
            std::tuple{stub_node{stub_node{policy::short_name<'a'>},
                                 stub_node{policy::short_name<'b'>}},
                       parsing::token_type{parsing::prefix_type::short_, "a"},
                       0u,
                       true},
            std::tuple{stub_node{stub_node{policy::short_name<'a'>},
                                 stub_node{policy::short_name<'b'>}},
                       parsing::token_type{parsing::prefix_type::short_, "b"},
                       1u,
                       true},
            std::tuple{
                stub_node{stub_node{policy::long_name<S_("hello")>},
                          stub_node{policy::short_name<'b'>}},
                parsing::token_type{parsing::prefix_type::long_, "hello"},
                0u,
                true},
            std::tuple{stub_node{stub_node{policy::long_name<S_("hello")>},
                                 stub_node{policy::short_name<'b'>}},
                       parsing::token_type{parsing::prefix_type::short_, "b"},
                       1u,
                       true},
            std::tuple{stub_node{stub_node{policy::long_name<S_("hello")>},
                                 stub_node{policy::short_name<'b'>}},
                       parsing::token_type{parsing::prefix_type::short_, "g"},
                       0u,
                       false},
            std::tuple{stub_node{stub_node{policy::long_name<S_("hello")>},
                                 stub_node{policy::short_name<'b'>}},
                       parsing::token_type{parsing::prefix_type::long_, "foo"},
                       0u,
                       false},
            std::tuple{stub_node{},
                       parsing::token_type{parsing::prefix_type::long_, "foo"},
                       0u,
                       false},
        });
}

BOOST_AUTO_TEST_CASE(pre_parse_test)
{
    auto f = [](const auto& node,
                auto args,
                auto tokens,
                auto expected_args,
                auto expected_tokens,
                auto expected_result,
                std::string fail_message,
                const auto&... parents) {
        try {
            const auto result = node.pre_parse(args, tokens, parents...);

            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK_EQUAL(result, expected_result);
            BOOST_CHECK_EQUAL(args, expected_args);
            BOOST_CHECK_EQUAL(tokens, expected_tokens);
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
        }
    };

    test::data_set(
        f,
        std::tuple{
            // Basic name matching
            std::tuple{
                stub_node{policy::long_name<S_("hello")>},
                std::vector<parsing::token_type>{
                    {parsing::prefix_type::none, "--hello"}},
                parsing::token_list{},
                std::vector<parsing::token_type>{},
                parsing::token_list{{parsing::prefix_type::long_, "hello"}},
                true,
                ""},
            std::tuple{stub_node{policy::short_name<'h'>},
                       std::vector<parsing::token_type>{
                           {parsing::prefix_type::none, "-h"}},
                       parsing::token_list{},
                       std::vector<parsing::token_type>{},
                       parsing::token_list{{parsing::prefix_type::short_, "h"}},
                       true,
                       ""},
            std::tuple{
                stub_node{policy::none_name<S_("hello")>},
                std::vector<parsing::token_type>{
                    {parsing::prefix_type::none, "hello"}},
                parsing::token_list{},
                std::vector<parsing::token_type>{},
                parsing::token_list{{parsing::prefix_type::none, "hello"}},
                true,
                ""},
            std::tuple{stub_node{},
                       std::vector<parsing::token_type>{
                           {parsing::prefix_type::none, "hello"}},
                       parsing::token_list{},
                       std::vector<parsing::token_type>{
                           {parsing::prefix_type::none, "hello"}},
                       parsing::token_list{},
                       true,
                       ""},
            std::tuple{stub_node{policy::long_name<S_("hello")>},
                       std::vector<parsing::token_type>{
                           {parsing::prefix_type::none, "--foo"}},
                       parsing::token_list{},
                       std::vector<parsing::token_type>{
                           {parsing::prefix_type::none, "--foo"}},
                       parsing::token_list{},
                       false,
                       ""},

            // Policies
            std::tuple{
                stub_node{policy::long_name<S_("hello")>,
                          policy::value_separator<'='>,
                          policy::fixed_count<1>},
                std::vector<parsing::token_type>{
                    {parsing::prefix_type::none, "--hello=42"}},
                parsing::token_list{},
                std::vector<parsing::token_type>{},
                parsing::token_list{{parsing::prefix_type::long_, "hello"},
                                    {parsing::prefix_type::none, "42"}},
                true,
                ""},
            std::tuple{stub_node{policy::long_name<S_("hello")>,
                                 policy::value_separator<'='>,
                                 policy::fixed_count<1>},
                       std::vector<parsing::token_type>{
                           {parsing::prefix_type::none, "--hello"}},
                       parsing::token_list{},
                       std::vector<parsing::token_type>{
                           {parsing::prefix_type::none, "--hello"}},
                       parsing::token_list{},
                       false,
                       ""},
            std::tuple{stub_node{policy::long_name<S_("hello")>,
                                 policy::value_separator<'='>,
                                 policy::fixed_count<1>,
                                 policy::alias(policy::long_name<S_("foo")>)},
                       std::vector<parsing::token_type>{
                           {parsing::prefix_type::none, "--hello=42"}},
                       parsing::token_list{},
                       std::vector<parsing::token_type>{},
                       parsing::token_list{{parsing::prefix_type::long_, "foo"},
                                           {parsing::prefix_type::none, "42"}},
                       true,
                       "",
                       stub_node{policy::router{[](auto, auto) {}},
                                 stub_node{policy::long_name<S_("hello")>,
                                           policy::value_separator<'='>,
                                           policy::fixed_count<1>,
                                           policy::alias(
                                               policy::long_name<S_("foo")>)},
                                 stub_node{policy::long_name<S_("foo")>,
                                           policy::fixed_count<1>}}},
            std::tuple{stub_node{policy::long_name<S_("hello")>,
                                 policy::short_name<'h'>,
                                 policy::short_form_expander,
                                 policy::value_separator<'='>,
                                 policy::fixed_count<1>},
                       std::vector<parsing::token_type>{
                           {parsing::prefix_type::none, "-has=42"}},
                       parsing::token_list{},
                       std::vector<parsing::token_type>{
                           {parsing::prefix_type::short_, "a"},
                           {parsing::prefix_type::short_, "s"}},
                       parsing::token_list{{parsing::prefix_type::short_, "h"},
                                           {parsing::prefix_type::none, "42"}},
                       true,
                       ""},
            std::tuple{stub_node{policy::long_name<S_("hello")>,
                                 policy::short_name<'h'>,
                                 policy::short_form_expander,
                                 policy::value_separator<'='>,
                                 policy::fixed_count<1>},
                       std::vector<parsing::token_type>{
                           {parsing::prefix_type::none, "-ash=42"}},
                       parsing::token_list{},
                       std::vector<parsing::token_type>{
                           {parsing::prefix_type::none, "-ash=42"}},
                       parsing::token_list{},
                       false,
                       ""},
            std::tuple{
                stub_node{policy::long_name<S_("hello")>,
                          policy::fixed_count<1>},
                std::vector<parsing::token_type>{
                    {parsing::prefix_type::none, "--hello"},
                    {parsing::prefix_type::none, "42"}},
                parsing::token_list{},
                std::vector<parsing::token_type>{},
                parsing::token_list{{parsing::prefix_type::long_, "hello"},
                                    {parsing::prefix_type::none, "42"}},
                true,
                ""},

            // Overflow
            std::tuple{
                stub_node{policy::long_name<S_("hello")>,
                          policy::fixed_count<1>},
                std::vector<parsing::token_type>{
                    {parsing::prefix_type::none, "--hello"},
                    {parsing::prefix_type::none, "42"},
                    {parsing::prefix_type::none, "foo"}},
                parsing::token_list{},
                std::vector<parsing::token_type>{
                    {parsing::prefix_type::none, "foo"}},
                parsing::token_list{{parsing::prefix_type::long_, "hello"},
                                    {parsing::prefix_type::none, "42"}},
                true,
                ""},
            std::tuple{
                stub_node{policy::long_name<S_("hello")>,
                          policy::fixed_count<1>},
                std::vector<parsing::token_type>{
                    {parsing::prefix_type::none, "--hello"},
                    {parsing::prefix_type::none, "42"},
                    {parsing::prefix_type::none, "foo"}},
                parsing::token_list{{parsing::prefix_type::long_, "goodbye"}},
                std::vector<parsing::token_type>{
                    {parsing::prefix_type::none, "foo"}},
                parsing::token_list{{parsing::prefix_type::long_, "goodbye"},
                                    {parsing::prefix_type::long_, "hello"},
                                    {parsing::prefix_type::none, "42"}},
                true,
                ""},

            // Exception
            std::tuple{stub_node{policy::long_name<S_("hello")>,
                                 policy::fixed_count<1>},
                       std::vector<parsing::token_type>{
                           {parsing::prefix_type::none, "--hello"}},
                       parsing::token_list{},
                       std::vector<parsing::token_type>{
                           {parsing::prefix_type::none, "--hello"}},
                       parsing::token_list{},
                       false,
                       "Minimum count not reached: --hello"},
        });
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(only_policies_or_nodes_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/tree_node.hpp"

using namespace arg_router;

using tn = tree_node<policy::long_name_t<S_("hello")>,
                     tree_node<policy::long_name_t<S_("child")>>,
                     double,
                     policy::short_name_t<traits::integral_constant<'A'>>>;

int main() {
    static_assert(
        std::is_same_v<
            typename tn::parameters_type,
            std::tuple<policy::long_name_t<S_("hello")>,
                       tree_node<policy::long_name_t<S_("child")>>,
                       double,
                       policy::short_name_t<traits::integral_constant<'A'>>>>);
    return 0;
}
    )",
        "tree_node constructor can only accept other tree_nodes and policies");
}

BOOST_AUTO_TEST_CASE(parse_phase_policies_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/custom_parser.hpp"
#include "arg_router/tree_node.hpp"

using namespace arg_router;

template <typename... Params>
class stub_node : public tree_node<Params...>
{
public:
    using tree_node<Params...>::parse;

    constexpr explicit stub_node(Params... params) :
        tree_node<Params...>{std::move(params)...}
    {
    }
};

int main() {
    auto f = stub_node{
        policy::custom_parser<int>{[](auto) { return 42; }},
        policy::custom_parser<double>{[](auto) { return 3.14; }}
    };
    f.template parse<int>("hello");
    return 0;
}
    )",
        "Only zero or one policies supporting a parse phase is supported");
}

BOOST_AUTO_TEST_CASE(missing_phase_policies_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/default_value.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/tree_node.hpp"

using namespace arg_router;

template <typename... Params>
class stub_node : public tree_node<Params...>
{
public:
    using tree_node<Params...>::parse;

    constexpr explicit stub_node(Params... params) :
        tree_node<Params...>{std::move(params)...}
    {
    }
};

int main() {
    auto f = stub_node{
        policy::default_value{42},
        policy::required};
    f.template parse<int>("hello");
    return 0;
}
    )",
        "Only zero or one policies supporting a missing phase is supported");
}

BOOST_AUTO_TEST_CASE(routing_phase_policies_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/router.hpp"
#include "arg_router/tree_node.hpp"

using namespace arg_router;

template <typename... Params>
class stub_node : public tree_node<Params...>
{
public:
    using tree_node<Params...>::parse;

    constexpr explicit stub_node(Params... params) :
        tree_node<Params...>{std::move(params)...}
    {
    }

    using tree_node<Params...>::routing_phase;
};

int main() {
    auto f = stub_node{
        policy::router{[](auto...) {}},
        policy::router{[](auto...) {}}
    };
    f.routing_phase(parsing::token_list{});
    return 0;
}
    )",
        "Only zero or one policies supporting a routing phase is supported");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
