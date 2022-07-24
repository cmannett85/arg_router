/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/policy/value_separator.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_count.hpp"
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

BOOST_AUTO_TEST_SUITE(value_separator_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::value_separator_t<traits::integral_constant<'='>>>,
                  "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(constructor_and_get_test)
{
    const auto c_a = policy::value_separator<'='>;
    BOOST_CHECK_EQUAL(c_a.value_separator(), "=");

    const auto c_4 = policy::value_separator<'/'>;
    BOOST_CHECK_EQUAL(c_4.value_separator(), "/");
}

BOOST_AUTO_TEST_CASE(pre_parse_phase_test)
{
    auto f = [](auto result,
                auto args,
                auto expected_result,
                auto expected_match,
                auto expected_args,
                const auto&... parents) {
        const auto policy = policy::value_separator<'='>;
        auto node = stub_node{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        auto processed_target = utility::compile_time_optional{};
        auto target = parsing::parse_target{node};

        const auto match = policy.pre_parse_phase(adapter, processed_target, target, parents...);
        BOOST_CHECK_EQUAL(result, expected_result);
        BOOST_CHECK_EQUAL(match, expected_match);
        BOOST_CHECK_EQUAL(args, expected_args);

        BOOST_CHECK(target);
        BOOST_CHECK(target.tokens().empty());
        BOOST_CHECK(target.sub_targets().empty());
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello=42"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello"},
                                                        {parsing::prefix_type::none, "42"}},
                       parsing::pre_parse_action::valid_node,
                       std::vector<parsing::token_type>{},
                       stub_node{policy::long_name<S_("hello")>, policy::fixed_count<1>}},
            std::tuple{std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello=42"},
                                                        {parsing::prefix_type::none, "foo"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello"},
                                                        {parsing::prefix_type::none, "42"}},
                       parsing::pre_parse_action::valid_node,
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "foo"}},
                       stub_node{policy::long_name<S_("hello")>, policy::fixed_count<1>}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::long_, "hello=42"}},
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::long_, "hello"},
                                                        {parsing::prefix_type::none, "42"}},
                       parsing::pre_parse_action::valid_node,
                       std::vector<parsing::token_type>{},
                       stub_node{policy::long_name<S_("hello")>, policy::fixed_count<1>}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::long_, "hello=42"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "foo"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::long_, "hello"},
                                                        {parsing::prefix_type::none, "42"}},
                       parsing::pre_parse_action::valid_node,
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "foo"}},
                       stub_node{policy::long_name<S_("hello")>, policy::fixed_count<1>}},
            std::tuple{std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-d=42"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-d"},
                                                        {parsing::prefix_type::none, "42"}},
                       parsing::pre_parse_action::valid_node,
                       std::vector<parsing::token_type>{},
                       stub_node{policy::long_name<S_("hello")>, policy::fixed_count<1>}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::short_, "h"}},
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::short_, "h"}},
                       parsing::pre_parse_action::skip_node,
                       std::vector<parsing::token_type>{},
                       stub_node{policy::long_name<S_("hello")>, policy::fixed_count<1>}},
            std::tuple{std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello"}},
                       std::vector<parsing::token_type>{},
                       parsing::pre_parse_action::skip_node,
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello"}},
                       stub_node{policy::long_name<S_("hello")>, policy::fixed_count<1>}},
            std::tuple{std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello="}},
                       std::vector<parsing::token_type>{},
                       parsing::pre_parse_action::skip_node,
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello="}},
                       stub_node{policy::long_name<S_("hello")>, policy::fixed_count<1>}},
            std::tuple{
                std::vector<parsing::token_type>{},
                std::vector<parsing::token_type>{{parsing::prefix_type::none, "--こんにちは=42"}},
                std::vector<parsing::token_type>{{parsing::prefix_type::none, "--こんにちは"},
                                                 {parsing::prefix_type::none, "42"}},
                parsing::pre_parse_action::valid_node,
                std::vector<parsing::token_type>{},
                stub_node{policy::long_name<S_("こんにちは")>, policy::fixed_count<1>}},
            std::tuple{
                std::vector<parsing::token_type>{},
                std::vector<parsing::token_type>{
                    {parsing::prefix_type::none, "--hello=よんじゅうに"}},
                std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello"},
                                                 {parsing::prefix_type::none, "よんじゅうに"}},
                parsing::pre_parse_action::valid_node,
                std::vector<parsing::token_type>{},
                stub_node{policy::long_name<S_("hello")>, policy::fixed_count<1>}},
            std::tuple{
                std::vector<parsing::token_type>{},
                std::vector<parsing::token_type>{
                    {parsing::prefix_type::none, "--こんにちは=よんじゅうに"}},
                std::vector<parsing::token_type>{{parsing::prefix_type::none, "--こんにちは"},
                                                 {parsing::prefix_type::none, "よんじゅうに"}},
                parsing::pre_parse_action::valid_node,
                std::vector<parsing::token_type>{},
                stub_node{policy::long_name<S_("こんにちは")>, policy::fixed_count<1>}},
        });
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(must_be_one_character_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/value_separator.hpp"
int main() {
    const auto ln = arg_router::policy::value_separator_utf8<S_("")>;
    return 0;
}
    )",
        "Value separator must only be one character");
}

