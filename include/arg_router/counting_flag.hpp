// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/policy/multi_stage_value.hpp"
#include "arg_router/policy/short_form_expander.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/string_to_policy.hpp"

namespace arg_router
{
/** Represents a flag that can appear multiple times in the command line.
 *
 * A flag is a boolean indicator, it has no value assigned on the command line, its presence
 * represents a positive boolean value.  However a counting flag's value is the number of times it
 * appears on the command line.  By default this type does not do short-form name collapsing, add
 * policy::short_form_expander to the policies during construction to enable that.
 *
 * Create with the counting_flag(Policies...) function for consistency with arg_t.
 * @tparam T Counting value type, must be explicitly convertible to std::size_t
 * @tparam Policies Pack of policies that define its behaviour
 */
template <typename T, typename... Policies>
class counting_flag_t :
    public tree_node<policy::multi_stage_value<T, bool>,
                     policy::min_max_count_t<traits::integral_constant<std::size_t{0}>,
                                             traits::integral_constant<std::size_t{0}>>,
                     std::decay_t<Policies>...>
{
    static_assert(policy::is_all_policies_v<std::tuple<std::decay_t<Policies>...>>,
                  "Counting flags must only contain policies (not other nodes)");
    static_assert(traits::has_long_name_method_v<counting_flag_t> ||
                      traits::has_short_name_method_v<counting_flag_t>,
                  "Counting flag must have a long and/or short name policy");
    static_assert(!traits::has_display_name_method_v<counting_flag_t>,
                  "Counting flag must not have a display name policy");
    static_assert(!traits::has_none_name_method_v<counting_flag_t>,
                  "Counting flag must not have a none name policy");
    static_assert(traits::supports_static_cast_conversion_v<T, std::size_t>,
                  "T must be explicitly convertible to std::size_t");

    using parent_type =
        tree_node<policy::multi_stage_value<T, bool>,
                  policy::min_max_count_t<traits::integral_constant<std::size_t{0}>,
                                          traits::integral_constant<std::size_t{0}>>,
                  std::decay_t<Policies>...>;

public:
    using typename parent_type::policies_type;

    /** Flag value type. */
    using value_type = T;

    /** Help data type. */
    template <bool Flatten>
    using help_data_type = typename parent_type::template default_leaf_help_data_type<Flatten>;

    /** Constructor.
     *
     * @param policies Policy instances
     */
    constexpr explicit counting_flag_t(Policies... policies) noexcept :
        parent_type{policy::multi_stage_value<T, bool>{merge_impl},
                    policy::min_max_count_t<traits::integral_constant<std::size_t{0}>,
                                            traits::integral_constant<std::size_t{0}>>{},
                    std::move(policies)...}
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
     */
    template <typename... Parents>
    bool parse([[maybe_unused]] parsing::parse_target&& target,
               [[maybe_unused]] const Parents&... parents) const noexcept
    {
        // Presence of the flag yields a constant true.  Validation is done by the parent mode as it
        // carries the final result
        return true;
    }

private:
    static_assert(!parent_type::template any_phases_v<value_type,
                                                      policy::has_parse_phase_method,
                                                      policy::has_routing_phase_method>,
                  "Counting flag does not support policies with parse or routing phases "
                  "(e.g. custom_parser)");

    // Value will always be true
    constexpr static void merge_impl(std::optional<value_type>& result, bool /*value*/) noexcept
    {
        if (!result) {
            result = static_cast<value_type>(1);
            return;
        }

        *result = static_cast<value_type>(static_cast<std::size_t>(*result) + 1);
    }
};

/** Constructs a counting_flag_t with the given policies.
 *
 * Just like 'normal' flags, counting flags with shortnames can be concatenated or 'collapsed' on
 * the command line,
 * e.g.
 * @code
 * foo -a -b -c
 * foo -abc
 * @endcode
 *
 * Compile-time strings can be passed in directly and will be converted to the appropriate policies
 * automatically.  The rules are:
 * -# The first multi-character string becomes a policy::long_name_t
 * -# The second multi-character string becomes a policy::description_t
 * -# The first single-charcter string becomes a policy::short_name_t
 *
 * The above are unicode aware.  The strings can be passed in any order relative to the other
 * policies, but it is recommended to put them first to ease reading.
 *
 * @tparam Policies Pack of policies that define its behaviour
 * @param policies Pack of policy instances
 * @return Flag instance
 */
template <typename T, typename... Policies>
[[nodiscard]] constexpr auto counting_flag(Policies... policies) noexcept
{
    return std::apply(
        [](auto... converted_policies) {
            // Add the short-form expander if one of the policies implements the short name method,
            // and if the short and long prefix are not the same
            constexpr auto has_short_name =
                boost::mp11::mp_any_of<std::tuple<std::decay_t<decltype(converted_policies)>...>,
                                       traits::has_short_name_method>::value;
            if constexpr ((config::long_prefix != config::short_prefix) && has_short_name) {
                return counting_flag_t<T,
                                       policy::short_form_expander_t<>,
                                       std::decay_t<decltype(converted_policies)>...>{
                    policy::short_form_expander,
                    std::move(converted_policies)...};
            } else {
                return counting_flag_t<T, std::decay_t<decltype(converted_policies)>...>{
                    std::move(converted_policies)...};
            }
        },
        utility::string_to_policy::convert<
            utility::string_to_policy::first_string_mapper<policy::long_name_t>,
            utility::string_to_policy::second_string_mapper<policy::description_t>,
            utility::string_to_policy::single_char_mapper<policy::short_name_t>>(
            std::move(policies)...));
}
}  // namespace arg_router
