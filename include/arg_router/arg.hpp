// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/multi_arg_base.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/string_to_policy.hpp"

namespace arg_router
{
/** Represents a named argument on the command line that has a value that needs parsing.
 *
 * Cannot have a none name or display name policy.
 * @tparam T Argument value type
 * @tparam Policies Pack of policies that define its behaviour
 */
template <typename T, typename... Policies>
class arg_t :
    public multi_arg_base_t<T,
                            1,
                            policy::min_max_count_t<traits::integral_constant<std::size_t{1}>,
                                                    traits::integral_constant<std::size_t{1}>>,
                            std::decay_t<Policies>...>
{
    using parent_type =
        multi_arg_base_t<T,
                         1,
                         policy::min_max_count_t<traits::integral_constant<std::size_t{1}>,
                                                 traits::integral_constant<std::size_t{1}>>,
                         std::decay_t<Policies>...>;

    static_assert(!traits::has_none_name_method_v<arg_t>, "Arg must not have a none name policy");
    static_assert(!traits::has_display_name_method_v<arg_t>,
                  "Arg must not have a display name policy");

public:
    using typename parent_type::policies_type;

    /** Argument value type. */
    using value_type = typename parent_type::value_type;

    /** Help data type. */
    template <bool Flatten>
    using help_data_type = typename parent_type::template default_leaf_help_data_type<Flatten>;

    /** Constructor.
     *
     * @param policies Policy instances
     */
    constexpr explicit arg_t(Policies... policies) noexcept :
        parent_type{policy::min_max_count_t<traits::integral_constant<std::size_t{1}>,
                                            traits::integral_constant<std::size_t{1}>>{},
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

    template <typename... Parents>
    value_type parse(parsing::parse_target&& target, const Parents&... parents) const
    {
        return parent_type::parse(target, *this, parents...);
    }
};

/** Constructs an arg_t with the given policies and value type.
 *
 * This is necessary due to CTAD being required for all template parameters or none, and
 * unfortunately in our case we need @a T to be explicitly set by the user whilst @a Policies should
 * be deduced.
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
 * @tparam T Argument value type
 * @tparam Policies Pack of policies that define its behaviour
 * @param policies Pack of policy instances
 * @return Argument instance
 */
template <typename T, typename... Policies>
[[nodiscard]] constexpr auto arg(Policies... policies) noexcept
{
    return std::apply(
        [](auto... converted_policies) {
            return arg_t<T, std::decay_t<decltype(converted_policies)>...>{
                std::move(converted_policies)...};
        },
        utility::string_to_policy::convert<
            utility::string_to_policy::first_string_mapper<policy::long_name_t>,
            utility::string_to_policy::second_string_mapper<policy::description_t>,
            utility::string_to_policy::single_char_mapper<policy::short_name_t>>(
            std::move(policies)...));
}
}  // namespace arg_router
