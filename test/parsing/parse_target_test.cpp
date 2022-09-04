/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/parsing/parse_target.hpp"
#include "arg_router/tree_node.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;

namespace
{
std::vector<utility::unsafe_any> expected_target_and_parents;

template <typename Node, typename... Parents>
void parse_checker(parsing::parse_target target, const Node& node, const Parents&... parents)
{
    BOOST_CHECK_EQUAL(utility::type_hash<Node>(), target.node_type());

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

template <typename... Policies>
class stub_node : public tree_node<Policies...>
{
public:
    using value_type = bool;

    constexpr explicit stub_node(Policies... policies) :
        tree_node<Policies...>{std::move(policies)...}
    {
    }

    template <typename... Parents>
    [[nodiscard]] value_type parse(parsing::parse_target target, const Parents&... parents) const
    {
        parse_checker(target, *this, parents...);
        return true;
    }
};

template <std::size_t I, std::size_t... Is, typename Root>
std::vector<utility::unsafe_any> make_pre_parse_test_data(const Root& root)
{
    auto result = std::vector<utility::unsafe_any>{};

    const auto refs = test::get_parents<I, Is...>(root);
    result.reserve(std::tuple_size_v<std::decay_t<decltype(refs)>>);
    utility::tuple_iterator([&](auto /*i*/, auto ref) { result.push_back(ref); }, refs);

    return result;
}
}  // namespace

BOOST_AUTO_TEST_SUITE(parsing_suite)

BOOST_AUTO_TEST_SUITE(parse_target_suite)

BOOST_AUTO_TEST_CASE(accessors_test)
{
    auto f = [](auto expected_tokens) {
        const auto node = stub_node{};
        const auto expected_index = utility::type_hash<std::decay_t<decltype(node)>>();
        auto target = parsing::parse_target{expected_tokens, node};

        BOOST_CHECK(target);
        BOOST_CHECK_EQUAL(expected_tokens, target.tokens());
        BOOST_CHECK_EQUAL(expected_index, target.node_type());
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector<parsing::token_type>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "hello"}}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "hello"},
                                                        {parsing::prefix_type::none, "goodbye"}}},
        });
}

BOOST_AUTO_TEST_CASE(function_test)
{
    auto tokens = std::vector<parsing::token_type>{{parsing::prefix_type::none, "hello"}};
    auto root = stub_node{stub_node{},            //
                          stub_node{stub_node{},  //
                                    stub_node{}}};

    const auto& node = test::get_node<1, 1>(root);
    expected_target_and_parents = make_pre_parse_test_data<1, 1>(root);

    auto target = parsing::parse_target{tokens, node};
    BOOST_CHECK(target);
    const auto parse_result1 = target();
    BOOST_REQUIRE(parse_result1.has_value());
    BOOST_CHECK(parse_result1.get<bool>());

    BOOST_CHECK(!target);
    const auto parse_result2 = target();
    BOOST_CHECK(!parse_result2.has_value());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
