#pragma once

#include "arg_router/arg.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/node_category.hpp"
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/count.hpp"
#include "arg_router/policy/custom_parser.hpp"
#include "arg_router/policy/default_value.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/has_contiguous_value_tokens.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/positional_arg.hpp"
#include "arg_router/root.hpp"
#include "arg_router/utility/compile_time_string.hpp"
#include "arg_router/utility/tree_recursor.hpp"

namespace arg_router
{
namespace policy
{
// We have to do this forward declaration and specialisation otherwise the
// rule key check below fails when the validator itself is tested
namespace validation
{
template <typename... Rules>
class validator;
}

template <typename... Rules>
struct is_policy<validation::validator<Rules...>> : std::true_type {
};

namespace validation
{
/** Defines a validator rule.
 *
 * See validator for how it is to be used.
 * 
 * A condition is defined as:
 * @code
 * struct my_condition {
 *     template <typename T, typename... Parents>
 *     constexpr static void check()
 *     {
 *         // Compile-time checks
 *     }
 * };
 * @endcode
 * Where <TT>T</TT> is the current type from the tree, from the root down to policy
 * level, and <TT>Parents</TT> is a pack of ancestors in increasing generation
 * from <TT>T</TT>.  The last in the pack is always the root unless <TT>T</TT>
 * is itself the root, in which case <TT>Parents</TT> is empty.
 *
 * @tparam T A trait-like type that has a static bool member <TT>value</TT> that
 * evaluates to true if the tested type is a match (e.g. <TT>std::is_same</TT>)
 * @tparam Conditions A pack of conditions that all must be satisfied for
 * compilation to be successful
 */
template <template <typename...> typename T, typename... Conditions>
using rule = std::tuple<boost::mp11::mp_quote<T>, Conditions...>;

/** Quoted metafunction rule overload.
 * 
 * This is the only way to use template params when defining a rule.  The
 * resulting type of @a T should have the form:
 * @code
 * template <typename MyParam>
 * struct my_rule {
 *     template <typename T>
 *     struct fn {
 *         constexpr static bool value = ...;
 *     };
 * };
 * @endcode
 * 
 * @tparam T A quoted metafunction type with a nested template <TT>fn</TT> that
 * has a static bool member <TT>value</TT> that evaluates to true if the tested
 * @tparam Conditions A pack of conditions that all must be satisfied for
 * compilation to be successful
 */
template <typename T, typename... Conditions>
using rule_q = std::tuple<T, Conditions...>;

/** A policy that provides validation checking against a parse tree root.
 * 
 * The rules are checked in order, so where there is overlap (i.e. a policy or
 * tree_node could be valid in multiple entries) be sure to list the more
 * specific rule first.
 * 
 * @tparam Rules A pack of rule types
 */
template <typename... Rules>
class validator
{
public:
    /** Validation rules. */
    using rules_type = std::tuple<Rules...>;

private:
    template <typename Current>
    struct rule_lookup {
        template <typename Rule>
        struct fn {
            constexpr static bool value =
                boost::mp11::mp_first<Rule>::template fn<Current>::value;
        };
    };

    struct validate_fn {
        template <typename Current, typename... Parents>
        constexpr static void fn()
        {
            // Find the matching rule
            constexpr auto rule_index = boost::mp11::mp_find_if_q<  //
                rules_type,
                rule_lookup<Current>>::value;
            static_assert(rule_index != std::tuple_size_v<rules_type>,
                          "No rule for Current");

            // Remove the rule key so we just have a list of conditions
            using conditions = boost::mp11::
                mp_drop_c<std::tuple_element_t<rule_index, rules_type>, 1>;
            utility::tuple_type_iterator<conditions>([](auto /*i*/, auto ptr) {
                using condition = std::remove_pointer_t<decltype(ptr)>;
                condition::template check<Current, Parents...>();
            });
        }
    };

public:
    /** Trigger the validation.
     *
     * A <TT>static_assert</TT> will fail with a useful(ish) error message if
     * validation fails.
     * @tparam Root Root type to validate
     * @return void
     */
    template <typename Root>
    constexpr static void validate()
    {
        utility::tree_recursor<validate_fn, Root>();
    }
};

/** Namespace for a collection of common rule_q types.
 */
namespace common_rules
{
/** A rule that evaluates to true when the @a Current tree type despecialised
 * matches to any of @a T.
 *
 * @tparam T Despecialised types to compare to
 */
template <template <typename...> typename... T>
struct despecialised_any_of_rule {
    static_assert((sizeof...(T) > 0),
                  "Must be at least one despecialised type");

