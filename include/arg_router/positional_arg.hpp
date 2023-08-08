// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/multi_arg_base.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/display_name.hpp"
#include "arg_router/utility/string_to_policy.hpp"

namespace arg_router
{
/** Represents a positional argument on the command line that has potentially multiple values that
 * need parsing.
 *
 * If no policy implementing <TT>minimum_count()</TT> and <TT>maximum_count()</TT> methods is used
 * (e.g. policy::min_max_count_t), then an unbounded policy::min_max_count_t is prepended to the
 * policies internally.
 *
 * A policy::token_end_marker_t can be used to mark the end of a variable length value token list
 * on the command line.
 * @tparam T Argument value type, must have a <TT>push_back(..)</TT> method
 * @tparam Policies Pack of policies that define its behaviour
 */
template <typename T, typename... Policies>
class positional_arg_t : public multi_arg_base_t<T, 0, Policies...>
{
    using parent_type = multi_arg_base_t<T, 0, Policies...>;

    static_assert(traits::has_display_name_method_v<positional_arg_t>,
                  "Positional arg must have a display name policy");
    static_assert(!traits::has_long_name_method_v<positional_arg_t>,
                  "Positional arg must not have a long name policy");
    static_assert(!traits::has_short_name_method_v<positional_arg_t>,
                  "Positional arg must not have a short name policy");
    static_assert(!traits::has_none_name_method_v<positional_arg_t>,
                  "Positional arg must not have a none name policy");

public:
    using typename parent_type::policies_type;

    /** Argument value type. */
    using value_type = T;

    /** Help data type. */
    template <bool Flatten>
    class help_data_type
    {
        [[nodiscard]] constexpr static auto label_generator() noexcept
        {
            constexpr auto name_index =
                boost::mp11::mp_find_if<policies_type, traits::has_display_name_method>::value;
            using name_type = typename std::tuple_element_t<name_index, policies_type>::string_type;

            return AR_STRING("<"){} + name_type{} + AR_STRING("> "){} +
                   parent_type::template default_leaf_help_data_type<Flatten>::count_suffix();
        }

    public:
        using label = std::decay_t<decltype(label_generator())>;
        using description =
            typename parent_type::template default_leaf_help_data_type<Flatten>::description;
        using children = std::tuple<>;
    };

    /** Constructor.
     *
     * @param policies Policy instances
     */
    constexpr explicit positional_arg_t(Policies... policies) noexcept :
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
    [[nodiscard]] value_type parse(parsing::parse_target target, const Parents&... parents) const
    {
        return parent_type::parse(target, *this, parents...);
    }

private:
    static_assert(!parent_type::template any_phases_v<value_type, policy::has_routing_phase_method>,
                  "Positional arg does not support policies with routing phases "
                  "(e.g. router)");
};

/** Constructs an positional_arg_t with the given policies and value type.
 *
 * This is necessary due to CTAD being required for all template parameters or none, and
 * unfortunately in our case we need @a T to be explicitly set by the user whilst @a Policies
 * should be deduced.
 *
 * Compile-time strings can be passed in directly and will be converted to the appropriate policies
 * automatically.  The rules are:
 * -# The first string becomes a policy::display_name_t
 * -# The second string becomes a policy::description_t
 *
 * The above are unicode aware.  The strings can be passed in any order relative to the other
 * policies, but it is recommended to put them first to ease reading.
 *
 * @tparam T Argument value type, must have a <TT>push_back(..)</TT> method
 * @tparam Policies Pack of policies that define its behaviour
 * @param policies Pack of policy instances
 * @return Argument instance
 */
template <typename T, typename... Policies>
[[nodiscard]] constexpr auto positional_arg(Policies... policies) noexcept
{
    return std::apply(
        [](auto... converted_policies) {
            return positional_arg_t<T, std::decay_t<decltype(converted_policies)>...>{
                std::move(converted_policies)...};
        },
        utility::string_to_policy::convert<
            utility::string_to_policy::first_text_mapper<policy::display_name_t>,
            utility::string_to_policy::second_text_mapper<policy::description_t>>(
            std::move(policies)...));
}
}  // namespace arg_router
