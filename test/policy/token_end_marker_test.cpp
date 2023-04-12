// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/policy/token_end_marker.hpp"
#include "arg_router/literals.hpp"
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
    constexpr explicit stub_node(Policies... policies) :
        tree_node<Policies...>{std::move(policies)...}
    {
    }

    template <typename... Parents>
    [[nodiscard]] bool parse(parsing::parse_target, const Parents&...) const
    {
        return true;
    }
};
}  // namespace

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(token_end_marker_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::token_end_marker_t<AR_STRING("--")>>,
                  "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(constructor_and_get_test)
{
    constexpr auto hello_str = policy::token_end_marker<AR_STRING("hello")>;
    static_assert(hello_str.token_end_marker() == "hello");

    constexpr auto three_char_str = policy::token_end_marker<AR_STRING("boo")>;
    static_assert(three_char_str.token_end_marker() == "boo");

    constexpr auto world_str = policy::token_end_marker_t{AR_STRING("world"){}};
    static_assert(world_str.token_end_marker() == "world");
}

#ifdef AR_ENABLE_CPP20_STRINGS
BOOST_AUTO_TEST_CASE(string_literal_test)
{
    const auto world_str = policy::token_end_marker_t{"--"_S};
    static_assert(world_str.token_end_marker() == "--");
}
#endif

BOOST_AUTO_TEST_CASE(pre_parse_phase_test)
{
    auto f = [&](auto args, auto expected_result, auto expected_args, const auto&... parents) {
        auto node = stub_node{};
        auto result = std::vector<parsing::token_type>{};

        auto adapter = parsing::dynamic_token_adapter{result, args};
        auto processed_target = utility::compile_time_optional{parsing::parse_target{parents...}};
        auto target = parsing::parse_target{node};

        const auto match =
            policy::token_end_marker<AR_STRING("--")>.pre_parse_phase(adapter,
                                                                      processed_target,
                                                                      target,
                                                                      parents...);
        BOOST_CHECK_EQUAL(match.get(), parsing::pre_parse_action::valid_node);
        BOOST_CHECK_EQUAL(result, expected_result);
        BOOST_CHECK_EQUAL(args, expected_args);

        BOOST_CHECK(target);
        BOOST_CHECK(target.tokens().empty());
        BOOST_CHECK(target.sub_targets().empty());
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "one"},
                                                        {parsing::prefix_type::none, "two"},
                                                        {parsing::prefix_type::none, "three"},
                                                        {parsing::prefix_type::none, "--"}},
                       std::vector<parsing::token_type>{
                           {parsing::prefix_type::none, "one"},
                           {parsing::prefix_type::none, "two"},
                           {parsing::prefix_type::none, "three"},
                       },
                       std::vector<parsing::token_type>{},
                       stub_node{policy::min_count<1>}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "one"},
                                                        {parsing::prefix_type::none, "two"},
                                                        {parsing::prefix_type::none, "three"},
                                                        {parsing::prefix_type::none, "--"}},
                       std::vector<parsing::token_type>{
                           {parsing::prefix_type::none, "one"},
                           {parsing::prefix_type::none, "two"},
                           {parsing::prefix_type::none, "three"},
                       },
                       std::vector<parsing::token_type>{},
                       stub_node{policy::max_count<1>}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "one"},
                                                        {parsing::prefix_type::none, "two"},
                                                        {parsing::prefix_type::none, "three"}},
                       std::vector<parsing::token_type>{
                           {parsing::prefix_type::none, "one"},
                           {parsing::prefix_type::none, "two"},
                           {parsing::prefix_type::none, "three"},
                       },
                       std::vector<parsing::token_type>{},
                       stub_node{policy::min_count<1>}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "one"},
                                                        {parsing::prefix_type::none, "two"},
                                                        {parsing::prefix_type::none, "--"},
                                                        {parsing::prefix_type::none, "three"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "one"},
                                                        {parsing::prefix_type::none, "two"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "three"}},
                       stub_node{policy::min_count<1>}},
        });
}

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile({{
                                  R"(
#include "arg_router/policy/token_end_marker.hpp"
#include "arg_router/utility/compile_time_string.hpp"
int main() {
    const auto ln = arg_router::policy::token_end_marker<AR_STRING("")>;
    return 0;
}
    )",
                                  "Token end markers must not be an empty string",
                                  "empty_test"},
                              {
                                  R"(
#include "arg_router/policy/token_end_marker.hpp"
#include "arg_router/utility/compile_time_string.hpp"
int main() {
    const auto ln = arg_router::policy::token_end_marker<AR_STRING("a b")>;
    return 0;
}
    )",
                                  "Token end markers cannot contain whitespace",
                                  "space_test"},
                              {
                                  R"(
#include "arg_router/policy/token_end_marker.hpp"
#include "arg_router/policy/min_max_count.hpp"
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
        const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<0, typename stub_node::policies_type>;

        auto args = vector<parsing::token_type>{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        auto processed_target =
            utility::compile_time_optional{parsing::parse_target{parents...}};
        auto target = parsing::parse_target{*this, parents...};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 processed_target,
                                                 target,
                                                 parents...);
    }

    template <typename... Parents>
    [[nodiscard]] bool parse(parsing::parse_target, const Parents&...) const
    {
        return true;
    }
};
}  // namespace

int main() {
    const auto parent = stub_node{policy::token_end_marker<AR_STRING("--")>};
    auto tokens = vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "hello"}};
    parent.pre_parse_phase(tokens, parent);
    return 0;
}
    )",
                                  "Token end marker can only be used in variable list length nodes",
                                  "pre_parse_phase_test_no_min_max"},
                              {
                                  R"(
#include "arg_router/policy/token_end_marker.hpp"
#include "arg_router/policy/min_max_count.hpp"
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
        const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<0, typename stub_node::policies_type>;

        auto args = vector<parsing::token_type>{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        auto processed_target =
            utility::compile_time_optional{parsing::parse_target{parents...}};
        auto target = parsing::parse_target{*this, parents...};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 processed_target,
                                                 target,
                                                 parents...);
    }

    template <typename... Parents>
    [[nodiscard]] bool parse(parsing::parse_target, const Parents&...) const
    {
        return true;
    }
};
}  // namespace

int main() {
    const auto parent = stub_node{policy::token_end_marker<AR_STRING("--")>,
                                  policy::fixed_count<1>};
    auto tokens = vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "hello"}};
    parent.pre_parse_phase(tokens, parent);
    return 0;
}
    )",
                                  "Token end marker can only be used in variable list length nodes",
                                  "pre_parse_phase_test_fixed_1"},
                              {
                                  R"(
#include "arg_router/policy/token_end_marker.hpp"
#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/policy/multi_stage_value.hpp"
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
        const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<0, typename stub_node::policies_type>;

        auto args = vector<parsing::token_type>{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        auto processed_target =
            utility::compile_time_optional{parsing::parse_target{parents...}};
        auto target = parsing::parse_target{*this, parents...};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 processed_target,
                                                 target,
                                                 parents...);
    }

    template <typename... Parents>
    [[nodiscard]] bool parse(parsing::parse_target, const Parents&...) const
    {
        return true;
    }
};
}  // namespace

int main() {
    const auto parent = stub_node{policy::token_end_marker<AR_STRING("--")>,
                                  policy::multi_stage_value<int, bool>{[](auto&, auto&&){}}};
    auto tokens = vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "hello"}};
    parent.pre_parse_phase(tokens, parent);
    return 0;
}
    )",
                                  "Token end marker can only be used in variable list length nodes",
                                  "pre_parse_phase_test_multi_stage_value"}});
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