    template <typename Current>
    struct fn {
        constexpr static bool value = boost::mp11::mp_or<
            traits::is_specialisation_of<Current, T>...>::value;
    };
};
}  // namespace common_rules

/** A rule condition that checks that the type is despecialised unique in its
 * owner.
 */
struct despecialised_unique_in_owner {
    template <typename T, typename... Parents>
    constexpr static void check()
    {
        if constexpr (sizeof...(Parents) > 0) {
            using Owner = boost::mp11::mp_first<std::tuple<Parents...>>;

            // Check that there's only one in the owner (itself)
            static_assert(algorithm::count_despecialised_v<
                              T,
                              typename Owner::policies_type> == 1,
                          "Policy must be present and unique in owner");
        }
    }
};

/** A rule condition that checks a policy is unique up to the nearest mode or
 * root - but skips the owner.
 */
struct policy_unique_from_owner_parent_to_mode_or_root {
    template <typename Policy, typename PathToThis>
    struct checker {
        template <typename Current, typename... Parents>
        constexpr static void fn()
        {
            using path_type = std::tuple<Parents...>;

            // Skip checking ourself
            if constexpr (!std::is_same_v<PathToThis, path_type>) {
                static_assert(!std::is_same_v<Policy, Current>,
                              "Policy must be unique in the parse tree up to "
                              "the nearest mode or root");
            }
        }
    };

    // Don't recurse into child modes, they effectively have their own namespace
    struct skipper {
        template <typename Current, typename...>
        constexpr static bool fn()
        {
            return node_category::is_generic_mode_like_v<Current>;
        }
    };

    template <typename T, typename... Parents>
    constexpr static void check()
    {
        using ParentTuple = std::tuple<Parents...>;
        constexpr auto NumParents = sizeof...(Parents);

        // Make sure there's at least one parent beyond the owner
        if constexpr (NumParents > 1) {
            // Find a mode type, if there's one present we stop moving up through
            // the ancestors at that point, otherwise we go up to the root
            constexpr auto mode_index = boost::mp11::mp_find_if<
                ParentTuple,
                node_category::is_generic_mode_like>::value;
            using start_type = boost::mp11::mp_eval_if_c<
                mode_index == NumParents,
                boost::mp11::mp_back<ParentTuple>,
                boost::mp11::mp_at,
                ParentTuple,
                traits::integral_constant<mode_index>>;

            using path_type =
                boost::mp11::mp_take_c<ParentTuple,
                                       std::min(mode_index + 1, NumParents)>;

            // Recurse the tree from the oldest generation, testing that no
            // other policy matches ours
            utility::
                tree_recursor<checker<T, path_type>, skipper, start_type>();
        }
    }
};

/** Defines the mapping of a parent ancestry index against an expected node
 * type.
 *
 * @tparam Index Ancestry index, 0 being the owner
 * @tparam ParentType Despecialised type expected to be at the index
 */
template <std::size_t Index, template <typename...> typename ParentType>
struct parent_index_pair_type {
    /** Ancestry index */
    constexpr static std::size_t index = Index;

    /** Functional type that does the despecialised comparison of @a ParentType
     * against @a T.
     *
     * @tparam T Type to despecialised compare against
     */
    template <typename T>
    using fn = traits::is_specialisation_of<T, ParentType>;
};

/** A rule condition that checks one of the parent index and type pairs given
 * in @a ParentIndexTypes matches @a Parents.
 *
 * @a ParentIndexTypes can have multiple entries against the same index, as long
 * as one of the types is a match then the check will pass.
 * @tparam ParentIndexTypes Pack of parent_index_pair_type
 */
template <typename... ParentIndexTypes>
struct parent_types {
    static_assert(sizeof...(ParentIndexTypes) > 0,
                  "Must be at least one parent_index_pair_type");

