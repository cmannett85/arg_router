// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/tree_node.hpp"

namespace arg_router
{
/** Base class for nodes that support multiple value tokens.
 *
 * If no policy implementing <TT>minimum_count()</TT> and <TT>maximum_count()</TT> methods is used
 * (e.g. policy::min_max_count_t), then an unbounded policy::min_max_count_t is prepended to the
 * policies internally.
 *
 * This is the base class for arg_t, multi_arg_t, and postional_arg_t.
 * @note Only supports nodes with a minimum of 1 value token (i.e. is not used for flag-like types)
 * @tparam T Argument value type
 * @tparam MinCount Minimum count value to use if one not specified by user
 * @tparam Policies Pack of policies that define its behaviour
 */
template <typename T, std::size_t MinCount, typename... Policies>
class multi_arg_base_t : public add_missing_min_max_policy<MinCount, Policies...>::type
{
    static_assert(policy::is_all_policies_v<std::tuple<std::decay_t<Policies>...>>,
                  "Arg must only contain policies (not other nodes)");

    using parent_type = typename add_missing_min_max_policy<MinCount, Policies...>::type;

    template <std::size_t N>
    constexpr static bool has_fixed_count = []() {
        return (parent_type::minimum_count() == N) && (parent_type::maximum_count() == N);
    }();

    static_assert(!has_fixed_count<0>, "Cannot have a fixed count of zero");
    static_assert(has_fixed_count<1> || traits::has_push_back_method_v<T>,
                  "value_type must have a push_back() method");

    static_assert(traits::has_long_name_method_v<multi_arg_base_t> ||
                      traits::has_short_name_method_v<multi_arg_base_t> ||
                      traits::has_none_name_method_v<multi_arg_base_t> ||
                      traits::has_display_name_method_v<multi_arg_base_t>,
                  "Arg must be named");

public:
    using typename parent_type::policies_type;

    /** Argument value type. */
    using value_type = T;

protected:
    /** Constructor.
     *
     * @param policies Policy instances
     */
    template <auto has_min_max = add_missing_min_max_policy<MinCount, Policies...>::has_min_max>
    constexpr explicit multi_arg_base_t(Policies... policies,
                                        // NOLINTNEXTLINE(*-named-parameter)
                                        std::enable_if_t<has_min_max>* = nullptr) noexcept :
        parent_type{std::move(policies)...}
    {
    }

    template <auto has_min_max = add_missing_min_max_policy<MinCount, Policies...>::has_min_max>
    constexpr explicit multi_arg_base_t(Policies... policies,
                                        // NOLINTNEXTLINE(*-named-parameter)
                                        std::enable_if_t<!has_min_max>* = nullptr) noexcept :
        parent_type{policy::min_max_count_t<
                        traits::integral_constant<MinCount>,
                        traits::integral_constant<std::numeric_limits<std::size_t>::max()>>{},
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
     * @return Parsed result, note that if a routing policy is present then the result is moved into
     * that so this value is the moved-from result
     * @exception multi_lang_exception Thrown if parsing failed
     */
    template <typename... Parents>
    [[nodiscard]] value_type parse(parsing::parse_target target, const Parents&... parents) const
    {
        auto result = value_type{};
        if constexpr (traits::has_push_back_method_v<value_type> &&
                      !traits::is_specialisation_of_v<value_type, std::basic_string>) {
            for (auto token : target.tokens()) {
                result.push_back(
                    parent_type::template parse<value_type>(token.name, *this, parents...));
            }
        } else if (!target.tokens().empty()) {
            result = parent_type::template parse<value_type>(target.tokens().front().name,
                                                             *this,
                                                             parents...);
        }

        // Validation
        std::apply(
            [&](auto&&... ancestors) {
                utility::tuple_type_iterator<policies_type>([&](auto i) {
                    using policy_type = std::tuple_element_t<i, policies_type>;
                    if constexpr (policy::has_validation_phase_method_v<policy_type, value_type>) {
                        this->policy_type::validation_phase(result, ancestors.get()...);
                    }
                });
            },
            parsing::clean_node_ancestry_list(*this, parents...));

        // Routing
        using routing_policy =
            typename parent_type::template phase_finder_t<policy::has_routing_phase_method>;
        if constexpr (!std::is_void_v<routing_policy>) {
            this->routing_policy::routing_phase(std::move(result));
        }

        return result;
    }
};
}  // namespace arg_router
