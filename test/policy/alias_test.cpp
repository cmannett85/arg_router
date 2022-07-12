/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/policy/alias.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace std::string_literals;

namespace
{
std::vector<utility::unsafe_any> expected_target_and_parents;

template <typename Node, typename... Parents>
void parse_checker(parsing::parse_target target, const Node& node, const Parents&... parents)
{
    BOOST_CHECK_EQUAL(std::type_index{typeid(Node)}.hash_code(), target.node_type().hash_code());

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
        policy::long_name<S_("test_root")>,
        stub_node{policy::long_name<S_("test1")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::fixed_count<0>,
                            policy::alias(policy::long_name<S_("flag2")>)},
                  stub_node{policy::long_name<S_("flag2")>, policy::fixed_count<0>},
                  stub_node{policy::long_name<S_("flag3")>},
                  policy::router{[](bool, bool, bool) {}}},
        stub_node{policy::long_name<S_("test2")>,
                  stub_node{policy::long_name<S_("arg1")>,
                            policy::fixed_count<1>,
                            policy::alias(policy::long_name<S_("arg3")>)},
                  stub_node{policy::long_name<S_("arg2")>},
                  stub_node{policy::long_name<S_("arg3")>, policy::fixed_count<1>},
                  policy::router{[](bool, bool, bool) {}}},
        stub_node{policy::long_name<S_("test3")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::fixed_count<0>,
                            policy::alias(policy::long_name<S_("flag2")>,
                                          policy::long_name<S_("flag3")>)},
                  stub_node{policy::long_name<S_("flag2")>, policy::fixed_count<0>},
                  stub_node{policy::long_name<S_("flag3")>, policy::fixed_count<0>},
                  policy::router{[](bool, bool, bool) {}}},
        stub_node{
            policy::long_name<S_("test4")>,
            stub_node{policy::long_name<S_("arg1")>,
                      policy::fixed_count<3>,
                      policy::alias(policy::long_name<S_("arg2")>, policy::long_name<S_("arg3")>)},
            stub_node{policy::long_name<S_("arg2")>, policy::fixed_count<3>},
            stub_node{policy::long_name<S_("arg3")>, policy::fixed_count<3>},
            policy::router{[](bool, bool, bool) {}}},
        stub_node{policy::long_name<S_("test5")>,
                  stub_node{policy::long_name<S_("one_of")>,
                            stub_node{policy::long_name<S_("flag1")>,
                                      policy::fixed_count<0>,
                                      policy::alias(policy::long_name<S_("flag2")>)},
                            stub_node{policy::long_name<S_("flag2")>, policy::fixed_count<0>}},
                  stub_node{policy::long_name<S_("flag3")>},
                  policy::router{[](bool, bool) {}}},
        stub_node{policy::long_name<S_("test6")>,
                  stub_node{policy::long_name<S_("one_of")>,
                            stub_node{policy::long_name<S_("flag1")>,
                                      policy::fixed_count<0>,
                                      policy::alias(policy::long_name<S_("flag3")>)},
                            stub_node{policy::long_name<S_("flag2")>}},
                  stub_node{policy::long_name<S_("flag3")>, policy::fixed_count<0>},
                  policy::router{[](bool, bool) {}}},
        stub_node{policy::long_name<S_("test7")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::fixed_count<0>,
                            policy::alias(policy::long_name<S_("パラメータ一")>)},
                  stub_node{policy::long_name<S_("パラメータ一")>, policy::fixed_count<0>},
                  stub_node{policy::long_name<S_("flag3")>},
                  policy::router{[](bool, bool, bool) {}}},
    };

    auto f = [&](auto args, auto expected_target_data, auto expected_args, auto parents_tuple) {
        auto result = vector<parsing::token_type>{};

        std::apply(
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
    const auto root = stub_node{policy::long_name<S_("root")>,
                                stub_node{policy::long_name<S_("arg1")>,
                                          policy::fixed_count<2>,
                                          policy::alias(policy::long_name<S_("arg2")>)},
                                stub_node{policy::long_name<S_("arg2")>, policy::fixed_count<2>},
                                stub_node{policy::long_name<S_("arg3")>},
                                policy::router{[](bool, bool, bool) {}}};

    auto result = std::vector<parsing::token_type>{{parsing::prefix_type::long_, "arg1"},
                                                   {parsing::prefix_type::none, "42"}};
    const auto& owner = std::get<0>(root.children());
    auto args = std::vector<parsing::token_type>{};
    auto adapter = parsing::dynamic_token_adapter{result, args};
    auto target = parsing::parse_target{owner, root};

    const auto match =
        owner.pre_parse_phase(adapter, utility::compile_time_optional{}, target, owner, root);

    BOOST_CHECK_EXCEPTION(match.get(), parse_exception, [](const auto& e) {
        return e.what() == "Too few values for alias, needs 2: --arg1"s;
    });
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(zero_aliases_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/alias.hpp"

using namespace arg_router;

int main() {
    auto a = policy::alias();
    return 0;
}
    )",
        "At least one name needed for alias");
}