    template <std::size_t MaxIndex>
    struct index_filter {
        template <typename Pair>
        using fn = boost::mp11::mp_bool<(Pair::index < MaxIndex)>;
    };

    template <typename... Parents>
    struct checker {
        template <typename Pair>
        using fn = typename Pair::template fn<
            std::tuple_element_t<Pair::index, std::tuple<Parents...>>>;
    };

    template <typename T, typename... Parents>
    constexpr static void check()
    {
        // Remove any entries whose index is beyond the Parents list size
        using clamped_indices =
            boost::mp11::mp_filter_q<index_filter<sizeof...(Parents)>,
                                     std::tuple<ParentIndexTypes...>>;

        // Check that each despecialised parent type matches the corresponding
        // parent in the tree
        using matches =
            boost::mp11::mp_transform_q<checker<Parents...>, clamped_indices>;

        static_assert(
            boost::mp11::mp_any_of<matches, boost::mp11::mp_to_bool>::value,
            "Parent must be one of a set of types");
    }
};

template <template <typename...> typename Policy>
struct basic_must_have_policy {
    template <typename T>
    constexpr static auto value =
        algorithm::has_specialisation_v<Policy, typename T::policies_type>;
};

/** A rule condition that checks that @a T's policies do contain @a Policy.
 *
 * @tparam Policy Despecialised policy to check for
 */
template <template <typename...> typename Policy>
struct must_have_policy {
    template <typename T, typename...>
    constexpr static void check()
    {
        static_assert(basic_must_have_policy<Policy>::template value<T>,
                      "T must have this policy");
    }
};

/** A rule condition that checks that @a T's policies do not contain @a Policy.
 *
 * @tparam Policy Despecialised policy to check for
 */
template <template <typename...> typename Policy>
struct must_not_have_policy {
    template <typename T, typename...>
    constexpr static void check()
    {
        static_assert(!basic_must_have_policy<Policy>::template value<T>,
                      "T must not have this policy");
    }
};

template <template <typename...> typename Policy>
struct basic_child_must_have_policy {
    template <typename Child>
    using child_checker =
        algorithm::has_specialisation<Policy, typename Child::policies_type>;

    template <typename T>
    constexpr static auto value =
        boost::mp11::mp_all_of<typename T::children_type, child_checker>::value;
};

/** A rule condition that checks that every child under @a T has a particular
 * policy.
 *
 * @tparam Policy Despecialised policy to check for
 */
template <template <typename...> typename Policy>
struct child_must_have_policy {
    template <typename T, typename...>
    constexpr static void check()
    {
        static_assert(basic_child_must_have_policy<Policy>::template value<T>,
                      "All children of T must have this policy");
    }
};

/** A rule condition that checks that every child under @a T does not have a
 * particular policy.
 *
 * @tparam Policy Despecialised policy to check for
 */
template <template <typename...> typename Policy>
struct child_must_not_have_policy {
    template <typename T, typename...>
    constexpr static void check()
    {
        if constexpr ((std::tuple_size_v<typename T::children_type>) > 0) {
            static_assert(
                !basic_child_must_have_policy<Policy>::template value<T>,
                "All children of T must not have this policy");
        }
    }
};

/** A rule condition that checks if there are more than one child of @a T that 
 * is a mode, only one can be anonymous.
 */
struct single_anonymous_mode {
    template <typename T, typename...>
    constexpr static void check()
    {
        constexpr auto num_anonymous = boost::mp11::mp_count_if<  //
            typename T::children_type,
            node_category::is_anonymous_mode_like>::value;

        static_assert((num_anonymous <= 1),
                      "Only one child mode can be anonymous");
    }
};

/** A rule condition that checks that one of the @a Policies is in T.
 *
 * @tparam Policies Pack of policies
 */
template <template <typename...> typename... Policies>
struct at_least_one_of_policies {
    static_assert(sizeof...(Policies) >= 2,
                  "Condition requires at least two policies");

