// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/policy/alias.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace arg_router::literals;
using namespace std::string_literals;

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

    template <typename ProcessedTarget, typename... Parents>
    parsing::pre_parse_result pre_parse_phase(
        parsing::dynamic_token_adapter& tokens,
        utility::compile_time_optional<ProcessedTarget> processed_target,
        parsing::parse_target& target,
        const Parents&... parents) const
    {
        auto retval = parsing::pre_parse_result{parsing::pre_parse_action::skip_node};
        utility::tuple_type_iterator<typename stub_node::policies_type>([&](auto i) {
            using this_policy = std::tuple_element_t<i, typename stub_node::policies_type>;
            if constexpr (traits::is_specialisation_of_v<this_policy, policy::alias_t>) {
                retval = this->this_policy::pre_parse_phase(tokens,
                                                            processed_target,
                                                            target,
                                                            parents...);
            }
        });

        return retval;
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

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(alias_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::alias_t<>>, "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(pre_parse_phase_test)
{
    const auto root = stub_node{
        policy::long_name_t{"test_root"_S},
        stub_node{policy::long_name_t{"test1"_S},
                  stub_node{policy::long_name_t{"flag1"_S},
                            policy::fixed_count<0>,
                            policy::alias(policy::long_name_t{"flag2"_S})},
                  stub_node{policy::long_name_t{"flag2"_S}, policy::fixed_count<0>},
                  stub_node{policy::long_name_t{"flag3"_S}},
                  policy::router{[](bool, bool, bool) {}}},
        stub_node{policy::long_name_t{"test2"_S},
                  stub_node{policy::long_name_t{"arg1"_S},
                            policy::fixed_count<1>,
                            policy::alias(policy::long_name_t{"arg3"_S})},
                  stub_node{policy::long_name_t{"arg2"_S}},
                  stub_node{policy::long_name_t{"arg3"_S}, policy::fixed_count<1>},
                  policy::router{[](bool, bool, bool) {}}},
        stub_node{policy::long_name_t{"test3"_S},
                  stub_node{policy::long_name_t{"flag1"_S},
                            policy::fixed_count<0>,
                            policy::alias(policy::long_name_t{"flag2"_S},
                                          policy::long_name_t{"flag3"_S})},
                  stub_node{policy::long_name_t{"flag2"_S}, policy::fixed_count<0>},
                  stub_node{policy::long_name_t{"flag3"_S}, policy::fixed_count<0>},
                  policy::router{[](bool, bool, bool) {}}},
        stub_node{
            policy::long_name_t{"test4"_S},
            stub_node{policy::long_name_t{"arg1"_S},
                      policy::fixed_count<3>,
                      policy::alias(policy::long_name_t{"arg2"_S}, policy::long_name_t{"arg3"_S})},
            stub_node{policy::long_name_t{"arg2"_S}, policy::fixed_count<3>},
            stub_node{policy::long_name_t{"arg3"_S}, policy::fixed_count<3>},
            policy::router{[](bool, bool, bool) {}}},
        stub_node{policy::long_name_t{"test5"_S},
                  stub_node{policy::long_name_t{"one_of"_S},
                            stub_node{policy::long_name_t{"flag1"_S},
                                      policy::fixed_count<0>,
                                      policy::alias(policy::long_name_t{"flag2"_S})},
                            stub_node{policy::long_name_t{"flag2"_S}, policy::fixed_count<0>}},
                  stub_node{policy::long_name_t{"flag3"_S}},
                  policy::router{[](bool, bool) {}}},
        stub_node{policy::long_name_t{"test6"_S},
                  stub_node{policy::long_name_t{"one_of"_S},
                            stub_node{policy::long_name_t{"flag1"_S},
                                      policy::fixed_count<0>,
                                      policy::alias(policy::long_name_t{"flag3"_S})},
                            stub_node{policy::long_name_t{"flag2"_S}}},
                  stub_node{policy::long_name_t{"flag3"_S}, policy::fixed_count<0>},
                  policy::router{[](bool, bool) {}}},
        stub_node{policy::long_name_t{"test7"_S},
                  stub_node{policy::long_name_t{"flag1"_S},
                            policy::fixed_count<0>,
                            policy::alias(policy::long_name_t{"パラメータ一"_S})},
                  stub_node{policy::long_name_t{"パラメータ一"_S}, policy::fixed_count<0>},
                  stub_node{policy::long_name_t{"flag3"_S}},
                  policy::router{[](bool, bool, bool) {}}},
    };

    auto f = [&](auto args, auto expected_target_data, auto expected_args, auto parents_tuple) {
        auto result = std::vector<parsing::token_type>{};

        utility::apply(
            [&](auto&& node, auto&&... parents) {
                auto adapter = parsing::dynamic_token_adapter{result, args};
                auto target = parsing::parse_target{node.get(), (parents.get())...};

                const auto match = node.get().pre_parse_phase(adapter,
                                                              utility::compile_time_optional{},
                                                              target,
                                                              node.get(),
                                                              (parents.get())...);
                BOOST_CHECK_EQUAL(match, parsing::pre_parse_action::skip_node_but_use_sub_targets);

                BOOST_CHECK(target.tokens().empty());
                BOOST_REQUIRE_EQUAL(expected_target_data.size(), target.sub_targets().size());
                for (auto i = 0u; i < expected_target_data.size(); ++i) {
                    auto& sub_target = target.sub_targets()[i];
                    const auto& expected_sub_target = expected_target_data[i];

                    BOOST_CHECK_EQUAL(expected_sub_target.tokens, sub_target.tokens());

                    expected_target_and_parents = std::move(expected_sub_target.target_and_parents);
                    BOOST_CHECK(sub_target);
                    sub_target();
                    BOOST_CHECK(!sub_target);
                }
            },
            parents_tuple);

        BOOST_CHECK_EQUAL(args, expected_args);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "--flag1"}},
                       std::vector{make_pre_parse_test_data<0, 1>(root, {})},
                       std::vector<parsing::token_type>{},
                       test::get_parents<0, 0>(root)},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "--flag1"},
                                                        {parsing::prefix_type::none, "foo"}},
                       std::vector{make_pre_parse_test_data<0, 1>(root, {})},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "foo"}},
                       test::get_parents<0, 0>(root)},
            std::tuple{
                std::vector<parsing::token_type>{{parsing::prefix_type::none, "--arg1"},
                                                 {parsing::prefix_type::none, "42"}},
                std::vector{
                    make_pre_parse_test_data<1, 2>(root, {{parsing::prefix_type::none, "42"}})},
                std::vector<parsing::token_type>{},
                test::get_parents<1, 0>(root)},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "--flag1"}},
                       std::vector{make_pre_parse_test_data<2, 1>(root, {}),
                                   make_pre_parse_test_data<2, 2>(root, {})},
                       std::vector<parsing::token_type>{},
                       test::get_parents<2, 0>(root)},
            std::tuple{
                std::vector<parsing::token_type>{{parsing::prefix_type::none, "--arg1"},
                                                 {parsing::prefix_type::none, "1"},
                                                 {parsing::prefix_type::none, "2"},
                                                 {parsing::prefix_type::none, "3"},
                                                 {parsing::prefix_type::none, "4"}},
                std::vector{make_pre_parse_test_data<3, 1>(root,
                                                           {{parsing::prefix_type::none, "1"},
                                                            {parsing::prefix_type::none, "2"},
                                                            {parsing::prefix_type::none, "3"}}),
                            make_pre_parse_test_data<3, 2>(root,
                                                           {{parsing::prefix_type::none, "1"},
                                                            {parsing::prefix_type::none, "2"},
                                                            {parsing::prefix_type::none, "3"}})},
                std::vector<parsing::token_type>{{parsing::prefix_type::none, "4"}},
                test::get_parents<3, 0>(root)},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "--flag1"}},
                       std::vector{make_pre_parse_test_data<4, 0, 1>(root, {})},
                       std::vector<parsing::token_type>{},
                       test::get_parents<4, 0, 0>(root)},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "--flag1"}},
                       std::vector{make_pre_parse_test_data<5, 1>(root, {})},
                       std::vector<parsing::token_type>{},
                       test::get_parents<5, 0, 0>(root)},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "--flag1"}},
                       std::vector{make_pre_parse_test_data<6, 1>(root, {})},
                       std::vector<parsing::token_type>{},
                       test::get_parents<6, 0>(root)},
        });
}

