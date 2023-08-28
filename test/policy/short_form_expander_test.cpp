// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/policy/short_form_expander.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/tree_node.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;

namespace
{
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
    [[nodiscard]] value_type parse(parsing::parse_target, const Parents&...) const
    {
        return true;
    }
};
}  // namespace

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(short_form_expander_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::short_form_expander_t<>>, "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(pre_parse_phase_test)
{
    auto f = [](auto result,
                auto args,
                auto expected_result,
                auto expected_args,
                const auto&... parents) {
        const auto policy = policy::short_form_expander;
        auto node = stub_node{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        auto processed_target = utility::compile_time_optional{};
        auto target = parsing::parse_target{node};

        const auto match = policy.pre_parse_phase(adapter, processed_target, target, parents...);
        BOOST_CHECK_EQUAL(result, expected_result);
        BOOST_CHECK_EQUAL(match, parsing::pre_parse_action::valid_node);
        BOOST_CHECK_EQUAL(args, expected_args);

        BOOST_CHECK(target);
        BOOST_CHECK(target.tokens().empty());
        BOOST_CHECK(target.sub_targets().empty());
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{},
                       stub_node{policy::short_name<'h'>}},
            std::tuple{std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello"}},
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello"}},
                       stub_node{policy::short_name<'h'>}},
            std::tuple{std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-h"}},
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-h"}},
                       stub_node{policy::short_name<'h'>}},
            std::tuple{std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--h"}},
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--h"}},
                       stub_node{policy::short_name<'h'>}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::long_, "h"}},
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::long_, "h"}},
                       std::vector<parsing::token_type>{},
                       stub_node{policy::short_name<'h'>}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::short_, "h"}},
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::short_, "h"}},
                       std::vector<parsing::token_type>{},
                       stub_node{policy::short_name<'h'>}},
            std::tuple{std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-hello"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::short_, "h"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::short_, "e"},
                                                        {parsing::prefix_type::short_, "l"},
                                                        {parsing::prefix_type::short_, "l"},
                                                        {parsing::prefix_type::short_, "o"}},
                       stub_node{policy::short_name<'h'>}},
            std::tuple{std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-h"},
                                                        {parsing::prefix_type::none, "-f"}},
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-h"},
                                                        {parsing::prefix_type::none, "-f"}},
                       stub_node{policy::short_name<'h'>}},
            std::tuple{
                std::vector<parsing::token_type>{},
                std::vector<parsing::token_type>{{parsing::prefix_type::none, "-こんにちは"}},
                std::vector<parsing::token_type>{{parsing::prefix_type::short_, "こ"}},
                std::vector<parsing::token_type>{{parsing::prefix_type::short_, "ん"},
                                                 {parsing::prefix_type::short_, "に"},
                                                 {parsing::prefix_type::short_, "ち"},
                                                 {parsing::prefix_type::short_, "は"}},
                stub_node{policy::short_name_utf8<AR_STRING("こ")>}},
            std::tuple{std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-hello"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::short_, "h"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::short_, "e"},
                                                        {parsing::prefix_type::short_, "l"},
                                                        {parsing::prefix_type::short_, "l"},
                                                        {parsing::prefix_type::short_, "o"}},
                       stub_node{policy::short_name_utf8<AR_STRING("h")>}},
            std::tuple{std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{
                           {parsing::prefix_type::none, "-🙂b🇦🇬Δ猫"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::short_, "🙂"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::short_, "b"},
                                                        {parsing::prefix_type::short_, "🇦🇬"},
                                                        {parsing::prefix_type::short_, "Δ"},
                                                        {parsing::prefix_type::short_, "猫"}},
                       stub_node{policy::short_name_utf8<AR_STRING("🙂")>}},
        });
}

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile({{
                                  R"(
#include "arg_router/policy/short_form_expander.hpp"
#include "arg_router/tree_node.hpp"

using namespace arg_router;

namespace
{
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
    void pre_parse_phase(
        std::vector<parsing::token_type>& result,
        [[maybe_unused]] const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<0, typename stub_node::policies_type>;

        auto args = std::vector<parsing::token_type>{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        auto processed_target = utility::compile_time_optional{};
        auto target = parsing::parse_target{*this, parents...};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 processed_target,
                                                 target,
                                                 *this,
                                                 parents...);
    }
};
}  // namespace

int main() {
    const auto node = stub_node{policy::short_form_expander};
    auto tokens = std::vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "hello"}};
    node.pre_parse_phase(tokens);
    return 0;
}
    )",
                                  "Short-form expansion support requires a short name policy",
                                  "short_form_expansion_support_requires_short_name_policy"},
                              {
                                  R"(
#undef AR_LONG_PREFIX
#define AR_LONG_PREFIX "-"

#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_form_expander.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/tree_node.hpp"

using namespace arg_router;

namespace
{
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
    void pre_parse_phase(
        std::vector<parsing::token_type>& result,
        [[maybe_unused]] const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<0, typename stub_node::policies_type>;

        auto args = std::vector<parsing::token_type>{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        auto processed_target = utility::compile_time_optional{};
        auto target = parsing::parse_target{*this, parents...};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 processed_target,
                                                 target,
                                                 *this,
                                                 parents...);
    }
};
}  // namespace

int main() {
    const auto node = stub_node{policy::short_form_expander,
                                policy::long_name<AR_STRING("hello")>,
                                policy::short_name<'H'>};
    auto tokens = std::vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "hello"}};
    node.pre_parse_phase(tokens);
    return 0;
}
    )",
                                  "Short and long prefixes cannot be the same",
                                  "short_and_long_prefixes_not_equal"}});
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
