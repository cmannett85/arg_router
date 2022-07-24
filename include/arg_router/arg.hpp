/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/tree_node.hpp"

namespace arg_router
{
/** Represents an argument on the command line that has a value that needs parsing.
 *
 * @tparam T Argument value type
 * @tparam Policies Pack of policies that define its behaviour
 */
template <typename T, typename... Policies>
class arg_t :
    public tree_node<std::decay_t<decltype(policy::fixed_count<1>)>, std::decay_t<Policies>...>
{
    static_assert(policy::is_all_policies_v<std::tuple<std::decay_t<Policies>...>>,
                  "Args must only contain policies (not other nodes)");

    using parent_type =
        tree_node<std::decay_t<decltype(policy::fixed_count<1>)>, std::decay_t<Policies>...>;

    static_assert(traits::has_long_name_method_v<arg_t> || traits::has_short_name_method_v<arg_t>,
                  "Arg must have a long and/or short name policy");
    static_assert(!traits::has_display_name_method_v<arg_t>,
                  "Arg must not have a display name policy");
    static_assert(!traits::has_none_name_method_v<arg_t>, "Arg must not have a none name policy");

public:
    using typename parent_type::policies_type;

    /** Argument value type. */
    using value_type = T;

    /** Help data type. */
    template <bool Flatten>
    using help_data_type = typename parent_type::template default_leaf_help_data_type<Flatten>;

    /** Constructor.
     *
     * @param policies Policy instances
     */
    constexpr explicit arg_t(Policies... policies) noexcept :
        parent_type{policy::fixed_count<1>, std::move(policies)...}
    {
    }

    template <typename Validator, bool HasTarget, typename... Parents>
    [[nodiscard]] std::optional<parsing::parse_target> pre_parse(
        parsing::pre_parse_data<Validator, HasTarget> pre_parse_data,
        const Parents&... parents) const

    {
        return parent_type::pre_parse(pre_parse_data, *this, parents...);
    }

    /** Parse function.
     *
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param target Parse target
     * @param parents Parents instances pack
     * @return Parsed result
     * @exception parse_exception Thrown if parsing failed
     */
    template <typename... Parents>
    value_type parse(parsing::parse_target&& target, const Parents&... parents) const
    {
        // Parse the value token
        auto result = parent_type::template parse<value_type>(target.tokens().front().name,
                                                              *this,
                                                              parents...);

        // Validation
        utility::tuple_type_iterator<policies_type>([&](auto i) {
            using policy_type = std::tuple_element_t<i, policies_type>;
            if constexpr (policy::has_validation_phase_method_v<policy_type, value_type>) {
                this->policy_type::validation_phase(result, *this, parents...);
            }
        });

        // Routing
        using routing_policy =
            typename parent_type::template phase_finder_t<policy::has_routing_phase_method>;
        if constexpr (!std::is_void_v<routing_policy>) {
            this->routing_policy::routing_phase(std::move(result));
        }

        return result;
    }
};

/** Constructs an arg_t with the given policies and value type.
 *
 * This is necessary due to CTAD being required for all template parameters or none, and
 * unfortunately in our case we need @a T to be explicitly set by the user whilst @a Policies should
 * be deduced.
 * @tparam T Argument value type
 * @tparam Policies Pack of policies that define its behaviour
 * @param policies Pack of policy instances
 * @return Argument instance
 */
template <typename T, typename... Policies>
[[nodiscard]] constexpr auto arg(Policies... policies) noexcept
{
    return arg_t<T, std::decay_t<Policies>...>{std::move(policies)...};
}
}  // namespace arg_router