BOOST_AUTO_TEST_CASE(pre_parse_phase_too_small_view_test)
{
    const auto root = stub_node{policy::long_name_t{"root"_S},
                                stub_node{policy::long_name_t{"arg1"_S},
                                          policy::fixed_count<2>,
                                          policy::alias(policy::long_name_t{"arg2"_S})},
                                stub_node{policy::long_name_t{"arg2"_S}, policy::fixed_count<2>},
                                stub_node{policy::long_name_t{"arg3"_S}},
                                policy::router{[](bool, bool, bool) {}}};

    auto result = std::vector<parsing::token_type>{{parsing::prefix_type::long_, "arg1"},
                                                   {parsing::prefix_type::none, "42"}};
    const auto& owner = std::get<0>(root.children());
    auto args = std::vector<parsing::token_type>{};
    auto adapter = parsing::dynamic_token_adapter{result, args};
    auto target = parsing::parse_target{owner, root};

    const auto match =
        owner.pre_parse_phase(adapter, utility::compile_time_optional{}, target, owner, root);

    BOOST_CHECK_EXCEPTION((void)match.get(), multi_lang_exception, [](const auto& e) {
        return (e.ec() == error_code::too_few_values_for_alias) && (e.tokens().size() == 1) &&
               (e.tokens().front() == parsing::token_type{parsing::prefix_type::long_, "arg1"});
    });
}

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile(
        {{R"(
#include "arg_router/policy/alias.hpp"

using namespace arg_router;

int main() {
    auto a = policy::alias();
    return 0;
}
    )",
          "At least one name needed for alias",
          "zero_aliases_test"},
         {
             R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    auto a = policy::alias(flag(policy::long_name_t{"flag1"_S}));
    return 0;
}
    )",
             "All parameters must be policies",
             "all_params_must_be_policies_test"},
         {
             R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/display_name.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    auto a = policy::alias(policy::display_name_t{"hello"_S});
    return 0;
}
    )",
             "All parameters must provide a long and/or short form name",
             "all_params_must_be_names_test"},
         {
             R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
    void pre_parse_phase(
        std::vector<parsing::token_type>& result,
        [[maybe_unused]] const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<0, typename stub_node::policies_type>;
            
        auto args = std::vector<parsing::token_type>{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        auto target = parsing::parse_target{*this, parents...};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 utility::compile_time_optional{},
                                                 target,
                                                 *this,
                                                 parents...);
    }
};
}  // namespace

