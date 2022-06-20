/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/utility/tree_recursor.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"

using namespace arg_router;

namespace
{
struct test_Fn {
    template <typename Current, typename... Parents>
    constexpr static void fn()
    {
        using parents_type = std::tuple<Parents...>;

        if constexpr (std::is_same_v<
                          Current,
                          std::decay_t<decltype(policy::validation::default_validator)>>) {
            static_assert(
                std::is_same_v<
                    parents_type,
                    std::tuple<root_t<std::decay_t<decltype(policy::validation::default_validator)>,
                                      flag_t<policy::description_t<S_("test1")>,
                                             policy::long_name_t<S_("test")>,
                                             policy::router<std::function<void(bool)>>>,
                                      flag_t<policy::description_t<S_("test2")>,
                                             policy::short_name_t<traits::integral_constant<'a'>>,
                                             policy::router<std::function<void(bool)>>>>>>);
        } else if constexpr (std::is_same_v<Current, policy::description_t<S_("test1")>>) {
            static_assert(
                std::is_same_v<
                    parents_type,
                    std::tuple<flag_t<policy::description_t<S_("test1")>,
                                      policy::long_name_t<S_("test")>,
                                      policy::router<std::function<void(bool)>>>,
                               root_t<std::decay_t<decltype(policy::validation::default_validator)>,
                                      flag_t<policy::description_t<S_("test1")>,
                                             policy::long_name_t<S_("test")>,
                                             policy::router<std::function<void(bool)>>>,
                                      flag_t<policy::description_t<S_("test2")>,
                                             policy::short_name_t<traits::integral_constant<'a'>>,
                                             policy::router<std::function<void(bool)>>>>>>);
        } else if constexpr (std::is_same_v<Current, policy::long_name_t<S_("test")>>) {
            static_assert(
                std::is_same_v<
                    parents_type,
                    std::tuple<flag_t<policy::description_t<S_("test1")>,
                                      policy::long_name_t<S_("test")>,
                                      policy::router<std::function<void(bool)>>>,
                               root_t<std::decay_t<decltype(policy::validation::default_validator)>,
                                      flag_t<policy::description_t<S_("test1")>,
                                             policy::long_name_t<S_("test")>,
                                             policy::router<std::function<void(bool)>>>,
                                      flag_t<policy::description_t<S_("test2")>,
                                             policy::short_name_t<traits::integral_constant<'a'>>,
                                             policy::router<std::function<void(bool)>>>>>>);
        } else if constexpr (std::is_same_v<Current, policy::description_t<S_("test2")>>) {
            static_assert(
                std::is_same_v<
                    parents_type,
                    std::tuple<flag_t<policy::description_t<S_("test2")>,
                                      policy::short_name_t<traits::integral_constant<'a'>>,
                                      policy::router<std::function<void(bool)>>>,
                               root_t<std::decay_t<decltype(policy::validation::default_validator)>,
                                      flag_t<policy::description_t<S_("test1")>,
                                             policy::long_name_t<S_("test")>,
                                             policy::router<std::function<void(bool)>>>,
                                      flag_t<policy::description_t<S_("test2")>,
                                             policy::short_name_t<traits::integral_constant<'a'>>,
                                             policy::router<std::function<void(bool)>>>>>>);
        } else if constexpr (std::is_same_v<Current,
                                            policy::short_name_t<traits::integral_constant<'a'>>>) {
            static_assert(
                std::is_same_v<
                    parents_type,
                    std::tuple<flag_t<policy::description_t<S_("test2")>,
                                      policy::short_name_t<traits::integral_constant<'a'>>,
                                      policy::router<std::function<void(bool)>>>,
                               root_t<std::decay_t<decltype(policy::validation::default_validator)>,
                                      flag_t<policy::description_t<S_("test1")>,
                                             policy::long_name_t<S_("test")>,
                                             policy::router<std::function<void(bool)>>>,
                                      flag_t<policy::description_t<S_("test2")>,
                                             policy::short_name_t<traits::integral_constant<'a'>>,
                                             policy::router<std::function<void(bool)>>>>>>);
        } else if constexpr (std::is_same_v<Current,
                                            flag_t<policy::description_t<S_("test1")>,
                                                   policy::long_name_t<S_("test")>,
                                                   policy::router<std::function<void(bool)>>>>) {
            static_assert(
                std::is_same_v<
                    parents_type,
                    std::tuple<root_t<std::decay_t<decltype(policy::validation::default_validator)>,
                                      flag_t<policy::description_t<S_("test1")>,
                                             policy::long_name_t<S_("test")>,
                                             policy::router<std::function<void(bool)>>>,
                                      flag_t<policy::description_t<S_("test2")>,
                                             policy::short_name_t<traits::integral_constant<'a'>>,
                                             policy::router<std::function<void(bool)>>>>>>);
        } else if constexpr (std::is_same_v<
                                 Current,
                                 flag_t<policy::description_t<S_("test2")>,
                                        policy::short_name_t<traits::integral_constant<'a'>>,
                                        policy::router<std::function<void(bool)>>>>) {
            static_assert(
                std::is_same_v<
                    parents_type,
                    std::tuple<root_t<std::decay_t<decltype(policy::validation::default_validator)>,
                                      flag_t<policy::description_t<S_("test1")>,
                                             policy::long_name_t<S_("test")>,
                                             policy::router<std::function<void(bool)>>>,
                                      flag_t<policy::description_t<S_("test2")>,
                                             policy::short_name_t<traits::integral_constant<'a'>>,
                                             policy::router<std::function<void(bool)>>>>>>);
        } else if constexpr (std::is_same_v<
                                 Current,
                                 root_t<
                                     std::decay_t<decltype(policy::validation::default_validator)>,
                                     flag_t<policy::description_t<S_("test1")>,
                                            policy::long_name_t<S_("test")>,
                                            policy::router<std::function<void(bool)>>>,
                                     flag_t<policy::description_t<S_("test2")>,
                                            policy::short_name_t<traits::integral_constant<'a'>>,
                                            policy::router<std::function<void(bool)>>>>>) {
            static_assert(std::is_same_v<parents_type, std::tuple<>>);
        }
    }
};

struct skip_test_fn {
    template <typename Current, typename... Parents>
    constexpr static void fn()
    {
        static_assert(!std::is_same_v<Current, policy::description_t<S_("test2")>>, "Fail");
    }
};

struct skip_Fn {
    template <typename Current, typename...>
    constexpr static bool fn()
    {
        return traits::is_specialisation_of_v<Current, arg_router::mode_t>;
    }
};

template <typename Current, typename... Parents>
struct tree_type_visitor {
    using type = std::tuple<Current, Parents...>;
};
}  // namespace

