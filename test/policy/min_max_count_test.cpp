// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/policy/display_name.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

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
    [[nodiscard]] bool parse(parsing::parse_target, const Parents&...) const
    {
        return true;
    }
};
}  // namespace

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(min_max_count_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(
        policy::is_policy_v<
            policy::min_max_count_t<traits::integral_constant<0>, traits::integral_constant<0>>>,
        "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(count_test)
{
    static_assert(policy::min_count<2>.minimum_count() == 2, "Fail");
    static_assert(policy::min_count<2>.maximum_count() == (std::numeric_limits<std::size_t>::max()),
                  "Fail");

    static_assert(policy::fixed_count<42u>.minimum_count() == 42, "Fail");
    static_assert(policy::fixed_count<42u>.maximum_count() == 42, "Fail");

    static_assert(policy::fixed_count<5>.minimum_count() == 5, "Fail");
    static_assert(policy::fixed_count<5>.maximum_count() == 5, "Fail");

    static_assert(policy::max_count<5>.minimum_count() == 0, "Fail");
    static_assert(policy::max_count<5>.maximum_count() == 5, "Fail");
}

BOOST_AUTO_TEST_CASE(pre_parse_phase_test)
{
    auto f = [&](const auto& policy,
                 auto result,
                 auto args,
                 auto expected_result,
                 auto expected_args,
                 auto ec,
                 const auto&... parents) {
        auto node = stub_node{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        auto processed_target = utility::compile_time_optional{parsing::parse_target{parents...}};
        auto target = parsing::parse_target{node};
        try {
            const auto match =
                policy.pre_parse_phase(adapter, processed_target, target, parents...);
            BOOST_CHECK_EQUAL(match.get(), parsing::pre_parse_action::valid_node);
            BOOST_CHECK(!ec);
            BOOST_CHECK_EQUAL(result, expected_result);
            BOOST_CHECK_EQUAL(args, expected_args);

            BOOST_CHECK(target);
            BOOST_CHECK(target.tokens().empty());
            BOOST_CHECK(target.sub_targets().empty());
        } catch (multi_lang_exception& e) {
            BOOST_REQUIRE(ec);
            BOOST_CHECK_EQUAL(e.ec(), ec->ec());
            BOOST_CHECK_EQUAL(e.tokens(), ec->tokens());
        }
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{policy::min_count<1>,
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--node1"},
                                                        {parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "foo"},
                                                        {parsing::prefix_type::none, "hello"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--node1"},
                                                        {parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "foo"},
                                                        {parsing::prefix_type::none, "hello"}},
                       std::vector<parsing::token_type>{},
                       std::optional<multi_lang_exception>{},
                       stub_node{policy::long_name<AR_STRING("node1")>, policy::min_count<1>}},
            std::tuple{policy::fixed_count<1>,
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--node1"},
                                                        {parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "foo"},
                                                        {parsing::prefix_type::none, "hello"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--node1"},
                                                        {parsing::prefix_type::none, "42"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "foo"},
                                                        {parsing::prefix_type::none, "hello"}},
                       std::optional<multi_lang_exception>{},
                       stub_node{policy::long_name<AR_STRING("node1")>, policy::fixed_count<1>}},
            std::tuple{policy::min_count<1>,
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--node1"},
                                                        {parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "foo"},
                                                        {parsing::prefix_type::none, "hello"}},
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--node1"},
                                                        {parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "foo"},
                                                        {parsing::prefix_type::none, "hello"}},
                       std::vector<parsing::token_type>{},
                       std::optional<multi_lang_exception>{},
                       stub_node{policy::long_name<AR_STRING("node1")>, policy::min_count<1>}},
            std::tuple{policy::min_count<1>,
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--node1"},
                                                        {parsing::prefix_type::none, "42"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "foo"},
                                                        {parsing::prefix_type::none, "hello"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--node1"},
                                                        {parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "foo"},
                                                        {parsing::prefix_type::none, "hello"}},
                       std::vector<parsing::token_type>{},
                       std::optional<multi_lang_exception>{},
                       stub_node{policy::long_name<AR_STRING("node1")>, policy::min_count<1>}},
            std::tuple{
                policy::min_count<2>,
                std::vector<parsing::token_type>{},
                std::vector<parsing::token_type>{{parsing::prefix_type::none, "--node2"},
                                                 {parsing::prefix_type::none, "42"}},
                std::vector<parsing::token_type>{{parsing::prefix_type::none, "--node1"},
                                                 {parsing::prefix_type::none, "42"}},
                std::vector<parsing::token_type>{},
                std::optional<multi_lang_exception>{
                    test::create_exception(error_code::minimum_count_not_reached, {"--node2"})},
                stub_node{policy::long_name<AR_STRING("node2")>, policy::min_count<2>}},
            std::tuple{policy::max_count<2>,
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--node1"},
                                                        {parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "foo"},
                                                        {parsing::prefix_type::none, "hello"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--node1"},
                                                        {parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "foo"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "hello"}},
                       std::optional<multi_lang_exception>{},
                       stub_node{policy::long_name<AR_STRING("node1")>, policy::max_count<2>}},
            std::tuple{policy::max_count<2>,
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "foo"},
                                                        {parsing::prefix_type::none, "hello"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "foo"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "hello"}},
                       std::optional<multi_lang_exception>{},
                       stub_node{policy::display_name<AR_STRING("node1")>, policy::max_count<2>}},
            std::tuple{policy::fixed_count<1>,
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--node1"},
                                                        {parsing::prefix_type::none, "霊"},
                                                        {parsing::prefix_type::none, "foo"},
                                                        {parsing::prefix_type::none, "hello"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--node1"},
                                                        {parsing::prefix_type::none, "霊"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "foo"},
                                                        {parsing::prefix_type::none, "hello"}},
                       std::optional<multi_lang_exception>{},
                       stub_node{policy::long_name<AR_STRING("node1")>, policy::fixed_count<1>}},
        });
}