int main() {
    const auto root = stub_node{policy::alias(policy::long_name_t{"flag2"_S}),
                                policy::fixed_count<0>};

    auto result = std::vector<parsing::token_type>{
                        {parsing::prefix_type::long_, "flag2"},
                        {parsing::prefix_type::long_, "flag3"}};
    root.pre_parse_phase(result);
    return 0;
}
    )",
             "Cannot find parent mode",
             "cannot_find_parent_node_empty_test"},
         {
             R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
    void pre_parse_phase(
        std::vector<parsing::token_type>& result,
        [[maybe_unused]] const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<1, typename stub_node::policies_type>;
            
        auto args = std::vector<parsing::token_type>{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        auto target = parsing::parse_target{*this, parents...};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 utility::compile_time_optional{},
                                                 target,
                                                 *this,
                                                 parents...);
    }
};
}  // namespace

int main() {
    const auto root =
        stub_node{policy::long_name_t{"mode"_S},
              stub_node{policy::long_name_t{"flag1"_S},
                        policy::alias(policy::long_name_t{"flag2"_S})},
              stub_node{policy::long_name_t{"flag2"_S}}};;

    auto result = std::vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "flag2"}};
    const auto& owner = std::get<0>(root.children());

    owner.pre_parse_phase(result, root);
    return 0;
}
    )",
             "Aliased nodes must have minimum and maximum count methods",
             "alias_must_have_minimum_and_maximum_count_methods_test"},
         {
             R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
    void pre_parse_phase(
        std::vector<parsing::token_type>& result,
        [[maybe_unused]] const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<2, typename stub_node::policies_type>;
            
        auto args = std::vector<parsing::token_type>{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        auto target = parsing::parse_target{*this, parents...};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 utility::compile_time_optional{},
                                                 target,
                                                 *this,
                                                 parents...);
    }
};
}  // namespace