BOOST_AUTO_TEST_CASE(all_params_must_be_policies_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto a = policy::alias(flag(policy::long_name<S_("flag1")>));
    return 0;
}
    )",
        "All parameters must be policies");
}

BOOST_AUTO_TEST_CASE(all_params_must_be_names_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/display_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto a = policy::alias(policy::display_name<S_("hello")>);
    return 0;
}
    )",
        "All parameters must provide a long and/or short form name");
}

BOOST_AUTO_TEST_CASE(cannot_find_parent_node_empty_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
    const auto root = stub_node{policy::alias(policy::long_name<S_("flag2")>),
                                policy::fixed_count<0>};

    auto result = vector<parsing::token_type>{
                        {parsing::prefix_type::long_, "flag2"},
                        {parsing::prefix_type::long_, "flag3"}};
    root.pre_parse_phase(result);
    return 0;
}
    )",
        "Cannot find parent mode");
}

BOOST_AUTO_TEST_CASE(alias_must_have_minimum_and_maximum_count_methods_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
        stub_node{policy::long_name<S_("mode")>,
              stub_node{policy::long_name<S_("flag1")>,
                        policy::alias(policy::long_name<S_("flag2")>)},
              stub_node{policy::long_name<S_("flag2")>}};;

    auto result = vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "flag2"}};
    const auto& owner = std::get<0>(root.children());

    owner.pre_parse_phase(result, root);
    return 0;
}
    )",
        "Aliased nodes must have minimum and maximum count methods");
}

BOOST_AUTO_TEST_CASE(alias_must_have_fixed_count_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
        stub_node{policy::long_name<S_("mode")>,
              stub_node{policy::long_name<S_("flag1")>,
                        policy::min_count<2>,
                        policy::alias(policy::long_name<S_("flag2")>)},
              stub_node{policy::long_name<S_("flag2")>}};;

    auto result = vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "flag2"}};
    const auto& owner = std::get<0>(root.children());

    owner.pre_parse_phase(result, root);
    return 0;
}
    )",
        "Aliased nodes must have a fixed count");
}

BOOST_AUTO_TEST_CASE(cannot_find_parent_node_missing_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
        stub_node{policy::long_name<S_("mode")>,
              stub_node{policy::long_name<S_("flag1")>,
                        policy::alias(policy::long_name<S_("flag2")>)},
              stub_node{policy::long_name<S_("flag2")>},
              stub_node{policy::long_name<S_("flag3")>}};

    auto result = vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "flag2"},
                    {parsing::prefix_type::long_, "flag3"}};
    const auto& owner = std::get<0>(root.children());

    owner.pre_parse_phase(result, root);
    return 0;
}
    )",
        "Cannot find parent mode");
}

BOOST_AUTO_TEST_CASE(cyclic_dependency_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
        stub_node{policy::long_name<S_("mode")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::alias(policy::long_name<S_("flag2")>),
                            policy::fixed_count<1>},
                  stub_node{policy::long_name<S_("flag2")>,
                            policy::alias(policy::long_name<S_("flag3")>),
                            policy::fixed_count<1>},
                  stub_node{policy::long_name<S_("flag3")>,
                            policy::alias(policy::long_name<S_("flag1")>),
                            policy::fixed_count<1>},
                  policy::router{[](bool, bool, bool) {}}};

    auto result = vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "flag2"},
                    {parsing::prefix_type::long_, "flag3"}};
    const auto& owner = std::get<0>(root.children());

    owner.pre_parse_phase(result, root);
    return 0;
}
    )",
        "Cyclic dependency detected");
}

