/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/tree_node.hpp"
#include "arg_router/flag.hpp"
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
std::vector<utility::unsafe_any> expected_target_and_parents;

template <typename Node, typename... Parents>
void parse_checker(parsing::parse_target target, const Node& node, const Parents&... parents)
{
    BOOST_CHECK_EQUAL(std::type_index{typeid(Node)}.hash_code(), target.node_type().hash_code());

    const auto target_and_parents_tuple = std::tuple{std::cref(node), std::cref(parents)...};
    utility::tuple_iterator(
        [&](auto i, auto parent) {
            const auto& expected_parent_cast =
                expected_target_and_parents[i].template get<std::decay_t<decltype(parent)>>().get();
            BOOST_CHECK_EQUAL(
                reinterpret_cast<std::ptrdiff_t>(std::addressof(expected_parent_cast)),
                reinterpret_cast<std::ptrdiff_t>(std::addressof(parent.get())));
        },
        target_and_parents_tuple);

    expected_target_and_parents.clear();
}

template <typename... Params>
class stub_node : public tree_node<Params...>
{
public:
    using value_type = bool;

    constexpr explicit stub_node(Params... params) : tree_node<Params...>{std::move(params)...} {}

    template <typename Validator, bool HasTarget, typename... Parents>
    std::optional<parsing::parse_target> pre_parse(
        parsing::pre_parse_data<Validator, HasTarget> pre_parse_data,
        const Parents&... parents) const
    {
        return tree_node<Params...>::pre_parse(pre_parse_data, *this, parents...);
    }

    template <typename... Parents>
    [[nodiscard]] value_type parse(parsing::parse_target target, const Parents&... parents) const
    {
        parse_checker(target, *this, parents...);
        return true;
    }
};

struct pre_parse_test_data {
    std::vector<parsing::token_type> tokens;
    std::vector<utility::unsafe_any> target_and_parents;
};

template <std::size_t I, std::size_t... Is, typename Root>
pre_parse_test_data make_pre_parse_test_data(const Root& root,
                                             std::vector<parsing::token_type> tokens)
{
    auto result = pre_parse_test_data{};
    result.tokens = std::move(tokens);

    const auto refs = test::get_parents<I, Is...>(root);
    result.target_and_parents.reserve(std::tuple_size_v<std::decay_t<decltype(refs)>>);
    utility::tuple_iterator([&](auto /*i*/, auto ref) { result.target_and_parents.push_back(ref); },
                            refs);

    return result;
}
}  // namespace

BOOST_AUTO_TEST_SUITE(tree_node_suite)

BOOST_AUTO_TEST_CASE(mixed_tree_node_types_test)
{
    using tn = tree_node<policy::long_name_t<S_("hello")>,
                         tree_node<policy::long_name_t<S_("child")>>,
                         policy::short_name_t<traits::integral_constant<'A'>>>;
    static_assert(std::is_same_v<typename tn::parameters_type,
                                 std::tuple<policy::long_name_t<S_("hello")>,
                                            tree_node<policy::long_name_t<S_("child")>>,
                                            policy::short_name_t<traits::integral_constant<'A'>>>>,
                  "parameters_type does not match");

    static_assert(std::is_same_v<typename tn::policies_type,
                                 std::tuple<policy::long_name_t<S_("hello")>,
                                            policy::short_name_t<traits::integral_constant<'A'>>>>,
                  "policies_type does not match");

    static_assert(std::is_same_v<typename tn::children_type,
                                 std::tuple<tree_node<policy::long_name_t<S_("child")>>>>,
                  "children_type does not match");
}

BOOST_AUTO_TEST_CASE(only_policies_tree_node_types_test)
{
    using tn = tree_node<policy::long_name_t<S_("hello")>,
                         policy::short_name_t<traits::integral_constant<'A'>>>;
    static_assert(std::is_same_v<typename tn::parameters_type,
                                 std::tuple<policy::long_name_t<S_("hello")>,
                                            policy::short_name_t<traits::integral_constant<'A'>>>>,
                  "parameters_type does not match");

    static_assert(std::is_same_v<typename tn::policies_type,
                                 std::tuple<policy::long_name_t<S_("hello")>,
                                            policy::short_name_t<traits::integral_constant<'A'>>>>,
                  "policies_type does not match");

    static_assert(std::is_same_v<typename tn::children_type, std::tuple<>>,
                  "children_type does not match");
}