int main() {
    const auto root =
        stub_node{policy::long_name_t{"mode"_S},
              stub_node{policy::long_name_t{"flag1"_S},
                        policy::min_count<2>,
                        policy::alias(policy::long_name_t{"flag2"_S})},
              stub_node{policy::long_name_t{"flag2"_S}}};;

    auto result = std::vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "flag2"}};
    const auto& owner = std::get<0>(root.children());

    owner.pre_parse_phase(result, root);
    return 0;
}
    )",
             "Aliased nodes must have a fixed count",
             "alias_must_have_fixed_count_test"},
         {
             R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
    void pre_parse_phase(
        std::vector<parsing::token_type>& result,
        [[maybe_unused]] const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<1, typename stub_node::policies_type>;
            
        auto args = std::vector<parsing::token_type>{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        auto target = parsing::parse_target{*this, parents...};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 utility::compile_time_optional{},
                                                 target,
                                                 *this,
                                                 parents...);
    }
};
}  // namespace

int main() {
    const auto root =
        stub_node{policy::long_name_t{"mode"_S},
              stub_node{policy::long_name_t{"flag1"_S},
                        policy::alias(policy::long_name_t{"flag2"_S})},
              stub_node{policy::long_name_t{"flag2"_S}},
              stub_node{policy::long_name_t{"flag3"_S}}};

    auto result = std::vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "flag2"},
                    {parsing::prefix_type::long_, "flag3"}};
    const auto& owner = std::get<0>(root.children());

    owner.pre_parse_phase(result, root);
    return 0;
}
    )",
             "Cannot find parent mode",
             "cannot_find_parent_node_missing_test"},
         {
             R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
    void pre_parse_phase(
        std::vector<parsing::token_type>& result,
        [[maybe_unused]] const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<1, typename stub_node::policies_type>;
            
        auto args = std::vector<parsing::token_type>{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        auto target = parsing::parse_target{*this, parents...};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 utility::compile_time_optional{},
                                                 target,
                                                 *this,
                                                 parents...);
    }
};
}  // namespace

int main() {
    const auto root =
        stub_node{policy::long_name_t{"mode"_S},
                  stub_node{policy::long_name_t{"flag1"_S},
                            policy::alias(policy::long_name_t{"flag2"_S}),
                            policy::fixed_count<1>},
                  stub_node{policy::long_name_t{"flag2"_S},
                            policy::alias(policy::long_name_t{"flag3"_S}),
                            policy::fixed_count<1>},
                  stub_node{policy::long_name_t{"flag3"_S},
                            policy::alias(policy::long_name_t{"flag1"_S}),
                            policy::fixed_count<1>},
                  policy::router{[](bool, bool, bool) {}}};

    auto result = std::vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "flag2"},
                    {parsing::prefix_type::long_, "flag3"}};
    const auto& owner = std::get<0>(root.children());

    owner.pre_parse_phase(result, root);
    return 0;
}
    )",
             "Cyclic dependency detected",
             "cyclic_dependency_test"},
         {
             R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
    void pre_parse_phase(
        std::vector<parsing::token_type>& result,
        [[maybe_unused]] const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<1, typename stub_node::policies_type>;
            
        auto args = std::vector<parsing::token_type>{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        auto target = parsing::parse_target{*this, parents...};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 utility::compile_time_optional{},
                                                 target,
                                                 *this,
                                                 parents...);
    }
};
}  // namespace