    template <typename T, typename...>
    constexpr static void check()
    {
        static_assert(
            (algorithm::count_specialisation_v<Policies,
                                               typename T::policies_type> +
             ...) >= 1,
            "T must have at least one of the policies");
    }
};

/** A rule condition that checks one of the @a Policies is in T.
 *
 * @tparam Policies Pack of policies
 */
template <template <typename...> typename... Policies>
struct one_of_policies_if_parent_is_not_root {
    static_assert(sizeof...(Policies) >= 1,
                  "Condition requires at least one policy");

    template <typename T, typename... Parents>
    constexpr static void check()
    {
        // This is needed otherwise the mp_first call fails if there aren't
        // enough elements in Parents - even if inside a if constexpr
        // expression
        using parent_type = boost::mp11::mp_eval_if_c<(sizeof...(Parents) < 1),
                                                      std::void_t<>,
                                                      boost::mp11::mp_first,
                                                      std::tuple<Parents...>>;

        if constexpr (!(traits::is_specialisation_of_v<parent_type, root_t>)) {
            static_assert(
                (algorithm::count_specialisation_v<Policies,
                                                   typename T::policies_type> +
                 ...) == 1,
                "T must have one of the assigned policies");
        }
    }
};

/** A rule condition that checks the number of children in @a T is greater than
 * @a MinChildren.
 *
 * @tparam MinChildren Minimum number of children @a T must have
 */
template <std::size_t MinChildren>
struct min_child_count {
    template <typename T, typename...>
    constexpr static void check()
    {
        constexpr auto num_children =
            std::tuple_size_v<typename T::children_type>;

        static_assert(num_children >= MinChildren,
                      "Minimum child count not reached");
    }
};

/** A rule condition that checks the alias names of @a T are not in the owner.
 */
struct aliased_must_not_be_in_owner {
    template <typename Alias, typename... Parents>
    constexpr static void check()
    {
        static_assert((sizeof...(Parents) > 0), "Alias must have an owner");

        using our_policies = typename Alias::aliased_policies_type;
        using owner_type = boost::mp11::mp_first<std::tuple<Parents...>>;
        using owner_policies = typename owner_type::policies_type;

        using policy_intersection =
            boost::mp11::mp_set_intersection<our_policies, owner_policies>;
        static_assert(std::tuple_size_v<policy_intersection> == 0,
                      "Alias names cannot appear in owner");
    }
};

/** A rule condition that checks that positional arg @a T is at the end of the
 * non-positional args in a node's child list.
 * 
 * The above is a long-winded way of saying that positional args must be at the
 * end of a child list for a node, no policies or other node types may appear
 * after.
 */
struct positional_args_must_be_at_end {
    template <typename T, typename...>
    constexpr static void check()
    {
        // Find the first index of a positional arg, then for each element type
        // after, check that it is also a positional arg

        // Remap the child types to a tuple of booleans, where true means that
        // they are specialisations of one of the given positional arg types
        using children_type = typename T::children_type;
        using is_pos_arg_map = boost::mp11::mp_transform_q<
            boost::mp11::mp_bind<
                boost::mp11::mp_to_bool,
                boost::mp11::mp_bind<node_category::is_positional_arg_like,
                                     boost::mp11::_1>>,

            children_type>;

        constexpr auto first_pos_arg =
            boost::mp11::mp_find<is_pos_arg_map, boost::mp11::mp_true>::value;
        if constexpr (first_pos_arg != std::tuple_size_v<is_pos_arg_map>) {
            using drop_before_first_pos_arg =
                boost::mp11::mp_drop_c<is_pos_arg_map, first_pos_arg>;
            static_assert(
                boost::mp11::mp_all_of<drop_before_first_pos_arg,
                                       boost::mp11::mp_to_bool>::value,
                "Positional args must all appear at the end of "
                "nodes/policy list for a node");
        }
    }
};

/** A rule condition that checks that positional arg @a T has a fixed argument
 * count if not at the end of the positional arg list.
 */
struct positional_args_must_have_fixed_count_if_not_at_end {
    template <typename T>
    struct fixed_count_checker {
        constexpr static bool f()
        {
            if constexpr (traits::has_count_method_v<T>) {
                return true;
            } else if constexpr (traits::has_minimum_count_method_v<T> &&
                                 traits::has_maximum_count_method_v<T>) {
                return T::minimum_count() == T::maximum_count();
            }

            return false;
        }

