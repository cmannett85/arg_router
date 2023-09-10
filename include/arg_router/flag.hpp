// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/policy/default_value.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/policy/short_form_expander.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/string_to_policy.hpp"

namespace arg_router
{
/** Represents a flag in the command line.
 *
 * A flag is a boolean indicator, it has no value assigned on the command line, its presence
 * represents a positive boolean value.  It has a default value of false and a fixed count of 0.
 * By default this type does not do short-form name collapsing, add policy::short_form_expander to
 * the policies during construction to enable that.
 *
 * Create with the flag(Policies...) function for consistency with arg_t.
 * @tparam Policies Pack of policies that define its behaviour
 */
template <typename... Policies>
class flag_t :
    public tree_node<policy::default_value<bool>,
                     policy::min_max_count_t<traits::integral_constant<std::size_t{0}>,
                                             traits::integral_constant<std::size_t{0}>>,
                     std::decay_t<Policies>...>
{
    static_assert(policy::is_all_policies_v<std::tuple<std::decay_t<Policies>...>>,
                  "Flags must only contain policies (not other nodes)");

    using parent_type =
        tree_node<policy::default_value<bool>,
                  policy::min_max_count_t<traits::integral_constant<std::size_t{0}>,
                                          traits::integral_constant<std::size_t{0}>>,
                  std::decay_t<Policies>...>;

    static_assert(traits::has_long_name_method_v<flag_t> || traits::has_short_name_method_v<flag_t>,
                  "Flag must have a long and/or short name policy");
    static_assert(!traits::has_display_name_method_v<flag_t>,
                  "Flag must not have a display name policy");
    static_assert(!traits::has_none_name_method_v<flag_t>, "Flag must not have a none name policy");

public:
    using typename parent_type::policies_type;

    /** Flag value type. */
    using value_type = bool;

    /** Constructor.
     *
     * @param policies Policy instances
     */
    constexpr explicit flag_t(Policies... policies) noexcept :
        parent_type{policy::default_value<bool>{false},
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
     * @exception multi_lang_exception Thrown if routing phase failed
     */
    template <typename... Parents>
    value_type parse([[maybe_unused]] parsing::parse_target&& target,
                     [[maybe_unused]] const Parents&... parents) const
    {
        // Presence of the flag yields a constant true
        const auto result = true;

        // Routing phase
        using routing_policy =
            typename parent_type::template phase_finder_t<policy::has_routing_phase_method>;
        if constexpr (!std::is_void_v<routing_policy>) {
            this->routing_policy::routing_phase(result);
        }

        return result;
    }

private:
    static_assert(!parent_type::template any_phases_v<value_type,
                                                      policy::has_parse_phase_method,
                                                      policy::has_validation_phase_method>,
                  "Flag does not support policies with parse or validation phases "
                  "(e.g. custom_parser or min_max_value)");
};

/** Constructs a flag_t with the given policies.
 *
 * Flags with shortnames can be concatenated or 'collapsed' on the command line, e.g.
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
template <typename... Policies>
[[nodiscard]] constexpr auto flag(Policies... policies) noexcept
{
    return utility::apply(
        [](auto... converted_policies) {
            // Add the short-form expander if one of the policies implements the short name method,
            // and if the short and long prefix are not the same
            constexpr auto has_short_name =
                boost::mp11::mp_any_of<std::tuple<std::decay_t<decltype(converted_policies)>...>,
                                       traits::has_short_name_method>::value;
            if constexpr ((config::long_prefix != config::short_prefix) && has_short_name) {
                return flag_t{policy::short_form_expander, std::move(converted_policies)...};
            } else {
                return flag_t{std::move(converted_policies)...};
            }
        },
        utility::string_to_policy::convert<
            utility::string_to_policy::first_string_mapper<policy::long_name_t>,
            utility::string_to_policy::second_string_mapper<policy::description_t>,
            utility::string_to_policy::single_char_mapper<policy::short_name_t>>(
            std::move(policies)...));
}
}  // namespace arg_router
