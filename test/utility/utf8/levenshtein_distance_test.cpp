// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/utility/utf8/levenshtein_distance.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/policy/runtime_enable.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/tree_node.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace arg_router::utility;
using namespace std::string_view_literals;

namespace
{
template <typename... PoliciesAndChildren>
class stub_node : public tree_node<PoliciesAndChildren...>
{
    using parent_type = tree_node<PoliciesAndChildren...>;

public:
    using children_type = typename parent_type::children_type;
    using policies_type = typename parent_type::policies_type;

    using value_type = bool;

    constexpr explicit stub_node(PoliciesAndChildren... policies) :
        tree_node<PoliciesAndChildren...>{std::move(policies)...}
    {
    }
};
}  // namespace

BOOST_AUTO_TEST_SUITE(utility_suite)

BOOST_AUTO_TEST_SUITE(utf8_suite)

BOOST_AUTO_TEST_CASE(levenshtein_distance_test)
{
    auto f = [](auto a, auto b, auto expected) {
        const auto result = utf8::levenshtein_distance(a, b);
        BOOST_CHECK_EQUAL(result, expected);
    };

    test::data_set(f,
                   {
                       std::tuple{"cat", "dog", 3},
                       std::tuple{"hello", "world", 4},
                       std::tuple{"", "abc", 3},
                       std::tuple{"abcd", "", 4},
                       std::tuple{"こんにちは", "hello", 5},
                       std::tuple{"こんにちは", "こんにち", 1},
                       std::tuple{"こんにち", "こんにちは", 1},
                       std::tuple{"🇬🇧🇦🇹🇮🇪", "🇬🇧🇮🇹🇮🇪", 1},
                       std::tuple{"क़m̃🙂b🇦🇬Δ猫", "क़m̃🙂b🇦🇬ち", 2},
                       std::tuple{"क़m̃abcΔ猫", "क़m̃🙂b🇦🇬Δ猫", 2},
                   });
}

BOOST_AUTO_TEST_CASE(closest_matching_child_node_test)
{
    const auto root =
        stub_node{stub_node{policy::long_name<AR_STRING("cat")>,
                            policy::short_name<'c'>,
                            policy::runtime_enable{true}},
                  stub_node{policy::long_name<AR_STRING("dog")>},
                  stub_node{policy::none_name<AR_STRING("Cam")>},
                  stub_node{policy::none_name<AR_STRING("Ella")>, policy::runtime_enable{false}},
                  stub_node{policy::short_name<'a'>,
                            policy::runtime_enable{false},
                            stub_node{policy::short_name<'b'>}},
                  dependency::one_of(
                      policy::required,
                      stub_node{policy::long_name<AR_STRING("hello")>, policy::short_name<'h'>},
                      stub_node{policy::short_name<'w'>})};

    auto f = [&](auto token, auto expected) {
        const auto result = utf8::closest_matching_child_node(root, token);
        BOOST_CHECK_EQUAL(result, expected);
    };

    test::data_set(
        f,
        {
            std::tuple{parsing::token_type{parsing::prefix_type::none, "--cat"},
                       vector<parsing::token_type>{{parsing::prefix_type::long_, "cat"}}},
            std::tuple{parsing::token_type{parsing::prefix_type::none, "--bat"},
                       vector<parsing::token_type>{{parsing::prefix_type::long_, "cat"}}},
            std::tuple{parsing::token_type{parsing::prefix_type::none, "--blob"},
                       vector<parsing::token_type>{{parsing::prefix_type::long_, "dog"}}},
            std::tuple{
                parsing::token_type{parsing::prefix_type::none, "--Ella"},
                vector<parsing::token_type>{{parsing::prefix_type::long_, "hello"},
                                            {parsing::prefix_type::none, "One of: --hello,-w"}}},
            std::tuple{parsing::token_type{parsing::prefix_type::none, "Spam"},
                       vector<parsing::token_type>{{parsing::prefix_type::none, "Cam"}}},
            std::tuple{
                parsing::token_type{parsing::prefix_type::none, "Yellow"},
                vector<parsing::token_type>{{parsing::prefix_type::long_, "hello"},
                                            {parsing::prefix_type::none, "One of: --hello,-w"}}},
            std::tuple{parsing::token_type{parsing::prefix_type::none, "-f"},
                       vector<parsing::token_type>{{parsing::prefix_type::short_, "c"}}},
            std::tuple{parsing::token_type{parsing::prefix_type::none, "-b"},
                       vector<parsing::token_type>{{parsing::prefix_type::short_, "c"}}},
        });
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(at_least_one_child_test)
{
    test::death_test_compile(R"(
#include "arg_router/utility/utf8/levenshtein_distance.hpp"

using namespace arg_router;

template <typename... PoliciesAndChildren>
class stub_node : public tree_node<PoliciesAndChildren...>
{
    using parent_type = tree_node<PoliciesAndChildren...>;

public:
    using children_type = typename parent_type::children_type;
    using policies_type = typename parent_type::policies_type;

    using value_type = bool;

    constexpr explicit stub_node(PoliciesAndChildren... policies) :
        tree_node<PoliciesAndChildren...>{std::move(policies)...}
    {
    }
};

int main() {
    const auto token = parsing::token_type{parsing::prefix_type::none, "bat"};
    const auto result = utility::utf8::closest_matching_child_node(stub_node{}, token);
    return 0;
}
    )",
                             "Node must have at least one child");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
