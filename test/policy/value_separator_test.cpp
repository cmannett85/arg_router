// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/policy/value_separator.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/tree_node.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace arg_router::literals;

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
    constexpr auto c_a = policy::value_separator<'='>;
    static_assert(c_a.value_separator() == "=");

    constexpr auto c_4 = policy::value_separator<'/'>;
    static_assert(c_4.value_separator() == "/");

    constexpr auto s_a = policy::value_separator_t{AR_STRING("="){}};
    static_assert(s_a.value_separator() == "=");
}

#ifdef ENABLE_CPP20_STRINGS
BOOST_AUTO_TEST_CASE(string_literal_test)
{
    const auto s_a = policy::value_separator_t{"="_S};
    static_assert(s_a.value_separator() == "=");
}
#endif

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
                       stub_node{policy::long_name<AR_STRING("hello")>, policy::fixed_count<1>}},
            std::tuple{std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello=42"},
                                                        {parsing::prefix_type::none, "foo"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello"},
                                                        {parsing::prefix_type::none, "42"}},
                       parsing::pre_parse_action::valid_node,
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "foo"}},
                       stub_node{policy::long_name<AR_STRING("hello")>, policy::fixed_count<1>}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::long_, "hello=42"}},
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::long_, "hello"},
                                                        {parsing::prefix_type::none, "42"}},
                       parsing::pre_parse_action::valid_node,
                       std::vector<parsing::token_type>{},
                       stub_node{policy::long_name<AR_STRING("hello")>, policy::fixed_count<1>}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::long_, "hello=42"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "foo"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::long_, "hello"},
                                                        {parsing::prefix_type::none, "42"}},
                       parsing::pre_parse_action::valid_node,
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "foo"}},
                       stub_node{policy::long_name<AR_STRING("hello")>, policy::fixed_count<1>}},
            std::tuple{std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-d=42"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-d"},
                                                        {parsing::prefix_type::none, "42"}},
                       parsing::pre_parse_action::valid_node,
                       std::vector<parsing::token_type>{},
                       stub_node{policy::long_name<AR_STRING("hello")>, policy::fixed_count<1>}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::short_, "h"}},
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::short_, "h"}},
                       parsing::pre_parse_action::skip_node,
                       std::vector<parsing::token_type>{},
                       stub_node{policy::long_name<AR_STRING("hello")>, policy::fixed_count<1>}},
            std::tuple{std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello"}},
                       std::vector<parsing::token_type>{},
                       parsing::pre_parse_action::skip_node,
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello"}},
                       stub_node{policy::long_name<AR_STRING("hello")>, policy::fixed_count<1>}},
            std::tuple{std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello="}},
                       std::vector<parsing::token_type>{},
                       parsing::pre_parse_action::skip_node,
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello="}},
                       stub_node{policy::long_name<AR_STRING("hello")>, policy::fixed_count<1>}},
            std::tuple{
                std::vector<parsing::token_type>{},
                std::vector<parsing::token_type>{{parsing::prefix_type::none, "--こんにちは=42"}},
                std::vector<parsing::token_type>{{parsing::prefix_type::none, "--こんにちは"},
                                                 {parsing::prefix_type::none, "42"}},
                parsing::pre_parse_action::valid_node,
                std::vector<parsing::token_type>{},
                stub_node{policy::long_name<AR_STRING("こんにちは")>, policy::fixed_count<1>}},
            std::tuple{
                std::vector<parsing::token_type>{},
                std::vector<parsing::token_type>{
                    {parsing::prefix_type::none, "--hello=よんじゅうに"}},
                std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello"},
                                                 {parsing::prefix_type::none, "よんじゅうに"}},
                parsing::pre_parse_action::valid_node,
                std::vector<parsing::token_type>{},
                stub_node{policy::long_name<AR_STRING("hello")>, policy::fixed_count<1>}},
            std::tuple{
                std::vector<parsing::token_type>{},
                std::vector<parsing::token_type>{
                    {parsing::prefix_type::none, "--こんにちは=よんじゅうに"}},
                std::vector<parsing::token_type>{{parsing::prefix_type::none, "--こんにちは"},
                                                 {parsing::prefix_type::none, "よんじゅうに"}},
                parsing::pre_parse_action::valid_node,
                std::vector<parsing::token_type>{},
                stub_node{policy::long_name<AR_STRING("こんにちは")>, policy::fixed_count<1>}},
        });
}

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile(
        {{R"(
#include "arg_router/policy/value_separator.hpp"
int main() {
    const auto ln = arg_router::policy::value_separator_utf8<AR_STRING("")>;
    return 0;
}
    )",
          "Value separator must only be one character",
          "must_be_one_character_test"},
         {
             R"(
#include "arg_router/policy/value_separator.hpp"
int main() {
    const auto ln = arg_router::policy::value_separator<' '>;
    return 0;
}
    )",
             "Value separator character must not be whitespace",
             "whitespace_test"},
         {
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
    const auto node = stub_node{policy::long_name<AR_STRING("test")>,
                                policy::value_separator<'='>};
    auto tokens = vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "hello"},
                    {parsing::prefix_type::none, "42"}};
    node.pre_parse_phase(tokens);
    return 0;
}
    )",
             "At least one parent needed for value_separator_t",
             "at_least_one_parent_test"},
         {
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
    const auto node = stub_node{policy::long_name<AR_STRING("hello")>,
                                policy::value_separator<'='>};
    auto tokens = vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "hello"}};
    node.pre_parse_phase(tokens);
    return 0;
}
        )",
             "Value separator support requires an owning node to have minimum and "
             "maximum count policies",
             "owner_must_have_count_policies_test"},
         {
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
    const auto node = stub_node{policy::long_name<AR_STRING("hello")>,
                                policy::max_count<3>,
                                policy::value_separator<'='>};
    auto tokens = vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "hello"}};
    node.pre_parse_phase(tokens);
    return 0;
}
    )",
             "Value separator support requires an owning node to have a fixed count "
             "of 1",
             "owner_must_have_fixed_count_test"},
         {
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
    const auto node = stub_node{policy::long_name<AR_STRING("hello")>,
                                policy::fixed_count<3>,
                                policy::value_separator<'='>};
    auto tokens = vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "hello"}};
    node.pre_parse_phase(tokens);
    return 0;
}
    )",
             "Value separator support requires an owning node to have a fixed count "
             "of 1",
             "owner_must_have_fixed_count_of_one_test"}});
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
