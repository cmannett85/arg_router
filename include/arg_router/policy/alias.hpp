/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/algorithm.hpp"
#include "arg_router/parsing.hpp"
#include "arg_router/policy/no_result_value.hpp"
#include "arg_router/policy/policy.hpp"
#include "arg_router/utility/tree_recursor.hpp"
#include "arg_router/utility/tuple_iterator.hpp"

namespace arg_router
{
namespace policy
{
/** Allows the 'aliasing' of arguments, i.e. a single argument will set multiple
 * others.
 *
 * @note An aliased argument cannot be routed, it's aliased arguments are set
 * instead
 * @tparam AliasedPolicies Pack of policies to alias
 */
template <typename... AliasedPolicies>
class alias_t : public no_result_value<>
{
public:
    /** Tuple of policy types. */
    using aliased_policies_type = std::tuple<std::decay_t<AliasedPolicies>...>;

    /** Policy priority. */
    constexpr static auto priority = std::size_t{100};

    /** Constructor.
     *
     * @param policies Policy instances
     */
    constexpr explicit alias_t(
        [[maybe_unused]] const AliasedPolicies&... policies)
    {
    }

    /** Duplicates any value tokens as aliases of other nodes.
     * 
     * The token duplication mechanism has two approaches, depending on the
     * owning node's fixed count:
     *  - If the count is zero then it is flag-like so the aliased names are
     *    just appended to the processed part of @a tokens
     *  - If the count is greater than zero then it is argument-like and the
     *    aliased names are appended to the processed part of @a tokens,
     *    each followed by @em count tokens (i.e. the values)
     *  
     * In either circumstance the original tokens are removed as they are for
     * the alias, rather than the @em aliased.
     *
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param tokens Currently processed tokens
     * @param processed_tokens Processed tokens performed by previous pre-parse
     * phases calls on other nodes
     * @param parents Parent node instances
     * @return Either true if successful, or a parse_exception if there too
     * few value tokens
     */
    template <typename... Parents>
    [[nodiscard]] parsing::pre_parse_result pre_parse_phase(
        parsing::dynamic_token_adapter& tokens,
        [[maybe_unused]] parsing::token_list::pending_view_type
            processed_tokens,
        [[maybe_unused]] const Parents&... parents) const
    {
        using node_type = boost::mp11::mp_first<std::tuple<Parents...>>;
        static_assert(
            !node_type::template any_phases_v<
                typename node_type::value_type,
                policy::has_parse_phase_method,
                policy::has_validation_phase_method,
                policy::has_routing_phase_method>,
            "Alias owning node cannot have policies that support parse, "
            "validation, or routing phases");

        // Find the owning mode
        using mode_type = typename nearest_mode<Parents...>::type;
        static_assert(!std::is_void_v<mode_type>, "Cannot find parent mode");

        // Find all the aliased targets
        using targets =
            typename alias_targets<aliased_policies_type, mode_type>::type;
        static_assert(cyclic_dependency_checker<targets, mode_type>::value,
                      "Cyclic dependency detected");

        // Verify that all the targets have the same count
        constexpr auto count = node_fixed_count<node_type>();
        check_target_counts<count, targets>();

        // +1 because the node must be named
        if ((count + 1) > tokens.size()) {
            return parse_exception{
                "Too few values for alias, needs " + std::to_string(count),
                parsing::node_token_type<node_type>()};
        }

        // Because this node's job is to insert extra tokens, and therefore
        // isn't a 'real' node in itself, it needs to return
        // skip_node_but_use_tokens so the owning tree node keeps the
        // side-effects (i.e. the updated lists) but doesn't check the label
        // token.  We don't want label checking in the owning tree_node because
        // we need to _replace_ the alias label token with the aliased tokens,
        // so any label check against this node will fail.  However, we do need
        // to do a label check _here_, otherwise this aliasing will occur
        // everytime the pre-parse is called - even on tokens that do not belong
        // to this aliased node!
        {
            auto alias_label = *tokens.begin();
            if (alias_label.prefix == parsing::prefix_type::none) {
                alias_label = parsing::get_token_type(alias_label.name);
            }

            if (!parsing::match<node_type>(alias_label)) {
                return parsing::pre_parse_action::skip_node;
            }
        }

        // Attempt a transfer so we can guarantee that the original tokens are
        // in the processed container (this is a no-op if they already are)
        tokens.transfer(tokens.begin() + count);

        tokens.processed().reserve(tokens.processed().size() +
                                   (std::tuple_size_v<targets> * (count + 1)));

        utility::tuple_type_iterator<targets>([&](auto i) {
            using target_type = std::tuple_element_t<i, targets>;
            const auto tt = parsing::node_token_type<target_type>();

            // Copy the aliased tokens to the front of the unprocessed
            auto it = tokens.processed().insert(
                tokens.processed().begin() + ((i + 1) * (count + 1)),
                tt);

            auto value_it = tokens.begin();
            for (auto j = 0u; j < count; ++j) {
                // Pre-increment the value iterator so we skip over the
                // label token
                tokens.processed().insert(++it, *(++value_it));
            }
        });

        // Now remove the original tokens as they refer to the aliased node
        tokens.processed().erase(tokens.processed().begin(),
                                 tokens.processed().begin() + count + 1);

        // The owning node's name checker will fail us now (because we removed
        // this node's label token), but we still want to keep the processed
        // and unprocessed container changes for later processing
        return parsing::pre_parse_action::skip_node_but_use_tokens;
    }

private:
    template <typename T>
    struct policy_checker {
        constexpr static auto value = traits::has_long_name_method_v<T> ||
                                      traits::has_short_name_method_v<T>;
    };