BOOST_AUTO_TEST_SUITE(utility_suite)

BOOST_AUTO_TEST_CASE(tree_recursor_test)
{
    using Root = root_t<std::decay_t<decltype(policy::validation::default_validator)>,
                        flag_t<policy::description_t<S_("test1")>,
                               policy::long_name_t<S_("test")>,
                               policy::router<std::function<void(bool)>>>,
                        flag_t<policy::description_t<S_("test2")>,
                               policy::short_name_t<traits::integral_constant<'a'>>,
                               policy::router<std::function<void(bool)>>>>;

    utility::tree_type_recursor<test_Fn, Root>();
}

BOOST_AUTO_TEST_CASE(tree_recursor_skip_test)
{
    using Root = root_t<std::decay_t<decltype(policy::validation::default_validator)>,
                        flag_t<policy::description_t<S_("test1")>,
                               policy::long_name_t<S_("test")>,
                               policy::router<std::function<void(bool)>>>,
                        arg_router::mode_t<  //
                            flag_t<policy::description_t<S_("test2")>,
                                   policy::short_name_t<traits::integral_constant<'a'>>>,
                            policy::router<std::function<void(bool)>>>>;

    utility::tree_type_recursor<skip_test_fn, skip_Fn, Root>();
}

BOOST_AUTO_TEST_CASE(tree_recursor_instance_test)
{
    const auto r = root(
        mode(flag(policy::long_name<S_("hello")>, policy::description<S_("Hello description")>),
             arg<int>(policy::long_name<S_("arg")>,
                      policy::required,
                      policy::description<S_("Arg description")>),
             policy::router{[&](auto, auto) {}}),
        policy::validation::default_validator);

    auto hit = std::bitset<4>{};
    auto hit_index = 0u;
    const auto visitor = [&](const auto& current, const auto&... parents) {
        using current_type = std::decay_t<decltype(current)>;

        constexpr auto matcher = [](const auto& node) {
            return std::is_same_v<current_type, std::decay_t<decltype(node)>>;
        };

        const auto address_checker = [&](auto expected_nodes) {
            const auto parents_tuple = std::tuple{std::cref(current), std::cref(parents)...};

            static_assert(std::tuple_size_v<std::decay_t<decltype(expected_nodes)>> ==
                              (sizeof...(parents) + 1),
                          "Parents tuple size mismatch");
            utility::tuple_iterator(
                [&](auto i, auto expected_node) {
                    using expected_node_type = typename std::decay_t<decltype(expected_node)>::type;
                    using parent_type =
                        typename std::tuple_element_t<i,
                                                      std::decay_t<decltype(parents_tuple)>>::type;
                    static_assert(std::is_same_v<expected_node_type, parent_type>,
                                  "Parent type mismatch");

                    BOOST_CHECK_EQUAL(
                        reinterpret_cast<std::ptrdiff_t>(std::addressof(expected_node.get())),
                        reinterpret_cast<std::ptrdiff_t>(
                            std::addressof(std::get<i>(parents_tuple).get())));
                },
                expected_nodes);
        };

        if constexpr (matcher(r)) {
            address_checker(std::tuple{std::cref(r)});
        } else if constexpr (matcher(test::get_node<0>(r))) {
            address_checker(test::get_parents<0>(r));
        } else if constexpr (matcher(test::get_node<0, 0>(r))) {
            address_checker(test::get_parents<0, 0>(r));
        } else if constexpr (matcher(test::get_node<0, 1>(r))) {
            address_checker(test::get_parents<0, 1>(r));
        }

        BOOST_CHECK(!hit[hit_index]);
        hit.set(hit_index++);
    };
    utility::tree_recursor(visitor, r);

    BOOST_CHECK(hit.all());
}