int main() {
    const auto root =
        stub_node{policy::long_name_t{"mode"_S},
                  stub_node{policy::long_name_t{"flag1"_S},
                            policy::alias(policy::long_name_t{"flag4"_S}),
                            policy::fixed_count<1>},
                  stub_node{policy::long_name_t{"flag2"_S}},
                  stub_node{policy::long_name_t{"flag3"_S}},
                  policy::router{[](bool, bool, bool) {}}};

    auto result = std::vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "flag2"},
                    {parsing::prefix_type::long_, "flag3"}};
    const auto& owner = std::get<0>(root.children());

    owner.pre_parse_phase(result, root);
    return 0;
}
    )",
             "Number of found modes must match alias policy count",
             "missing_target_test"},
         {
             R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
    void pre_parse_phase(
        std::vector<parsing::token_type>& result,
        [[maybe_unused]] const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<1, typename stub_node::policies_type>;
            
        auto args = std::vector<parsing::token_type>{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        auto target = parsing::parse_target{*this, parents...};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 utility::compile_time_optional{},
                                                 target,
                                                 *this,
                                                 parents...);
    }
};
}  // namespace

int main() {
    const auto root =
        stub_node{policy::long_name_t{"mode"_S},
                  stub_node{policy::long_name_t{"flag1"_S},
                            policy::alias(policy::long_name_t{"flag2"_S},
                                          policy::long_name_t{"flag2"_S}),
                            policy::fixed_count<1>},
                  stub_node{policy::long_name_t{"flag2"_S}},
                  stub_node{policy::long_name_t{"flag3"_S}},
                  policy::router{[](bool, bool, bool) {}}};

    auto result = std::vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "flag2"},
                    {parsing::prefix_type::long_, "flag3"}};
    const auto& owner = std::get<0>(root.children());

    owner.pre_parse_phase(result, root);
    return 0;
}
    )",
             "Number of found modes must match alias policy count",
             "duplicate_targets_test"},
         {
             R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
    void pre_parse_phase(
        std::vector<parsing::token_type>& result,
        [[maybe_unused]] const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<1, typename stub_node::policies_type>;
            
        auto args = std::vector<parsing::token_type>{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        auto target = parsing::parse_target{*this, parents...};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 utility::compile_time_optional{},
                                                 target,
                                                 *this,
                                                 parents...);
    }
};
}  // namespace

int main() {
    const auto root =
        stub_node{policy::long_name_t{"mode"_S},
                  stub_node{policy::long_name_t{"flag1"_S},
                            policy::alias(policy::long_name_t{"flag2"_S},
                                          policy::short_name_t{"a"_S}),
                            policy::fixed_count<1>},
                  stub_node{policy::long_name_t{"flag2"_S},
                            policy::short_name_t{"a"_S}},
                  stub_node{policy::long_name_t{"flag3"_S}},
                  policy::router{[](bool, bool, bool) {}}};

    auto result = std::vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "flag2"},
                    {parsing::prefix_type::long_, "flag3"}};
    const auto& owner = std::get<0>(root.children());

    owner.pre_parse_phase(result, root);
    return 0;
}
    )",
             "Node alias list must be unique, do you have short and long names from "
             "the same node?",
             "duplicate_target_different_name_types_test"},
         {
             R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/custom_parser.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
    void pre_parse_phase(
        std::vector<parsing::token_type>& result,
        [[maybe_unused]] const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<1, typename stub_node::policies_type>;
            
        auto args = std::vector<parsing::token_type>{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        auto target = parsing::parse_target{*this, parents...};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 utility::compile_time_optional{},
                                                 target,
                                                 *this,
                                                 parents...);
    }
};
}  // namespace

