#pragma once

#include "arg_router/parsing.hpp"
#include "arg_router/policy/policy.hpp"
#include "arg_router/traits.hpp"
#include "arg_router/utility/tree_recursor.hpp"

#include <boost/core/ignore_unused.hpp>

namespace arg_router
{
namespace policy
{
template <typename... DependsPolicies>
class dependent_t
{
public:
    /** Tuple of policy types. */
    using depends_policies_type = std::tuple<DependsPolicies...>;

    /** Constructor.
     *
     * @param policies Policy instances
     */
    constexpr explicit dependent_t(DependsPolicies&&... policies)
    {
        boost::ignore_unused(policies...);
    }

protected:
    template <typename... Parents>
    void pre_parse_phase(parsing::token_list& tokens,
                         utility::span<const parsing::token_type>& view,
                         const Parents&... parents) const
    {
        // Find the owning mode
        using mode_type = typename nearest_mode<Parents...>::type;
        static_assert(!std::is_void_v<mode_type>, "Cannot find parent mode");

        // Find all the depends targets
        using targets =
            typename depends_targets<depends_policies_type, mode_type>::type;
        static_assert(cyclic_dependency_checker<targets, mode_type>::value,
                      "Cyclic dependency detected");

        boost::ignore_unused(view, parents...);

        // Find the target tokens, tokens that have already been processed are
        // valid too
        utility::tuple_type_iterator<targets>([&](auto i) {
            using target = std::tuple_element_t<i, targets>;

            const auto finder = [&](parsing::token_type token) {
                auto found = std::find(tokens.pending_view().begin(),
                                       tokens.pending_view().end(),
                                       token) != tokens.pending_view().end();
                if (!found) {
                    found = std::find(tokens.processed_view().begin(),
                                      tokens.processed_view().end(),
                                      token) != tokens.processed_view().end();
                }

                return found;
            };

            auto found = false;
            if constexpr (traits::has_long_name_method_v<target>) {
                found = finder({parsing::prefix_type::LONG,  //
                                target::long_name()});
            }
            if constexpr (traits::has_short_name_method_v<target>) {
                found = !found && finder({parsing::prefix_type::SHORT,
                                          target::short_name()});
            }

            if (!found) {
                throw parse_exception{"Dependent argument missing",
                                      parsing::node_token_type<target>()};
            }
        });
    }

private:
    template <typename T>
    struct policy_checker {
        constexpr static auto value = traits::has_long_name_method_v<T> ||
                                      traits::has_short_name_method_v<T>;
    };

    static_assert((sizeof...(DependsPolicies) > 0),
                  "At least one name needed for dependent");
    static_assert(policy::is_all_policies_v<depends_policies_type>,
                  "All parameters must be policies");
    static_assert(
        boost::mp11::mp_all_of<depends_policies_type, policy_checker>::value,
        "All parameters must provide a long and/or short form name");

    // Find the nearest parent with a routing policy
    template <typename... Parents>
    struct nearest_mode {
        // By definition the dependent cannot be the owner, so filter that out
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
    // nodes referred to in DependsPolicies
    template <typename DependsPoliciesTuple, typename ModeType>
    struct depends_targets {
        template <typename Current, typename... Parents>
        struct visitor {
            // If current is one of the DependsPolicies and Parents is not
            // empty, then use the first element of Parents (i.e. the owning
            // node of the name policy).  If not, then set to void
            using type = boost::mp11::mp_eval_if_c<
                !(boost::mp11::mp_contains<DependsPoliciesTuple,
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
                          std::tuple_size_v<DependsPoliciesTuple>,
                      "Number of found modes must match depends policy count");
        static_assert(
            std::tuple_size_v<boost::mp11::mp_unique<type>> ==
                std::tuple_size_v<type>,
            "Node dependency list must be unique, do you have short and long "
            "names from the same node?");
    };

    template <typename DependsNodesTuple, typename ModeType>
    struct cyclic_dependency_checker {
        // For each depends, find all of its depends, stop when there are no
        // more or if you hit this policy - static_assert
        template <std::size_t I, typename Nodes>
        constexpr static bool check()
        {
            if constexpr (I >= std::tuple_size_v<Nodes>) {
                return true;
            } else {
                using depends_type = std::tuple_element_t<I, Nodes>;
                if constexpr (algorithm::has_specialisation_v<
                                  dependent_t,
                                  typename depends_type::policies_type>) {
                    if constexpr (boost::mp11::mp_contains<
                                      typename depends_type::policies_type,
                                      dependent_t>::value) {
                        return false;
                    } else {
                        using targets = typename depends_targets<
                            typename depends_type::depends_policies_type,
                            ModeType>::type;

                        return check<I + 1,
                                     boost::mp11::mp_append<Nodes, targets>>();
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
constexpr dependent_t<DependsPolicies...> dependent(DependsPolicies... policies)
{
    return dependent_t{std::move(policies)...};
}

template <typename... DependsPolicies>
struct is_policy<dependent_t<DependsPolicies...>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