BOOST_AUTO_TEST_CASE(tree_type_recursor_collector_test)
{
    using Root = root_t<std::decay_t<decltype(policy::validation::default_validator)>,
                        flag_t<policy::description_t<S_("test1")>,
                               policy::long_name_t<S_("test")>,
                               policy::router<std::function<void(bool)>>>,
                        arg_router::mode_t<  //
                            flag_t<policy::description_t<S_("test2")>,
                                   policy::short_name_t<traits::integral_constant<'a'>>>,
                            policy::router<std::function<void(bool)>>>>;

    using result_type = utility::tree_type_recursor_collector_t<tree_type_visitor, Root>;

    static_assert(std::tuple_size_v<result_type> == 16, "Test failed");
    static_assert(std::is_same_v<std::tuple_element_t<0, result_type>,
                                 std::tuple<arg_router::policy::default_value<bool>,
                                            flag_t<policy::description_t<S_("test1")>,
                                                   policy::long_name_t<S_("test")>,
                                                   policy::router<std::function<void(bool)>>>,
                                            Root>>,
                  "Test failed");
    static_assert(std::is_same_v<std::tuple_element_t<5, result_type>,
                                 std::tuple<flag_t<policy::description_t<S_("test1")>,
                                                   policy::long_name_t<S_("test")>,
                                                   policy::router<std::function<void(bool)>>>,
                                            Root>>,
                  "Test failed");
    static_assert(
        std::is_same_v<std::tuple_element_t<9, result_type>,
                       std::tuple<policy::short_name_t<traits::integral_constant<'a'>>,
                                  flag_t<policy::description_t<S_("test2")>,
                                         policy::short_name_t<traits::integral_constant<'a'>>>,
                                  arg_router::mode_t<
                                      flag_t<policy::description_t<S_("test2")>,
                                             policy::short_name_t<traits::integral_constant<'a'>>>,
                                      policy::router<std::function<void(bool)>>>,
                                  Root>>,
        "Test failed");
    static_assert(
        std::is_same_v<std::tuple_element_t<12, result_type>,
                       std::tuple<policy::router<std::function<void(bool)>>,
                                  arg_router::mode_t<  //
                                      flag_t<policy::description_t<S_("test2")>,
                                             policy::short_name_t<traits::integral_constant<'a'>>>,
                                      policy::router<std::function<void(bool)>>>,
                                  Root>>,
        "Test failed");
    static_assert(std::is_same_v<                             //
                      std::tuple_element_t<15, result_type>,  //
                      std::tuple<Root>>,
                  "Test failed");
}

BOOST_AUTO_TEST_SUITE_END()