int main() {
    const auto root = stub_node{policy::long_name_t{"flag1"_S},
                                policy::alias(policy::long_name_t{"flag2"_S}),
                                policy::custom_parser<bool>{
                                    [](std::string_view) { return false; }}};

    auto result = std::vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "flag2"},
                    {parsing::prefix_type::long_, "flag3"}};

    root.pre_parse_phase(result, root);
    return 0;
}
    )",
             "Alias owning node cannot have policies that support parse, "
             "validation, or routing phases",
             "parse_phase_test"},
         {
             R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
    void pre_parse_phase(
        std::vector<parsing::token_type>& result,
        [[maybe_unused]] const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<1, typename stub_node::policies_type>;
            
        auto args = std::vector<parsing::token_type>{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        auto target = parsing::parse_target{*this, parents...};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 utility::compile_time_optional{},
                                                 target,
                                                 *this,
                                                 parents...);
    }
};
}  // namespace

int main() {
    const auto root = stub_node{policy::long_name_t{"flag1"_S},
                                policy::alias(policy::long_name_t{"flag2"_S}),
                                policy::min_max_value<3, 6>()};

    auto result = std::vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "flag2"},
                    {parsing::prefix_type::long_, "flag3"}};

    root.pre_parse_phase(result, root);
    return 0;
}
    )",
             "Alias owning node cannot have policies that support parse, "
             "validation, or routing phases",
             "validation_phase_test"},
         {
             R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
    void pre_parse_phase(
        std::vector<parsing::token_type>& result,
        [[maybe_unused]] const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<1, typename stub_node::policies_type>;

        auto args = std::vector<parsing::token_type>{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        auto target = parsing::parse_target{*this, parents...};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 utility::compile_time_optional{},
                                                 target,
                                                 *this,
                                                 parents...);
    }
};
}  // namespace

int main() {
    const auto root = stub_node{policy::long_name_t{"flag1"_S},
                                policy::alias(policy::long_name_t{"flag2"_S}),
                                policy::router{[](bool) {}}};

    auto result = std::vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "flag2"},
                    {parsing::prefix_type::long_, "flag3"}};

    root.pre_parse_phase(result, root);
    return 0;
}
    )",
             "Alias owning node cannot have policies that support parse, "
             "validation, or routing phases",
             "routing_phase_test"},
         {
             R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
    void pre_parse_phase(
        std::vector<parsing::token_type>& result,
        [[maybe_unused]] const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<1, typename stub_node::policies_type>;

        auto args = std::vector<parsing::token_type>{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        auto target = parsing::parse_target{*this, parents...};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 utility::compile_time_optional{},
                                                 target,
                                                 *this,
                                                 parents...);
    }

    template <typename... Parents>
    void parse([[maybe_unused]] parsing::parse_target&& target,
               [[maybe_unused]] const Parents&... parents) const {}
};
}  // namespace

int main() {
    const auto root =
        stub_node{policy::long_name_t{"mode"_S},
                  stub_node{policy::long_name_t{"flag1"_S},
                            policy::alias(policy::long_name_t{"flag2"_S}),
                            policy::fixed_count<1>},
                  stub_node{policy::long_name_t{"flag2"_S},
                            policy::fixed_count<2>},
                  policy::router{[](bool, bool) {}}};

    auto result = std::vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "flag2"}};

    const auto& owner = std::get<0>(root.children());
    owner.pre_parse_phase(result, root);
    return 0;
}
    )",
             "All alias targets must have a count that matches the owner",
             "target_counts_test"}});
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
