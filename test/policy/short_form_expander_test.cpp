/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/policy/short_form_expander.hpp"
#include "arg_router/policy/long_name.hpp"
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
        });
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(owner_must_have_short_name_test)
{
    test::death_test_compile(
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
        vector<parsing::token_type>& result,
        [[maybe_unused]] const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<0, typename stub_node::policies_type>;

        auto args = vector<parsing::token_type>{};
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
    auto tokens = vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "hello"}};
    node.pre_parse_phase(tokens);
    return 0;
}
    )",
        "Short-form expansion support requires a short name policy");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
