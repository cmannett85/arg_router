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
    parsing::pre_parse_result pre_parse_phase(
        parsing::dynamic_token_adapter& tokens,
        [[maybe_unused]] parsing::token_list::pending_view_type
            processed_tokens,
        [[maybe_unused]] const Parents&... parents) const
    {
        auto retval =
            parsing::pre_parse_result{parsing::pre_parse_action::skip_node};
        utility::tuple_type_iterator<typename stub_node::policies_type>(
            [&](auto i) {
                using this_policy =
                    std::tuple_element_t<i, typename stub_node::policies_type>;
                if constexpr (traits::is_specialisation_of_v<this_policy,
                                                             policy::alias_t>) {
                    retval =
                        this->this_policy::pre_parse_phase(tokens,
                                                           processed_tokens,
                                                           parents...);
                }
            });

        return retval;
    }
};

template <std::size_t I, std::size_t... Is, typename T>
const auto& get_node(const T& parent)
{
    const auto& child = std::get<I>(parent.children());

    if constexpr (sizeof...(Is) > 0) {
        return get_node<Is...>(child);
    } else {
        return child;
    }
}

template <std::size_t I, std::size_t... Is, typename T>
auto get_parents(const T& parent)
{
    auto result = std::tuple{std::ref(get_node<I, Is...>(parent))};

    if constexpr (sizeof...(Is) > 0) {
        // All this because you can't resize a tuple in std...
        using index_tuple = boost::mp11::mp_pop_back<
            std::tuple<traits::integral_constant<I>,
                       traits::integral_constant<Is>...>>;
        return std::apply(
            [&](auto... NewIs) {
                return std::tuple_cat(result, get_parents<NewIs...>(parent));
            },
            index_tuple{});
    } else {
        return result;
    }
}
}  // namespace

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(alias_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::alias_t<>>,
                  "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(pre_parse_phase_test)
{
    const auto root = stub_node{
        policy::long_name<S_("test_root")>,
        stub_node{
            policy::long_name<S_("test1")>,
            stub_node{policy::long_name<S_("flag1")>,
                      policy::fixed_count<0>,
                      policy::alias(policy::long_name<S_("flag2")>)},
            stub_node{policy::long_name<S_("flag2")>, policy::fixed_count<0>},
            stub_node{policy::long_name<S_("flag3")>},
            policy::router{[](bool, bool, bool) {}}},
        stub_node{
            policy::long_name<S_("test2")>,
            stub_node{policy::long_name<S_("arg1")>,
                      policy::fixed_count<1>,
                      policy::alias(policy::long_name<S_("arg3")>)},
            stub_node{policy::long_name<S_("arg2")>},
            stub_node{policy::long_name<S_("arg3")>, policy::fixed_count<1>},
            policy::router{[](bool, bool, bool) {}}},
        stub_node{
            policy::long_name<S_("test3")>,
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
                      policy::alias(policy::long_name<S_("arg2")>,
                                    policy::long_name<S_("arg3")>)},
            stub_node{policy::long_name<S_("arg2")>, policy::fixed_count<3>},
            stub_node{policy::long_name<S_("arg3")>, policy::fixed_count<3>},
            policy::router{[](bool, bool, bool) {}}},
        stub_node{
            policy::long_name<S_("test5")>,
            stub_node{policy::long_name<S_("one_of")>,
                      stub_node{policy::long_name<S_("flag1")>,
                                policy::fixed_count<0>,
                                policy::alias(policy::long_name<S_("flag2")>)},
                      stub_node{policy::long_name<S_("flag2")>,
                                policy::fixed_count<0>}},
            stub_node{policy::long_name<S_("flag3")>},
            policy::router{[](bool, bool) {}}},
        stub_node{
            policy::long_name<S_("test6")>,
            stub_node{policy::long_name<S_("one_of")>,
                      stub_node{policy::long_name<S_("flag1")>,
                                policy::fixed_count<0>,
                                policy::alias(policy::long_name<S_("flag3")>)},
                      stub_node{policy::long_name<S_("flag2")>}},
            stub_node{policy::long_name<S_("flag3")>, policy::fixed_count<0>},
            policy::router{[](bool, bool) {}}}};

    auto f = [&](auto args,
                 auto expected_result,
                 auto expected_args,
                 const auto parents_tuple) {
        auto result = vector<parsing::token_type>{};
        auto processed_tokens = parsing::token_list{};

        std::apply(
            [&](auto&&... parents) {
                auto adapter = parsing::dynamic_token_adapter{result, args};
                const auto match =
                    std::get<0>(parents_tuple)
                        .get()
                        .pre_parse_phase(adapter,
                                         processed_tokens.pending_view(),
                                         (parents.get())...,
                                         root);
                BOOST_CHECK_EQUAL(
                    match,
                    parsing::pre_parse_action::skip_node_but_use_tokens);
            },
            parents_tuple);

        BOOST_CHECK_EQUAL(result, expected_result);
        BOOST_CHECK_EQUAL(args, expected_args);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{std::vector<parsing::token_type>{
                                      {parsing::prefix_type::none, "--flag1"}},
                                  std::vector<parsing::token_type>{
                                      {parsing::prefix_type::long_, "flag2"}},
                                  std::vector<parsing::token_type>{},
                                  get_parents<0, 0>(root)},
                       std::tuple{std::vector<parsing::token_type>{
                                      {parsing::prefix_type::none, "--flag1"},
                                      {parsing::prefix_type::none, "foo"}},
                                  std::vector<parsing::token_type>{
                                      {parsing::prefix_type::long_, "flag2"}},
                                  std::vector<parsing::token_type>{
                                      {parsing::prefix_type::none, "foo"}},
                                  get_parents<0, 0>(root)},
                       std::tuple{std::vector<parsing::token_type>{
                                      {parsing::prefix_type::none, "--arg1"},
                                      {parsing::prefix_type::none, "42"}},
                                  std::vector<parsing::token_type>{
                                      {parsing::prefix_type::long_, "arg3"},
                                      {parsing::prefix_type::none, "42"}},
                                  std::vector<parsing::token_type>{},
                                  get_parents<1, 0>(root)},
                       std::tuple{std::vector<parsing::token_type>{
                                      {parsing::prefix_type::none, "--flag1"}},
                                  std::vector<parsing::token_type>{
                                      {parsing::prefix_type::long_, "flag2"},
                                      {parsing::prefix_type::long_, "flag3"}},
                                  std::vector<parsing::token_type>{},
                                  get_parents<2, 0>(root)},
                       std::tuple{std::vector<parsing::token_type>{
                                      {parsing::prefix_type::none, "--arg1"},
                                      {parsing::prefix_type::none, "1"},
                                      {parsing::prefix_type::none, "2"},
                                      {parsing::prefix_type::none, "3"},
                                      {parsing::prefix_type::none, "4"}},
                                  std::vector<parsing::token_type>{
                                      {parsing::prefix_type::long_, "arg2"},
                                      {parsing::prefix_type::none, "1"},
                                      {parsing::prefix_type::none, "2"},
                                      {parsing::prefix_type::none, "3"},
                                      {parsing::prefix_type::long_, "arg3"},
                                      {parsing::prefix_type::none, "1"},
                                      {parsing::prefix_type::none, "2"},
                                      {parsing::prefix_type::none, "3"}},
                                  std::vector<parsing::token_type>{
                                      {parsing::prefix_type::none, "4"}},
                                  get_parents<3, 0>(root)},
                       std::tuple{std::vector<parsing::token_type>{
                                      {parsing::prefix_type::none, "--flag1"}},
                                  std::vector<parsing::token_type>{
                                      {parsing::prefix_type::long_, "flag2"}},
                                  std::vector<parsing::token_type>{},
                                  get_parents<4, 0, 0>(root)},
                       std::tuple{std::vector<parsing::token_type>{
                                      {parsing::prefix_type::none, "--flag1"}},
                                  std::vector<parsing::token_type>{
                                      {parsing::prefix_type::long_, "flag3"}},
                                  std::vector<parsing::token_type>{},
                                  get_parents<5, 0, 0>(root)},
                   });
}

