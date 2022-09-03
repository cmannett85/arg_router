/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/policy/display_name.hpp"
#include "arg_router/policy/no_result_value.hpp"
#include "arg_router/tree_node.hpp"

#include <variant>

namespace arg_router::dependency::detail
{
// Collect the child names and concetenate into a compile time string for the one_of display name
template <typename ParentDocName, typename... Params>
struct generate_string_of_child_names {
    template <typename Prefix, typename String>
    using prepend_prefix =
        boost::mp11::mp_flatten<boost::mp11::mp_insert_c<String, 0, typename Prefix::array_type>>;

    template <typename Child>
    struct build_name {
        // This is clunky, but it's still more readable than doing it with bare types
        constexpr static auto build()
        {
            using policies = typename Child::policies_type;
            constexpr auto num_policies = std::tuple_size_v<policies>;

            // Find display name
            constexpr auto display_index =
                boost::mp11::mp_find_if<policies, traits::has_display_name_method>::value;
            if constexpr (display_index >= num_policies) {
                // Find long name
                constexpr auto long_index =
                    boost::mp11::mp_find_if<policies, traits::has_long_name_method>::value;
                if constexpr (long_index >= num_policies) {
                    // Find short name
                    constexpr auto short_index =
                        boost::mp11::mp_find_if<policies, traits::has_short_name_method>::value;

                    static_assert(short_index < num_policies, "All children must be named");

                    return S_(config::short_prefix){} +
                           S_((std::tuple_element_t<short_index, policies>::short_name())){};
                } else {
                    return S_(config::long_prefix){} +
                           S_((std::tuple_element_t<long_index, policies>::long_name())){};
                }
            } else {
                return S_((std::tuple_element_t<display_index, policies>::display_name())){};
            }
        }

        using type = typename std::decay_t<decltype(build())>::array_type;
    };

    template <typename ChildNameArray>
    using joiner = boost::mp11::mp_push_back<ChildNameArray, traits::integral_constant<','>>;

    using children_type = boost::mp11::mp_filter<is_tree_node, std::tuple<Params...>>;

    // The gist is that for each child we get its name (prepended with an appropriate prefix) and
    // concatenate with a comma separator.  And then add a helpful prefix to the whole thing
    using string_array = prepend_prefix<
        ParentDocName,
        boost::mp11::mp_transform<
            joiner,
            boost::mp11::mp_transform_q<
                boost::mp11::mp_bind<traits::get_type,
                                     boost::mp11::mp_bind<build_name, boost::mp11::_1>>,
                children_type>>>;

    using type = utility::convert_to_cts_t<
        boost::mp11::mp_take_c<string_array, std::tuple_size_v<string_array> - 1>>;
};

template <typename ParentDocName, typename... Params>
class basic_one_of_t :
    public tree_node<
        policy::display_name_t<
            typename generate_string_of_child_names<ParentDocName, std::decay_t<Params>...>::type>,
        std::decay_t<Params>...>
{
    // C++ really needs a super_type...
    using parent_type = tree_node<
        policy::display_name_t<
            typename generate_string_of_child_names<ParentDocName, std::decay_t<Params>...>::type>,
        std::decay_t<Params>...>;

    static_assert((std::tuple_size_v<typename parent_type::children_type> >= 2),
                  "basic_one_of_t must have at least one two child nodes");
    static_assert(!traits::has_long_name_method_v<basic_one_of_t>,
                  "basic_one_of_t must not have a long name policy");
    static_assert(!traits::has_short_name_method_v<basic_one_of_t>,
                  "basic_one_of_t must not have a short name policy");
    static_assert(!traits::has_none_name_method_v<basic_one_of_t>,
                  "basic_one_of_t must not have a none name policy");

protected:
    using typename parent_type::children_type;
    using typename parent_type::policies_type;

    static_assert(
        boost::mp11::mp_any_of_q<
            policies_type,
            boost::mp11::mp_bind<policy::has_missing_phase_method, boost::mp11::_1, bool>>::value,
        "basic_one_of_t must have a missing phase method, a "
        "policy::required or policy::default_value are commonly used");

    static_assert(!parent_type::template any_phases_v<bool,
                                                      policy::has_pre_parse_phase_method,
                                                      policy::has_parse_phase_method,
                                                      policy::has_routing_phase_method>,
                  "basic_one_of_t does not support policies with pre-parse, parse, "
                  "or routing phases; as it delegates those to its children");

    using basic_value_type =
        boost::mp11::mp_transform<traits::get_value_type,
                                  boost::mp11::mp_remove_if<typename parent_type::children_type,
                                                            policy::has_no_result_value>>;

    static_assert((std::tuple_size_v<basic_value_type> >= 1),
                  "basic_one_of_t must have at least one child with a value_type");

    template <bool Flatten>
    class children_help_data_type
    {
        template <typename Child, typename Prefix>
        struct prefixer {
            using label = typename Prefix::template append_t<
                typename Child::template help_data_type<Flatten>::label>;
            using description = typename Child::template help_data_type<Flatten>::description;
            using children = std::tuple<>;
        };

        template <typename Child>
        using first_prefixer = prefixer<Child, S_("┌ ")>;

        template <typename Child>
        using middle_prefixer = prefixer<Child, S_("├ ")>;

        template <typename Child>
        using last_prefixer = prefixer<Child, S_("└ ")>;

    public:
        using children = boost::mp11::mp_replace_at_c<
            boost::mp11::mp_replace_first<boost::mp11::mp_transform<middle_prefixer, children_type>,
                                          first_prefixer<boost::mp11::mp_first<children_type>>>,
            std::tuple_size_v<children_type> - 1,
            last_prefixer<boost::mp11::mp_back<children_type>>>;
    };

    constexpr explicit basic_one_of_t(Params... params) noexcept :
        parent_type{std::tuple_element_t<0, policies_type>{}, std::move(params)...}
    {
    }
};
}  // namespace arg_router::dependency::detail