BOOST_AUTO_TEST_CASE(whitespace_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/value_separator.hpp"
int main() {
    const auto ln = arg_router::policy::value_separator<' '>;
    return 0;
}
    )",
        "Value separator character must not be whitespace");
}

BOOST_AUTO_TEST_CASE(at_least_one_parent_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/value_separator.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include <vector>

using namespace arg_router;
namespace
{
template <typename... Policies>
class stub_node : public tree_node<Policies...>
{
public:
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
            std::tuple_element_t<1, typename stub_node::policies_type>;

        auto args = vector<parsing::token_type>{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        auto processed_target = utility::compile_time_optional{};
        auto target = parsing::parse_target{*this, parents...};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 processed_target,
                                                 target);
    }
};
}  // namespace

int main() {
    const auto node = stub_node{policy::long_name<S_("test")>,
                                policy::value_separator<'='>};
    auto tokens = vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "hello"},
                    {parsing::prefix_type::none, "42"}};
    node.pre_parse_phase(tokens);
    return 0;
}
    )",
        "At least one parent needed for value_separator_t");
}

BOOST_AUTO_TEST_CASE(owner_must_have_count_policies_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/value_separator.hpp"
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
            std::tuple_element_t<1, typename stub_node::policies_type>;

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
    const auto node = stub_node{policy::long_name<S_("hello")>,
                                policy::value_separator<'='>};
    auto tokens = vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "hello"}};
    node.pre_parse_phase(tokens);
    return 0;
}
        )",
        "Value separator support requires an owning node to have minimum and "
        "maximum count policies");
}

BOOST_AUTO_TEST_CASE(owner_must_have_fixed_count_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/policy/value_separator.hpp"
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
            std::tuple_element_t<2, typename stub_node::policies_type>;

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
    const auto node = stub_node{policy::long_name<S_("hello")>,
                                policy::max_count<3>,
                                policy::value_separator<'='>};
    auto tokens = vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "hello"}};
    node.pre_parse_phase(tokens);
    return 0;
}
    )",
        "Value separator support requires an owning node to have a fixed count "
        "of 1");
}

BOOST_AUTO_TEST_CASE(owner_must_have_fixed_count_of_one_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/policy/value_separator.hpp"
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
            std::tuple_element_t<2, typename stub_node::policies_type>;

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
    const auto node = stub_node{policy::long_name<S_("hello")>,
                                policy::fixed_count<3>,
                                policy::value_separator<'='>};
    auto tokens = vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "hello"}};
    node.pre_parse_phase(tokens);
    return 0;
}
    )",
        "Value separator support requires an owning node to have a fixed count "
        "of 1");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
