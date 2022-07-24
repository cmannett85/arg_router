/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/parsing/parse_target.hpp"
#include "arg_router/parsing/parsing.hpp"
#include "arg_router/policy/policy.hpp"
#include "arg_router/traits.hpp"
#include "arg_router/utility/compile_time_optional.hpp"
#include "arg_router/utility/tree_recursor.hpp"

namespace arg_router::policy
{
/** Causes the owning node to be 'dependent' on others, i.e. that other node must also appear on the
 * command line.
 *
 * The depends targets must have long or short names. @tparam DependsPolicies Pack of name policies,
 * referring to nodes under the same mode, to depend on
 */
template <typename... DependsPolicies>
class dependent_t
{
public:
    /** Tuple of policy types. */
    using depends_policies_type = std::tuple<std::decay_t<DependsPolicies>...>;

    /** Constructor.
     *
     * @param policies Policy instances
     */
    constexpr explicit dependent_t([[maybe_unused]] const DependsPolicies&... policies) noexcept {}

    /** Scans the processed tokens to find all the dependent node names.
     *
     * @tparam ProcessedTarget @a processed_target payload type
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param tokens Currently processed tokens
     * @param processed_target Previously processed parse_target of parent node, or empty is there
     * is no non-root parent
     * @param target Pre-parse generated target
     * @param parents Parent node instances
     * @return Either true if successful, or a parse_exception if a dependent
     * argument is missing
     */
    template <typename ProcessedTarget, typename... Parents>
    [[nodiscard]] parsing::pre_parse_result pre_parse_phase(
        [[maybe_unused]] parsing::dynamic_token_adapter& tokens,
        utility::compile_time_optional<ProcessedTarget> processed_target,
        [[maybe_unused]] parsing::parse_target& target,
        [[maybe_unused]] const Parents&... parents) const
    {
        // Find the owning mode
        using mode_type = typename nearest_mode<Parents...>::type;
        static_assert(!std::is_void_v<mode_type>, "Cannot find parent mode");
        static_assert(!processed_target.empty, "processed_target cannot be empty");

        // Find all the depends targets
        using node_targets = typename depends_targets<depends_policies_type, mode_type>::type;
        static_assert(cyclic_dependency_checker<node_targets, mode_type>::value,
                      "Cyclic dependency detected");

        auto result = parsing::pre_parse_result{parsing::pre_parse_action::valid_node};
        utility::tuple_type_iterator<node_targets>([&](auto i) {
            using node_target = std::tuple_element_t<i, node_targets>;

            // Skip if already failed
            if (!result) {
                return;
            }

            // Try to find the matching node in the target, if it's not found then look in all the
            // sub-targets
            const auto target_index = std::type_index{typeid(node_target)};
            if (processed_target->node_type() == target_index) {
                return;
            }

            for (const auto& sub_target : processed_target->sub_targets()) {
                if (sub_target.node_type() == target_index) {
                    return;
                }
            }

            result = parse_exception{"Dependent argument missing (needs to be before the "
                                     "requiring token on the command line)",
                                     parsing::node_token_type<node_target>()};
        });

        return result;
    }

private:
    template <typename T>
    struct policy_checker {
        constexpr static auto value =
            traits::has_long_name_method_v<T> || traits::has_short_name_method_v<T>;
    };

    static_assert((sizeof...(DependsPolicies) > 0), "At least one name needed for dependent");
    static_assert(policy::is_all_policies_v<depends_policies_type>,
                  "All parameters must be policies");
    static_assert(boost::mp11::mp_all_of<depends_policies_type, policy_checker>::value,
                  "All parameters must provide a long and/or short form name");

    // Find the nearest parent with a routing policy. By definition the
    // dependent cannot be the owner, so filter that out
    template <typename... Parents>
    using nearest_mode =
        policy::nearest_mode_like<boost::mp11::mp_pop_front<std::tuple<Parents...>>>;

    // Starting from ModeType, recurse down through the tree and find all the nodes referred to in
    // DependsPolicies
    template <typename DependsPoliciesTuple, typename ModeType>
    struct depends_targets {
        template <typename Current, typename... Parents>
        struct visitor {
            // If current is one of the DependsPolicies and Parents is not empty, then use the first
            // element of Parents (i.e. the owning node of the name policy).  If not, then set to
            // void
            using type = boost::mp11::mp_eval_if_c<
                !(boost::mp11::mp_contains<DependsPoliciesTuple, Current>::value &&
                  (sizeof...(Parents) > 0)),
                void,
                boost::mp11::mp_at,
                std::tuple<Parents...>,
                traits::integral_constant<std::size_t{0}>>;
        };

        using type =
            boost::mp11::mp_remove_if<utility::tree_type_recursor_collector_t<visitor, ModeType>,
                                      std::is_void>;

        static_assert(std::tuple_size_v<type> == std::tuple_size_v<DependsPoliciesTuple>,
                      "Number of found modes must match depends policy count");
        static_assert(std::tuple_size_v<boost::mp11::mp_unique<type>> == std::tuple_size_v<type>,
                      "Node dependency list must be unique, do you have short and long "
                      "names from the same node?");
    };

    template <typename DependsNodesTuple, typename ModeType>
    struct cyclic_dependency_checker {
        // For each depends, find all of its depends, stop when there are no more or if you hit this
        // policy - static_assert
        template <std::size_t I, typename Nodes>
        [[nodiscard]] constexpr static bool check() noexcept
        {
            if constexpr (I >= std::tuple_size_v<Nodes>) {
                return true;
            } else {
                using depends_type = std::tuple_element_t<I, Nodes>;
                if constexpr (algorithm::has_specialisation_v<
                                  dependent_t,
                                  typename depends_type::policies_type>) {
                    if constexpr (boost::mp11::mp_contains<typename depends_type::policies_type,
                                                           dependent_t>::value) {
                        return false;
                    } else {
                        using targets =
                            typename depends_targets<typename depends_type::depends_policies_type,
                                                     ModeType>::type;

                        return check<I + 1, boost::mp11::mp_append<Nodes, targets>>();
                    }
                }

                return check<I + 1, Nodes>();
            }
        }

        constexpr static bool value = check<0, DependsNodesTuple>();
    };
};

/** Constructs a dependent_t with the given policies.
 *
 * This is used for similarity with arg_t.
 * @tparam DependsPolicies Pack of policies that define its behaviour
 * @param policies Pack of policy instances
 * @return Alias instance
 */
template <typename... DependsPolicies>
[[nodiscard]] constexpr auto dependent(DependsPolicies... policies) noexcept
{
    return dependent_t{std::move(policies)...};
}

template <typename... DependsPolicies>
struct is_policy<dependent_t<DependsPolicies...>> : std::true_type {
};
}  // namespace arg_router::policy