        constexpr static auto value = f();
    };

    template <typename T, typename...>
    constexpr static void check()
    {
        using children_type = typename T::children_type;
        using pos_args =
            boost::mp11::mp_filter<node_category::is_positional_arg_like,
                                   children_type>;

        constexpr auto num_pos_args = std::tuple_size_v<pos_args>;
        if constexpr (num_pos_args > 0) {
            using needs_fixed_count =
                boost::mp11::mp_take_c<pos_args, num_pos_args - 1>;
            using has_fixed_count =
                boost::mp11::mp_all_of<needs_fixed_count, fixed_count_checker>;
            static_assert(has_fixed_count::value,
                          "Positional args not at the end of the list must "
                          "have a fixed count");
        }
    }
};

/** A rule condition that checks if @a T's <TT>min_count()</TT> and
 * <TT>max_count()</TT> methods, if present, are logically separated.
 */
struct validate_counts {
    template <typename T, typename...>
    constexpr static void check()
    {
        if constexpr (traits::has_minimum_count_method_v<T> &&
                      traits::has_maximum_count_method_v<T>) {
            static_assert(T::minimum_count() <= T::maximum_count(),
                          "Minimum count must be less than maximum count");
        }
    }
};

struct cannot_have_fixed_count_of_zero {
    template <typename T, typename...>
    constexpr static void check()
    {
        static_assert(!node_category::has_fixed_count_v<T, 0>,
                      "Cannot have a fixed count of zero");
    }
};

/** A rule condition that checks if @a T does not have fixed count of 1, if so
 * then it's value_type must support a <TT>push_back()</TT> method.
 */
struct if_count_not_one_value_type_must_support_push_back {
    template <typename T>
    constexpr static bool has_fixed_count_of_one()
    {
        if constexpr (traits::has_count_method_v<T>) {
            return T::count() == 1;
        } else if constexpr (traits::has_minimum_count_method_v<T> &&
                             traits::has_maximum_count_method_v<T>) {
            return (T::minimum_count() == T::maximum_count()) &&
                   (T::minimum_count() == 1);
        }

        return false;
    }

    template <typename T, typename...>
    constexpr static void check()
    {
        if constexpr (!has_fixed_count_of_one<T>()) {
            static_assert(
                traits::has_push_back_method_v<typename T::value_type>,
                "If T does not have a fixed count of 1, then its "
                "value_type must have a push_back() method");
        }
    }
};

/** A rule condition that checks that any child mode-like types of @a T are
 * named.
 */
struct child_mode_must_be_named {
    template <typename T, typename...>
    constexpr static void check()
    {
        using anonymous_modes =
            boost::mp11::mp_filter<node_category::is_anonymous_mode_like,
                                   typename T::children_type>;
        static_assert(std::tuple_size_v<anonymous_modes> == 0,
                      "All child modes must be named");
    }
};

/** A rule condition that checks that mode-like type @a T has a router unless
 * all of its children are mode-like too.
 */
template <template <typename...> typename... RouterTypes>
struct mode_router_requirements {
    static_assert(sizeof...(RouterTypes), "Must be at least one RouterTypes");

    template <typename T>
    using router_checker = boost::mp11::mp_any_of<
        std::tuple<traits::is_specialisation_of<T, RouterTypes>...>,
        boost::mp11::mp_to_bool>;