BOOST_AUTO_TEST_CASE(pre_parse_phase_too_small_view_test)
{
    const auto root = stub_node{
        policy::long_name<S_("root")>,
        stub_node{policy::long_name<S_("arg1")>,
                  policy::fixed_count<2>,
                  policy::alias(policy::long_name<S_("arg2")>)},
        stub_node{policy::long_name<S_("arg2")>, policy::fixed_count<2>},
        stub_node{policy::long_name<S_("arg3")>},
        policy::router{[](bool, bool, bool) {}}};

    auto result =
        std::vector<parsing::token_type>{{parsing::prefix_type::long_, "arg1"},
                                         {parsing::prefix_type::none, "42"}};
    const auto& owner = std::get<0>(root.children());
    auto args = std::vector<parsing::token_type>{};
    auto adapter = parsing::dynamic_token_adapter{result, args};
    auto processed_tokens = parsing::token_list{};

    const auto match = owner.pre_parse_phase(adapter,
                                             processed_tokens.pending_view(),
                                             owner,
                                             root);

    BOOST_CHECK_EXCEPTION(match.extract(), parse_exception, [](const auto& e) {
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
        auto tokens = parsing::token_list{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 tokens.pending_view(),
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
        auto tokens = parsing::token_list{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 tokens.pending_view(),
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
        auto tokens = parsing::token_list{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 tokens.pending_view(),
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
        auto tokens = parsing::token_list{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 tokens.pending_view(),
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
        auto tokens = parsing::token_list{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 tokens.pending_view(),
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
        auto tokens = parsing::token_list{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 tokens.pending_view(),
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
        auto tokens = parsing::token_list{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 tokens.pending_view(),
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
        auto tokens = parsing::token_list{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 tokens.pending_view(),
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
        auto tokens = parsing::token_list{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 tokens.pending_view(),
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
        auto tokens = parsing::token_list{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 tokens.pending_view(),
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
        auto tokens = parsing::token_list{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 tokens.pending_view(),
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
        auto tokens = parsing::token_list{};
        auto adapter = parsing::dynamic_token_adapter{result, args};
        (void)this->this_policy::pre_parse_phase(adapter,
                                                 tokens.pending_view(),
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
