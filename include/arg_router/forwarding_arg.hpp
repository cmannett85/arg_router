// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/multi_arg_base.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/utility/string_to_policy.hpp"

namespace arg_router
{
/** Represents a none-named argument on the command line that has multiple values that needs
 * parsing, specifically for forwarding tokens verbatim.
 *
 * A policy::token_end_marker_t can be used to mark the end of a variable length value token list
 * on the command line.
 *
 * Cannot only have a none name policy.  The value_type is fixed as
 * <TT>vector\<std::string_view\></TT>.
 * @tparam Policies Pack of policies that define its behaviour
 */
template <typename... Policies>
class forwarding_arg_t :
    public multi_arg_base_t<vector<std::string_view>, 0, std::decay_t<Policies>...>
{
    using parent_type = multi_arg_base_t<vector<std::string_view>, 0, std::decay_t<Policies>...>;

    static_assert(!traits::has_display_name_method_v<forwarding_arg_t> &&
                      !traits::has_long_name_method_v<forwarding_arg_t> &&
                      !traits::has_short_name_method_v<forwarding_arg_t>,
                  "Forwarding arg can only have a none name policy");

public:
    using typename parent_type::policies_type;

    /** Argument value type. */
    using value_type = typename parent_type::value_type;

    /** Help data type. */
    template <bool Flatten>
    class help_data_type
    {
    public:
        using label = std::decay_t<decltype(
            parent_type::template default_leaf_help_data_type<Flatten>::label_generator() +
            AR_STRING(" "){} +
            parent_type::template default_leaf_help_data_type<Flatten>::count_suffix())>;
        using description =
            typename parent_type::template default_leaf_help_data_type<Flatten>::description;
        using children = std::tuple<>;
    };

    /** Constructor.
     *
     * @param policies Policy instances
     */
    constexpr explicit forwarding_arg_t(Policies... policies) noexcept :
        parent_type{std::move(policies)...}
    {
    }

    template <typename Validator, bool HasTarget, typename... Parents>
    [[nodiscard]] std::optional<parsing::parse_target> pre_parse(
        parsing::pre_parse_data<Validator, HasTarget> pre_parse_data,
        const Parents&... parents) const

    {
        return parent_type::pre_parse(pre_parse_data, *this, parents...);
    }

    template <typename... Parents>
    value_type parse(parsing::parse_target&& target, const Parents&... parents) const
    {
        return parent_type::parse(target, *this, parents...);
    }

private:
    static_assert(!parent_type::template any_phases_v<value_type, policy::has_routing_phase_method>,
                  "Forwarding arg does not support policies with routing phases "
                  "(e.g. router)");
};

/** Constructs a forwarding_arg_t with the given policies and value type.
 *
 * Compile-time strings can be passed in directly and will be converted to the appropriate policies
 * automatically.  The rules are:
 * -# The first multi-character string becomes a policy::none_name_t
 * -# The second multi-character string becomes a policy::description_t
 *
 * The above are unicode aware.  The strings can be passed in any order relative to the other
 * policies, but it is recommended to put them first to ease reading.
 *
 * @tparam Policies Pack of policies that define its behaviour
 * @param policies Pack of policy instances
 * @return Argument instance
 */
template <typename... Policies>
[[nodiscard]] constexpr auto forwarding_arg(Policies... policies) noexcept
{
    return std::apply(
        [](auto... converted_policies) {
            return forwarding_arg_t<std::decay_t<decltype(converted_policies)>...>{
                std::move(converted_policies)...};
        },
        utility::string_to_policy::convert<
            utility::string_to_policy::first_string_mapper<policy::none_name_t>,
            utility::string_to_policy::second_string_mapper<policy::description_t>>(
            std::move(policies)...));
}
}  // namespace arg_router
