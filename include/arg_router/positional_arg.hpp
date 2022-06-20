/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/tree_node.hpp"

namespace arg_router
{
namespace detail
{
template <typename... Policies>
class add_missing_min_max_policy
{
    template <typename Policy>
    struct has_min_max_t {
        constexpr static bool value =
            traits::has_minimum_count_method_v<Policy> &&
            traits::has_maximum_count_method_v<Policy>;
    };

    using policies_tuple = std::tuple<std::decay_t<Policies>...>;
    using unbounded_policy_type = std::decay_t<decltype(policy::min_count<0>)>;

public:
    constexpr static auto has_min_max =
        boost::mp11::mp_find_if<policies_tuple, has_min_max_t>::value !=
        std::tuple_size_v<policies_tuple>;

    using type = std::conditional_t<
        has_min_max,
        boost::mp11::mp_rename<policies_tuple, tree_node>,
        boost::mp11::mp_rename<
            boost::mp11::mp_push_front<policies_tuple, unbounded_policy_type>,
            tree_node>>;
};
}  // namespace detail

/** Represents a positional argument on the command line that has potentially
 * multiple values that need parsing.
 *
 * If no policy implementing <TT>minimum_count()</TT> and
 * <TT>maximum_count()</TT> methods is used (e.g. policy::min_max_count_t), then
 * an unbounded policy::min_max_count_t is prepended to the policies internally.
 * @tparam T Argument value type, must have a <TT>push_back(..)</TT> method
 * @tparam Policies Pack of policies that define its behaviour
 */
template <typename T, typename... Policies>
class positional_arg_t :
    public detail::add_missing_min_max_policy<Policies...>::type
{
    static_assert(
        policy::is_all_policies_v<std::tuple<std::decay_t<Policies>...>>,
        "Positional args must only contain policies (not other nodes)");

    using parent_type =
        typename detail::add_missing_min_max_policy<Policies...>::type;

    template <std::size_t N>
    constexpr static bool has_fixed_count = []() {
        return (parent_type::minimum_count() == N) &&
               (parent_type::maximum_count() == N);
    }();

    static_assert(!has_fixed_count<0>, "Cannot have a fixed count of zero");
    static_assert(has_fixed_count<1> || traits::has_push_back_method_v<T>,
                  "value_type must have a push_back() method");

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
                boost::mp11::mp_find_if<policies_type,
                                        traits::has_display_name_method>::value;
            constexpr auto name =
                std::tuple_element_t<name_index, policies_type>::display_name();

            return S_("<"){} + S_(name){} + S_("> "){} +
                   parent_type::template default_leaf_help_data_type<
                       Flatten>::count_suffix();
        }

    public:
        using label = std::decay_t<decltype(label_generator())>;
        using description =
            typename parent_type::template default_leaf_help_data_type<
                Flatten>::description;
        using children = std::tuple<>;
    };

    /** Constructor.
     *
     * @param policies Policy instances
     */
    template <auto has_min_max =
                  detail::add_missing_min_max_policy<Policies...>::has_min_max>
    constexpr explicit positional_arg_t(
        Policies... policies,
        std::enable_if_t<has_min_max>* = nullptr) noexcept :
        parent_type{std::move(policies)...}
    {
    }

    template <auto has_min_max =
                  detail::add_missing_min_max_policy<Policies...>::has_min_max>
    constexpr explicit positional_arg_t(
        Policies... policies,
        std::enable_if_t<!has_min_max>* = nullptr) noexcept :
        parent_type{policy::min_count<0>, std::move(policies)...}
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
    value_type parse(parsing::parse_target target,
                     const Parents&... parents) const
    {
        auto result = value_type{};
        if constexpr (traits::has_push_back_method_v<value_type>) {
            for (auto token : target.tokens()) {
                result.push_back(
                    parent_type::template parse<value_type>(token.name,
                                                            *this,
                                                            parents...));
            }
        } else if (!target.tokens().empty()) {
            result = parent_type::template parse<value_type>(
                target.tokens().front().name,
                *this,
                parents...);
        }

        // Validation
        utility::tuple_type_iterator<policies_type>([&](auto i) {
            using policy_type = std::tuple_element_t<i, policies_type>;
            if constexpr (policy::has_validation_phase_method_v<policy_type,
                                                                value_type>) {
                this->policy_type::validation_phase(result, *this, parents...);
            }
        });

        // No routing phase, a positional_arg cannot be used as a top-level node

        return result;
    }

private:
    static_assert(
        !parent_type::template any_phases_v<value_type,
                                            policy::has_routing_phase_method>,
        "Positional arg does not support policies with routing phases "
        "(e.g. router)");
};

/** Constructs an positional_arg_t with the given policies and value type.
 *
 * This is necessary due to CTAD being required for all template parameters or
 * none, and unfortunately in our case we need @a T to be explicitly set by the
 * user whilst @a Policies should be deduced.
 * @tparam T Argument value type, must have a <TT>push_back(..)</TT> method
 * @tparam Policies Pack of policies that define its behaviour
 * @param policies Pack of policy instances
 * @return Argument instance
 */
template <typename T, typename... Policies>
[[nodiscard]] constexpr auto positional_arg(Policies... policies) noexcept
{
    return positional_arg_t<T, std::decay_t<Policies>...>{
        std::move(policies)...};
}
}  // namespace arg_router