BOOST_AUTO_TEST_CASE(missing_target_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
        stub_node{policy::long_name<S_("mode")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::alias(policy::long_name<S_("flag4")>),
                            policy::fixed_count<1>},
                  stub_node{policy::long_name<S_("flag2")>},
                  stub_node{policy::long_name<S_("flag3")>},
                  policy::router{[](bool, bool, bool) {}}};

    auto result = vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "flag2"},
                    {parsing::prefix_type::long_, "flag3"}};
    const auto& owner = std::get<0>(root.children());

    owner.pre_parse_phase(result, root);
    return 0;
}
    )",
        "Number of found modes must match alias policy count");
}

BOOST_AUTO_TEST_CASE(duplicate_targets_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
        stub_node{policy::long_name<S_("mode")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::alias(policy::long_name<S_("flag2")>,
                                          policy::long_name<S_("flag2")>),
                            policy::fixed_count<1>},
                  stub_node{policy::long_name<S_("flag2")>},
                  stub_node{policy::long_name<S_("flag3")>},
                  policy::router{[](bool, bool, bool) {}}};

    auto result = vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "flag2"},
                    {parsing::prefix_type::long_, "flag3"}};
    const auto& owner = std::get<0>(root.children());

    owner.pre_parse_phase(result, root);
    return 0;
}
    )",
        "Number of found modes must match alias policy count");
}

BOOST_AUTO_TEST_CASE(duplicate_target_different_name_types_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
        stub_node{policy::long_name<S_("mode")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::alias(policy::long_name<S_("flag2")>,
                                          policy::short_name<'a'>),
                            policy::fixed_count<1>},
                  stub_node{policy::long_name<S_("flag2")>,
                            policy::short_name<'a'>},
                  stub_node{policy::long_name<S_("flag3")>},
                  policy::router{[](bool, bool, bool) {}}};

    auto result = vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "flag2"},
                    {parsing::prefix_type::long_, "flag3"}};
    const auto& owner = std::get<0>(root.children());

    owner.pre_parse_phase(result, root);
    return 0;
}
    )",
        "Node alias list must be unique, do you have short and long names from "
        "the same node?");
}

BOOST_AUTO_TEST_CASE(parse_phase_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/custom_parser.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
    const auto root = stub_node{policy::long_name<S_("flag1")>,
                                policy::alias(policy::long_name<S_("flag2")>),
                                policy::custom_parser<bool>{
                                    [](std::string_view) { return false; }}};

    auto result = vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "flag2"},
                    {parsing::prefix_type::long_, "flag3"}};

    root.pre_parse_phase(result, root);
    return 0;
}
    )",
        "Alias owning node cannot have policies that support parse, "
        "validation, or routing phases");
}

BOOST_AUTO_TEST_CASE(validation_phase_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
    const auto root = stub_node{policy::long_name<S_("flag1")>,
                                policy::alias(policy::long_name<S_("flag2")>),
                                policy::min_max_value{3, 6}};

    auto result = vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "flag2"},
                    {parsing::prefix_type::long_, "flag3"}};

    root.pre_parse_phase(result, root);
    return 0;
}
    )",
        "Alias owning node cannot have policies that support parse, "
        "validation, or routing phases");
}

BOOST_AUTO_TEST_CASE(routing_phase_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
    const auto root = stub_node{policy::long_name<S_("flag1")>,
                                policy::alias(policy::long_name<S_("flag2")>),
                                policy::router{[](bool) {}}};

    auto result = vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "flag2"},
                    {parsing::prefix_type::long_, "flag3"}};

    root.pre_parse_phase(result, root);
    return 0;
}
    )",
        "Alias owning node cannot have policies that support parse, "
        "validation, or routing phases");
}

BOOST_AUTO_TEST_CASE(target_counts_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
        stub_node{policy::long_name<S_("mode")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::alias(policy::long_name<S_("flag2")>),
                            policy::fixed_count<1>},
                  stub_node{policy::long_name<S_("flag2")>,
                            policy::fixed_count<2>},
                  policy::router{[](bool, bool) {}}};

    auto result = vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "flag2"}};

    const auto& owner = std::get<0>(root.children());
    owner.pre_parse_phase(result, root);
    return 0;
}
    )",
        "All alias targets must have a count that matches the owner");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