    template <typename T, typename...>
    constexpr static void check()
    {
        constexpr auto all_children_mode_like =
            boost::mp11::mp_all_of<typename T::children_type,
                                   node_category::is_generic_mode_like>::value;

        constexpr auto has_router =
            boost::mp11::mp_any_of<typename T::policies_type,
                                   router_checker>::value;

        static_assert(
            has_router ^ all_children_mode_like,
            "Mode must have a router or all its children are also modes");
    }
};

/** A rule condition that checks that mode-like type @a T, if anonymous, does
 * not have another mode-like child.
 */
struct anonymous_mode_cannot_have_mode_children {
    template <typename T, typename...>
    constexpr static void check()
    {
        if constexpr (node_category::is_anonymous_mode_like_v<T>) {
            static_assert(
                boost::mp11::mp_none_of<
                    typename T::children_type,
                    node_category::is_generic_mode_like>::value,
                "An anonymous mode cannot have any children that are modes");
        }
    }
};

/** The default validator instance. */
inline constexpr auto default_validator = validator<
    // List the policies first as there are more of them and therefore more
    // likely to be the target
    // Name policy rules
    rule_q<common_rules::despecialised_any_of_rule<policy::long_name_t,
                                                   policy::short_name_t>,
           despecialised_unique_in_owner,
           policy_unique_from_owner_parent_to_mode_or_root>,
    // Router
    rule_q<common_rules::despecialised_any_of_rule<policy::router>,
           despecialised_unique_in_owner,
           parent_types<parent_index_pair_type<0, mode_t>,
                        parent_index_pair_type<1, root_t>>>,
    // Alias
    rule_q<common_rules::despecialised_any_of_rule<policy::alias_t>,
           despecialised_unique_in_owner,
           aliased_must_not_be_in_owner>,
    // Generic policy rule
    rule<policy::is_policy,  //
         despecialised_unique_in_owner>,

    // Tree nodes
    // Flag
    rule_q<common_rules::despecialised_any_of_rule<flag_t>,
           must_not_have_policy<policy::required_t>,
           must_not_have_policy<policy::custom_parser>,
           must_not_have_policy<policy::validation::validator>,
           at_least_one_of_policies<policy::long_name_t, policy::short_name_t>,
           must_have_policy<policy::description_t>>,
    // Arg
    rule_q<common_rules::despecialised_any_of_rule<arg_t>,
           must_not_have_policy<policy::validation::validator>,
           at_least_one_of_policies<policy::long_name_t, policy::short_name_t>,
           one_of_policies_if_parent_is_not_root<policy::required_t,
                                                 policy::default_value,
                                                 policy::alias_t>,
           must_have_policy<policy::description_t>>,
    // Positional arg
    rule_q<common_rules::despecialised_any_of_rule<positional_arg_t>,
           must_not_have_policy<policy::validation::validator>,
           must_not_have_policy<policy::short_name_t>,
           must_not_have_policy<policy::required_t>,
           must_not_have_policy<policy::default_value>,
           must_not_have_policy<policy::alias_t>,
           must_not_have_policy<policy::router>,  // Cannot be top-level now
           must_have_policy<policy::long_name_t>,
           must_have_policy<policy::description_t>,
           validate_counts,
           cannot_have_fixed_count_of_zero,
           if_count_not_one_value_type_must_support_push_back>,
    // Mode
    rule_q<common_rules::despecialised_any_of_rule<mode_t>,
           must_not_have_policy<policy::short_name_t>,
           must_not_have_policy<policy::custom_parser>,
           must_not_have_policy<policy::default_value>,
           positional_args_must_be_at_end,
           positional_args_must_have_fixed_count_if_not_at_end,
           child_mode_must_be_named,
           mode_router_requirements<policy::router>,
           anonymous_mode_cannot_have_mode_children,
           parent_types<parent_index_pair_type<0, root_t>,
                        parent_index_pair_type<0, mode_t>>>,
    // Root
    rule_q<common_rules::despecialised_any_of_rule<root_t>,
           must_have_policy<policy::validation::validator>,
           min_child_count<1>,
           child_must_have_policy<policy::router>,
           child_must_not_have_policy<policy::required_t>,
           child_must_not_have_policy<policy::alias_t>,
           single_anonymous_mode>>{};
}  // namespace validation
}  // namespace policy
}  // namespace arg_router
