// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/policy/runtime_enable.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/tree_node.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace std::string_literals;

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

BOOST_AUTO_TEST_SUITE(runtime_enable_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::runtime_enable<>>, "Policy test has failed");
    static_assert(policy::is_policy_v<policy::runtime_enable_required<bool>>,
                  "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(type_test)
{
    static_assert(std::is_same_v<typename policy::runtime_enable_required<bool>::value_type, bool>);
}

BOOST_AUTO_TEST_CASE(pre_parse_phase_test)
{
    auto f = [&](auto is_required, auto enabled) {
        auto unprocessed = std::vector<parsing::token_type>{};
        auto processed = std::vector<parsing::token_type>{};
        auto tokens = parsing::dynamic_token_adapter{unprocessed, unprocessed};

        auto node = stub_node{};
        auto target = parsing::parse_target{node};

        auto result = parsing::pre_parse_result{parsing::pre_parse_action::valid_node};
        if (is_required) {
            const auto r = policy::runtime_enable_required{enabled, 42};
            result = r.pre_parse_phase(tokens, utility::compile_time_optional{}, target, node);
        } else {
            const auto r = policy::runtime_enable{enabled};
            result = r.pre_parse_phase(tokens, utility::compile_time_optional{}, target, node);
        }
        BOOST_CHECK_EQUAL(
            result.get(),
            enabled ? parsing::pre_parse_action::valid_node : parsing::pre_parse_action::skip_node);
    };

    test::data_set(f,
                   {
                       std::tuple{false, true},
                       std::tuple{false, false},

                       std::tuple{true, true},
                       std::tuple{true, false},
                   });
}

BOOST_AUTO_TEST_CASE(missing_phase_test)
{
    auto f = [&](auto enabled) {
        auto node = stub_node{policy::long_name<AR_STRING("hello")>};

        try {
            const auto r = policy::runtime_enable_required{enabled, 42};
            const auto result = r.template missing_phase<int>(node);
            BOOST_CHECK(!enabled);
            BOOST_CHECK_EQUAL(result, 42);
        } catch (multi_lang_exception& e) {
            BOOST_CHECK(enabled);
            BOOST_CHECK_EQUAL(e.ec(), error_code::missing_required_argument);
        }
    };

    test::data_set(f,
                   {
                       std::tuple{true},
                       std::tuple{false},
                   });
}

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile({{R"(
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/runtime_enable.hpp"
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

    parsing::pre_parse_result pre_parse() const
    {
        using this_policy =
            std::tuple_element_t<1, typename stub_node::policies_type>;

        auto unprocessed = std::vector<parsing::token_type>{};
        auto processed = std::vector<parsing::token_type>{};
        auto tokens = parsing::dynamic_token_adapter{unprocessed, unprocessed};

        auto target = parsing::parse_target{*this};

        return this->this_policy::template pre_parse_phase(
            tokens,
            utility::compile_time_optional{},
            target);
    }
};
}  // namespace

int main() {
    const auto node = stub_node{policy::long_name<AR_STRING("test")>,
                                policy::runtime_enable{true}};
    node.pre_parse();
    return 0;
}
    )",
                               "Runtime enable requires at least 1 parent",
                               "runtime_enable_at_least_1_parent"},
                              {R"(
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/runtime_enable.hpp"
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

    parsing::pre_parse_result pre_parse() const
    {
        using this_policy =
            std::tuple_element_t<2, typename stub_node::policies_type>;

        auto unprocessed = std::vector<parsing::token_type>{};
        auto processed = std::vector<parsing::token_type>{};
        auto tokens = parsing::dynamic_token_adapter{unprocessed, unprocessed};

        auto target = parsing::parse_target{*this};

        return this->this_policy::template pre_parse_phase(
            tokens,
            utility::compile_time_optional{},
            target,
            *this);
    }
};
}  // namespace

int main() {
    const auto node = stub_node{policy::long_name<AR_STRING("test")>,
                                policy::required,
                                policy::runtime_enable{true}};
    node.pre_parse();
    return 0;
}
    )",
                               "Runtime enable must not be used with policy::required",
                               "runtime_enable_no_required"}});
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