    static_assert((sizeof...(AliasedPolicies) > 0),
                  "At least one name needed for alias");
    static_assert(policy::is_all_policies_v<aliased_policies_type>,
                  "All parameters must be policies");
    static_assert(
        boost::mp11::mp_all_of<aliased_policies_type, policy_checker>::value,
        "All parameters must provide a long and/or short form name");

    template <typename NodeType>
    [[nodiscard]] constexpr static std::size_t node_fixed_count() noexcept
    {
        static_assert(
            traits::has_minimum_count_method_v<NodeType> &&
                traits::has_maximum_count_method_v<NodeType>,
            "Aliased nodes must have minimum and maximum count methods");
        static_assert(NodeType::minimum_count() == NodeType::maximum_count(),
                      "Aliased nodes must have a fixed count");

        return NodeType::minimum_count();
    }

    // Find the nearest parent with a routing policy. By definition the aliased
    // cannot be the owner, so filter that out
    template <typename... Parents>
    using nearest_mode = policy::nearest_mode_like<
        boost::mp11::mp_pop_front<std::tuple<Parents...>>>;

    // Starting from ModeType, recurse down through the tree and find all the
    // nodes referred to in AliasPolicies
    template <typename AliasPoliciesTuple, typename ModeType>
    struct alias_targets {
        template <typename Current, typename... Parents>
        struct visitor {
            // If current is one of the AliasedPolicies and Parents is not
            // empty, then use the first element of Parents (i.e. the owning
            // node of the name policy).  If not, then set to void
            using type = boost::mp11::mp_eval_if_c<
                !(boost::mp11::mp_contains<AliasPoliciesTuple,
                                           Current>::value &&
                  (sizeof...(Parents) > 0)),
                void,
                boost::mp11::mp_at,
                std::tuple<Parents...>,
                traits::integral_constant<std::size_t{0}>>;
        };

        using type = boost::mp11::mp_remove_if<
            utility::tree_type_recursor_t<visitor, ModeType>,
            std::is_void>;

        static_assert(std::tuple_size_v<type> ==
                          std::tuple_size_v<AliasPoliciesTuple>,
                      "Number of found modes must match alias policy count");
        static_assert(
            std::tuple_size_v<boost::mp11::mp_unique<type>> ==
                std::tuple_size_v<type>,
            "Node alias list must be unique, do you have short and long "
            "names from the same node?");
    };

    template <typename AliasNodesTuple, typename ModeType>
    struct cyclic_dependency_checker {
        // For each alias, find all of its alias, stop when there are no
        // more or if you hit this policy - static_assert
        template <std::size_t I, typename Nodes>
        [[nodiscard]] constexpr static bool check() noexcept
        {
            if constexpr (I >= std::tuple_size_v<Nodes>) {
                return true;
            } else {
                using aliased_type = std::tuple_element_t<I, Nodes>;
                if constexpr (algorithm::has_specialisation_v<
                                  alias_t,
                                  typename aliased_type::policies_type>) {
                    if constexpr (boost::mp11::mp_contains<
                                      typename aliased_type::policies_type,
                                      alias_t>::value) {
                        return false;
                    } else {
                        using targets = typename alias_targets<
                            typename aliased_type::aliased_policies_type,
                            ModeType>::type;

                        return check<I + 1,
                                     boost::mp11::mp_append<Nodes, targets>>();
                    }
                }

                return check<I + 1, Nodes>();
            }
        }

        constexpr static bool value = check<0, AliasNodesTuple>();
    };

    template <std::size_t Count, typename TargetsTuple>
    constexpr static void check_target_counts() noexcept
    {
        utility::tuple_type_iterator<TargetsTuple>([](auto i) {
            using target_type = std::tuple_element_t<i, TargetsTuple>;

            static_assert(
                node_fixed_count<target_type>() == Count,
                "All alias targets must have a count that matches the owner");
        });
    }
};

/** Constructs a alias_t with the given policies.
 *
 * This is used for similarity with arg_t.
 * @tparam AliasedPolicies Pack of policies that define its behaviour
 * @param policies Pack of policy instances
 * @return Alias instance
 */
template <typename... AliasedPolicies>
[[nodiscard]] constexpr auto alias(AliasedPolicies... policies) noexcept
{
    return alias_t{std::move(policies)...};
}

template <typename... AliasedPolicies>
struct is_policy<alias_t<AliasedPolicies...>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