BOOST_AUTO_TEST_CASE(pre_parse_phase_filled_test)
{
    auto root =
        stub_node{policy::long_name<AR_STRING("root")>,
                  stub_node{policy::display_name<AR_STRING("node0")>, policy::fixed_count<2>},
                  stub_node{policy::display_name<AR_STRING("node1")>, policy::fixed_count<1>},
                  stub_node{policy::display_name<AR_STRING("node2")>, policy::min_count<0>}};

    auto f = [&](const auto& node,
                 auto expected_match,
                 auto result,
                 auto args,
                 auto expected_result,
                 auto expected_args,
                 auto processed_target_tokens,
                 const auto& processed_target_node) {
        auto adapter = parsing::dynamic_token_adapter{result, args};
        auto processed_target = utility::compile_time_optional{
            parsing::parse_target{std::move(processed_target_tokens), processed_target_node, root}};
        auto target = parsing::parse_target{node};
        const auto match = node.pre_parse_phase(adapter, processed_target, target, node, root);

        BOOST_CHECK_EQUAL(match.get(), expected_match);
        BOOST_CHECK_EQUAL(result, expected_result);
        BOOST_CHECK_EQUAL(args, expected_args);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{test::get_node<0>(root),
                       parsing::pre_parse_action::valid_node,
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "foo"},
                                                        {parsing::prefix_type::none, "hello"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "foo"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "hello"}},
                       std::vector<parsing::token_type>{},
                       test::get_node<0>(root)},
            std::tuple{test::get_node<0>(root),
                       parsing::pre_parse_action::skip_node,
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "foo"},
                                                        {parsing::prefix_type::none, "hello"}},
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "foo"},
                                                        {parsing::prefix_type::none, "hello"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "foo"}},
                       test::get_node<0>(root)},
            std::tuple{test::get_node<0>(root),
                       parsing::pre_parse_action::valid_node,
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "foo"},
                                                        {parsing::prefix_type::none, "hello"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "foo"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "hello"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "foo"}},
                       test::get_node<1>(root)},
            std::tuple{test::get_node<2>(root),
                       parsing::pre_parse_action::valid_node,
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "foo"},
                                                        {parsing::prefix_type::none, "hello"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "foo"},
                                                        {parsing::prefix_type::none, "hello"}},
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{},
                       test::get_node<2>(root)},
            std::tuple{test::get_node<2>(root),
                       parsing::pre_parse_action::valid_node,
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "foo"},
                                                        {parsing::prefix_type::none, "hello"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "foo"},
                                                        {parsing::prefix_type::none, "hello"}},
                       std::vector<parsing::token_type>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "foo"}},
                       test::get_node<2>(root)},
        });
}

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile({{R"(
#include "arg_router/policy/min_max_count.hpp"

struct my_type {};

int main() {
    const auto tmp = arg_router::policy::min_max_count_t<my_type, my_type>{};
    return 0;
}
    )",
                               "MinType and MaxType must have a value_type",
                               "value_type_test"},
                              {
                                  R"(
#include "arg_router/policy/min_max_count.hpp"

struct my_type {
    using value_type = std::string;
};

int main() {
    const auto tmp = arg_router::policy::min_max_count_t<my_type, my_type>{};
    return 0;
}
    )",
                                  "MinType and MaxType must have a value_type that is implicitly "
                                  "convertible to std::size_t",
                                  "integral_test"},
                              {
                                  R"(
#include <string>
#include "arg_router/policy/min_max_count.hpp"

struct my_type {
    using value_type = double;
};

int main() {
    const auto tmp = arg_router::policy::min_max_count_t<my_type, my_type>{};
    return 0;
}
    )",
                                  "MinType and MaxType value_types must be integrals",
                                  "conversion_test"},
                              {
                                  R"(
#include "arg_router/policy/min_max_count.hpp"

using namespace arg_router;
int main() {
    const auto tmp = policy::min_max_count_t<traits::integral_constant<-5>,
                                             traits::integral_constant<5>>{};
    return 0;
}
    )",
                                  "MinType and MaxType must have a value that is a positive number",
                                  "min_count_positive_value_test"},
                              {
                                  R"(
#include "arg_router/policy/min_max_count.hpp"

using namespace arg_router;
int main() {
    const auto tmp = policy::min_max_count_t<traits::integral_constant<0>,
                                             traits::integral_constant<-5>>{};
    return 0;
}
    )",
                                  "MinType and MaxType must have a value that is a positive number",
                                  "max_count_positive_value_test"},
                              {
                                  R"(
#include "arg_router/policy/min_max_count.hpp"

using namespace arg_router;
int main() {
    const auto tmp = policy::min_max_count<5, 3>;
    return 0;
}
    )",
                                  "MinType must be less than or equal to MaxType",
                                  "valid_values_test"},
                              {
                                  R"(
#include "arg_router/policy/long_name.hpp"
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
            std::tuple_element_t<1, typename stub_node::policies_type>;

        auto args = vector<parsing::token_type>{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        auto processed_target =
            utility::compile_time_optional{parsing::parse_target{parents...}};
        auto target = parsing::parse_target{*this, parents...};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 processed_target,
                                                 target);
    }
};
}  // namespace

int main() {
    const auto parent = stub_node{policy::long_name<AR_STRING("parent")>,
                                  stub_node{policy::long_name<AR_STRING("test")>,
                                            policy::fixed_count<1>}};
    auto tokens = vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "hello"}};
    std::get<0>(parent.children()).pre_parse_phase(tokens, parent);
    return 0;
}
    )",
                                  "At least one parent needed for min_max_count_t",
                                  "pre_parse_phase_test"},
                              {
                                  R"(
#include "arg_router/policy/display_name.hpp"
#include "arg_router/policy/long_name.hpp"
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

    template <typename... Parents>
    [[nodiscard]] bool parse(parsing::parse_target, const Parents&...) const
    {
        return true;
    }
};
}  // namespace

int main() {
    const auto parent = stub_node{policy::long_name<AR_STRING("parent")>,
                                  stub_node{policy::display_name<AR_STRING("test")>,
                                            policy::fixed_count<1>}};
    auto tokens = vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "hello"}};
    std::get<0>(parent.children()).pre_parse_phase(tokens, parent);
    return 0;
}
    )",
                                  "processed_target cannot be empty",
                                  "processed_target_cannot_be_empty_test"}});
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