BOOST_AUTO_TEST_CASE(one_child_tree_node_types_test)
{
    using tn = tree_node<flag_t<policy::long_name_t<S_("hello")>,
                                policy::short_name_t<traits::integral_constant<'A'>>>>;
    static_assert(
        std::is_same_v<typename tn::parameters_type,
                       std::tuple<flag_t<policy::long_name_t<S_("hello")>,
                                         policy::short_name_t<traits::integral_constant<'A'>>>>>,
        "parameters_type does not match");

    static_assert(std::is_same_v<typename tn::policies_type, std::tuple<>>,
                  "policies_type does not match");

    static_assert(
        std::is_same_v<typename tn::children_type,
                       std::tuple<flag_t<policy::long_name_t<S_("hello")>,
                                         policy::short_name_t<traits::integral_constant<'A'>>>>>,
        "children_type does not match");
}

BOOST_AUTO_TEST_CASE(two_children_tree_node_types_test)
{
    using tn = tree_node<flag_t<policy::long_name_t<S_("hello")>,
                                policy::short_name_t<traits::integral_constant<'A'>>>,
                         flag_t<policy::short_name_t<traits::integral_constant<'B'>>>>;
    static_assert(
        std::is_same_v<typename tn::parameters_type,
                       std::tuple<flag_t<policy::long_name_t<S_("hello")>,
                                         policy::short_name_t<traits::integral_constant<'A'>>>,
                                  flag_t<policy::short_name_t<traits::integral_constant<'B'>>>>>,
        "parameters_type does not match");

    static_assert(std::is_same_v<typename tn::policies_type, std::tuple<>>,
                  "policies_type does not match");

    static_assert(
        std::is_same_v<typename tn::children_type,
                       std::tuple<flag_t<policy::long_name_t<S_("hello")>,
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
    static_assert(!is_tree_node_v<std::vector<float>>, "Should not be a tree_node");

    static_assert(is_tree_node_v<tree_node<float>>, "Should be a tree_node");
    static_assert(is_tree_node_v<tree_node<float, int, double>>, "Should be a tree_node");
    static_assert(is_tree_node_v<tree_node<float, int, double>>, "Should be a tree_node");
    static_assert(is_tree_node_v<tree_node<policy::long_name_t<S_("hello")>>>,
                  "Should be a tree_node");
    static_assert(
        is_tree_node_v<
            tree_node<policy::long_name_t<S_("hello")>, policy::long_name_t<S_("goodbye")>>>,
        "Should be a tree_node");
}

BOOST_AUTO_TEST_CASE(priority_ordered_policies_type_test)
{
    using tn = tree_node<policy::long_name_t<S_("hello")>,
                         policy::alias_t<policy::short_name_t<traits::integral_constant<'A'>>>,
                         policy::short_name_t<traits::integral_constant<'A'>>,
                         policy::default_value<int>>;

    static_assert(
        std::is_same_v<
            typename tn::priority_ordered_policies_type,
            std::tuple<policy::default_value<int>,
                       policy::alias_t<policy::short_name_t<traits::integral_constant<'A'>>>,
                       policy::long_name_t<S_("hello")>,
                       policy::short_name_t<traits::integral_constant<'A'>>>>,
        "parameters_type does not match");
}

BOOST_AUTO_TEST_CASE(pre_parse_test)
{
    auto f = [](const auto& node,
                auto args,
                auto expected_tokens,
                auto expected_sub_target_data,
                auto expected_args,
                auto match,
                std::string fail_message,
                const auto&... parents) {
        try {
            auto result = node.pre_parse(parsing::pre_parse_data{args}, parents.get()...);
            BOOST_CHECK(fail_message.empty());

            BOOST_CHECK_EQUAL(!match, !result);
            BOOST_CHECK_EQUAL(args, expected_args);

            if (match) {
                // Check the sub-targets first
                BOOST_REQUIRE_EQUAL(expected_sub_target_data.size(), result->sub_targets().size());
                for (auto i = 0u; i < expected_sub_target_data.size(); ++i) {
                    auto& sub_target = result->sub_targets()[i];
                    const auto& expected_sub_target = expected_sub_target_data[i];

                    BOOST_CHECK_EQUAL(expected_sub_target.tokens, sub_target.tokens());

                    expected_target_and_parents = std::move(expected_sub_target.target_and_parents);
                    BOOST_CHECK(sub_target);
                    sub_target();
                    BOOST_CHECK(!sub_target);
                }

                // Now the main target
                BOOST_CHECK_EQUAL(expected_tokens, result->tokens());
                expected_target_and_parents = {std::cref(node), std::cref(parents)...};
                BOOST_CHECK(*result);
                (*result)();
                BOOST_CHECK(!*result);
            }
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
        }
    };

    const auto alias_parent =
        stub_node{policy::router{[](auto, auto) {}},
                  stub_node{policy::long_name<S_("hello")>,
                            policy::value_separator<'='>,
                            policy::fixed_count<1>,
                            policy::alias(policy::long_name<S_("foo")>)},
                  stub_node{policy::long_name<S_("foo")>, policy::fixed_count<1>}};

    test::data_set(
        f,
        std::tuple{
            // Basic name matching
            std::tuple{stub_node{policy::long_name<S_("hello")>},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello"}},
                       std::vector<parsing::token_type>{},
                       std::vector<pre_parse_test_data>{},
                       std::vector<parsing::token_type>{},
                       true,
                       ""},
            std::tuple{stub_node{policy::short_name<'h'>},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-h"}},
                       std::vector<parsing::token_type>{},
                       std::vector<pre_parse_test_data>{},
                       std::vector<parsing::token_type>{},
                       true,
                       ""},
            std::tuple{stub_node{policy::none_name<S_("hello")>},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "hello"}},
                       std::vector<parsing::token_type>{},
                       std::vector<pre_parse_test_data>{},
                       std::vector<parsing::token_type>{},
                       true,
                       ""},
            std::tuple{stub_node{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "hello"}},
                       std::vector<parsing::token_type>{},
                       std::vector<pre_parse_test_data>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "hello"}},
                       false,
                       ""},
            std::tuple{stub_node{policy::long_name<S_("hello")>},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--foo"}},
                       std::vector<parsing::token_type>{},
                       std::vector<pre_parse_test_data>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--foo"}},
                       false,
                       ""},

            // Policies
            std::tuple{stub_node{policy::long_name<S_("hello")>,
                                 policy::value_separator<'='>,
                                 policy::fixed_count<1>},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello=42"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"}},
                       std::vector<pre_parse_test_data>{},
                       std::vector<parsing::token_type>{},
                       true,
                       ""},
            std::tuple{stub_node{policy::long_name<S_("hello")>,
                                 policy::value_separator<'='>,
                                 policy::fixed_count<1>},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello"}},
                       std::vector<parsing::token_type>{},
                       std::vector<pre_parse_test_data>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello"}},
                       false,
                       ""},
            std::tuple{
                test::get_node<0>(alias_parent),
                std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello=42"}},
                std::vector<parsing::token_type>{},
                std::vector<pre_parse_test_data>{
                    make_pre_parse_test_data<1>(alias_parent, {{parsing::prefix_type::none, "42"}}),
                },
                std::vector<parsing::token_type>{},
                true,
                "",
                std::cref(alias_parent)},
            std::tuple{stub_node{policy::long_name<S_("hello")>,
                                 policy::short_name<'h'>,
                                 policy::short_form_expander,
                                 policy::value_separator<'='>,
                                 policy::fixed_count<1>},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-has=42"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"}},
                       std::vector<pre_parse_test_data>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::short_, "a"},
                                                        {parsing::prefix_type::short_, "s"}},
                       true,
                       ""},
            std::tuple{stub_node{policy::long_name<S_("hello")>,
                                 policy::short_name<'h'>,
                                 policy::short_form_expander,
                                 policy::value_separator<'='>,
                                 policy::fixed_count<1>},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-ash=42"}},
                       std::vector<parsing::token_type>{},
                       std::vector<pre_parse_test_data>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-ash=42"}},
                       false,
                       ""},
            std::tuple{stub_node{policy::long_name<S_("hello")>, policy::fixed_count<1>},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello"},
                                                        {parsing::prefix_type::none, "42"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"}},
                       std::vector<pre_parse_test_data>{},
                       std::vector<parsing::token_type>{},
                       true,
                       ""},

            // Overflow
            std::tuple{stub_node{policy::long_name<S_("hello")>, policy::fixed_count<1>},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello"},
                                                        {parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "foo"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"}},
                       std::vector<pre_parse_test_data>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "foo"}},
                       true,
                       ""},

            // Exception
            std::tuple{stub_node{policy::long_name<S_("hello")>, policy::fixed_count<1>},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello"}},
                       std::vector<parsing::token_type>{},
                       std::vector<pre_parse_test_data>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello"}},
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
    f.routing_phase();
    return 0;
}
    )",
        "Only zero or one policies supporting a routing phase is supported");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
