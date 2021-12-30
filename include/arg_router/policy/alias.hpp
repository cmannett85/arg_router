#pragma once

#include "arg_router/algorithm.hpp"
#include "arg_router/exception.hpp"
#include "arg_router/parsing.hpp"
#include "arg_router/policy/no_result_value.hpp"
#include "arg_router/policy/policy.hpp"
#include "arg_router/utility/span.hpp"
#include "arg_router/utility/tree_recursor.hpp"
#include "arg_router/utility/tuple_iterator.hpp"

#include <boost/core/ignore_unused.hpp>
#include <boost/mp11/bind.hpp>

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
    using aliased_policies_type = std::tuple<AliasedPolicies...>;

    /** Constructor.
     *
     * @param policies Policy instances
     */
    constexpr explicit alias_t(AliasedPolicies&&... policies)
    {
        boost::ignore_unused(policies...);
    }

protected:
    /** Duplicates any value tokens as aliases of other nodes.
     * 
     * The owning type must have minimum_count() and maximum_count() methods
     * that return the same value, or a count() method(),  to use an alias.
     * - If the count is zero then it is flag-like so the aliased names are
     *   just inserted into @a tokens after the current token
     * - If the count is greater than zero then it is argument-like and the
     *   aliased names are inserted, each followed by @em count tokens
     *   (i.e. the value), after the current @em count tokens
     *
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param tokens Token list as received from the owning node
     * @param view The tokens to be used in the remaining parse phases, may be
     * empty
     * @param parents Parents instances pack
     * @exception parse_exception Thrown if @a view does not have enough tokens
     * in for the required value count
     */
    template <typename... Parents>
    void pre_parse_phase(parsing::token_list& tokens,
                         utility::span<const parsing::token_type>& view,
                         const Parents&... parents) const
    {
        boost::ignore_unused(parents...);

        // Check the owning node
        using node_type = boost::mp11::mp_first<std::tuple<Parents...>>;
        static_assert(traits::has_minimum_count_method_v<node_type> &&
                          traits::has_maximum_count_method_v<node_type>,
                      "Node requires a count(), or minimum_count() and "
                      "maximum_count() methods to use an alias");
        static_assert(node_type::minimum_count() == node_type::maximum_count(),
                      "Node requires minimum_count() and maximum_count() "
                      "to return the same value to use an alias");

        // Find the owning mode
        using mode_type = typename nearest_mode<Parents...>::type;
        static_assert(!std::is_void_v<mode_type>, "Cannot find parent mode");

        // Find all the aliased targets
        using targets =
            typename alias_targets<aliased_policies_type, mode_type>::type;
        static_assert(cyclic_dependency_checker<targets, mode_type>::value,
                      "Cyclic dependency detected");

        auto new_tokens = parsing::token_list{};
        if constexpr (node_type::minimum_count() == 0) {
            new_tokens.reserve(std::tuple_size_v<targets>);

            utility::tuple_type_iterator<targets>([&](auto i) {
                using target_type = std::tuple_element_t<i, targets>;
                new_tokens.add_pending(parsing::node_token_type<target_type>());
            });
        } else {
            // The first token is the argument name, so skip that
            if (node_type::minimum_count() > view.size()) {
                throw parse_exception{
                    "Too few values for alias, needs " +
                        std::to_string(node_type::minimum_count()),
                    parsing::node_token_type<node_type>()};
            }

            new_tokens.reserve(std::tuple_size_v<targets> *
                               node_type::minimum_count());
            utility::tuple_type_iterator<targets>([&](auto i) {
                using target_type = std::tuple_element_t<i, targets>;
                new_tokens.add_pending(parsing::node_token_type<target_type>());

                for (auto i = 0u; i < node_type::minimum_count(); ++i) {
                    new_tokens.add_pending(view[i]);
                }
            });
        }

        tokens.insert_pending(
            tokens.pending_view().begin() + node_type::minimum_count(),
            new_tokens.pending_view().begin(),
            new_tokens.pending_view().end());

        // Update the view in case the extra tokens have caused a reallocation
        view = tokens.pending_view().subspan(0, view.size());
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

    // Find the nearest parent with a routing policy
    template <typename... Parents>
    struct nearest_mode {
        // By definition the aliased cannot be the owner, so filter that out
        using parent_tuple = boost::mp11::mp_pop_front<std::tuple<Parents...>>;

        template <typename Parent>
        struct policy_finder {
            constexpr static bool value =
                boost::mp11::mp_find_if_q<
                    typename Parent::policies_type,
                    boost::mp11::mp_bind<policy::has_routing_phase_method,
                                         boost::mp11::_1>>::value !=
                std::tuple_size_v<typename Parent::policies_type>;
        };

        using parent_index =
            boost::mp11::mp_find_if<parent_tuple, policy_finder>;

        using type =
            boost::mp11::mp_eval_if_c<parent_index::value ==
                                          std::tuple_size_v<parent_tuple>,
                                      void,
                                      boost::mp11::mp_at,
                                      parent_tuple,
                                      parent_index>;
    };

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
        constexpr static bool check()
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
};

/** Constructs a alias_t with the given policies.
 *
 * This is used for similarity with arg_t.
 * @tparam AliasedPolicies Pack of policies that define its behaviour
 * @param policies Pack of policy instances
 * @return Alias instance
 */
template <typename... AliasedPolicies>
constexpr alias_t<AliasedPolicies...> alias(AliasedPolicies... policies)
{
    return alias_t{std::move(policies)...};
}

template <typename... AliasedPolicies>
struct is_policy<alias_t<AliasedPolicies...>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
