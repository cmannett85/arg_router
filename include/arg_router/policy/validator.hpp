/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/arg.hpp"
#include "arg_router/counting_flag.hpp"
#include "arg_router/dependency/alias_group.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/help.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/custom_parser.hpp"
#include "arg_router/policy/default_value.hpp"
#include "arg_router/policy/dependent.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/display_name.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/short_form_expander.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/positional_arg.hpp"
#include "arg_router/root.hpp"
#include "arg_router/utility/tree_recursor.hpp"

namespace arg_router
{
namespace policy
{
// We have to do this forward declaration and specialisation otherwise the rule key check below
// fails when the validator itself is tested
namespace validation
{
template <typename... Rules>
class validator;
}

template <typename... Rules>
struct is_policy<validation::validator<Rules...>> : std::true_type {
};

/** Namespace for types associated with parse tree validation.
 */
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
 * Where <TT>T</TT> is the current type from the tree, from the root down to policy level, and
 * <TT>Parents</TT> is a pack of ancestors in increasing generation from <TT>T</TT>.  The last in
 * the pack is always the root unless <TT>T</TT> is itself the root, in which case <TT>Parents</TT>
 * is empty.
 *
 * @tparam T A trait-like type that has a static bool member <TT>value</TT> that evaluates to true
 * if the tested type is a match (e.g. <TT>std::is_same</TT>)
 * @tparam Conditions A pack of conditions that all must be satisfied for compilation to be
 * successful
 */
template <template <typename...> typename T, typename... Conditions>
using rule = std::tuple<boost::mp11::mp_quote<T>, Conditions...>;

/** Quoted metafunction rule overload.
 * 
 * This is the only way to use template params when defining a rule.  The resulting type of @a T
 * should have the form:
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
 * @tparam T A quoted metafunction type with a nested template <TT>fn</TT> that has a static bool
 * member <TT>value</TT> that evaluates to true if the tested
 * @tparam Conditions A pack of conditions that all must be satisfied for compilation to be
 * successful
 */
template <typename T, typename... Conditions>
using rule_q = std::tuple<T, Conditions...>;

/** A policy that provides validation checking against a parse tree root.
 * 
 * The rules are checked in order, so where there is overlap (i.e. a policy or tree_node could be
 * valid in multiple entries) be sure to list the more specific rule first.
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
            constexpr static bool value = boost::mp11::mp_first<Rule>::template fn<Current>::value;
        };
    };

    struct validate_fn {
        template <typename Current, typename... Parents>
        constexpr static void fn() noexcept
        {
            // Find the matching rule
            constexpr auto rule_index = boost::mp11::mp_find_if_q<  //
                rules_type,
                rule_lookup<Current>>::value;
            static_assert(rule_index != std::tuple_size_v<rules_type>, "No rule for Current");

            // Remove the rule key so we just have a list of conditions
            using conditions =
                boost::mp11::mp_drop_c<std::tuple_element_t<rule_index, rules_type>, 1>;
            utility::tuple_type_iterator<conditions>([](auto i) {
                using condition = std::tuple_element_t<i, conditions>;
                condition::template check<Current, Parents...>();
            });
        }
    };

public:
    /** Trigger the validation.
     *
     * A <TT>static_assert</TT> will fail with a useful(ish) error message if validation fails.
     * @tparam Root Root type to validate
     * @return void
     */
    template <typename Root>
    constexpr static void validate() noexcept
    {
        utility::tree_type_recursor<validate_fn, Root>();
    }
};

/** Namespace for a collection of common rule_q types.
 */
namespace common_rules
{
/** A rule that evaluates to true when the @a Current tree type despecialised matches to any of
 * @a T.
 *
 * @tparam T Despecialised types to compare to
 */
template <template <typename...> typename... T>
struct despecialised_any_of_rule {
    static_assert((sizeof...(T) > 0), "Must be at least one despecialised type");

    template <typename Current>
    struct fn {
        constexpr static bool value =
            boost::mp11::mp_or<traits::is_specialisation_of<Current, T>...>::value;
    };
};
}  // namespace common_rules

/** A rule condition that checks that the type is despecialised unique in its owner.
 */
struct despecialised_unique_in_owner {
    template <typename T, typename... Parents>
    constexpr static void check() noexcept
    {
        if constexpr (sizeof...(Parents) > 0) {
            using Owner = boost::mp11::mp_first<std::tuple<Parents...>>;

            // Check that there's only one in the owner (itself)
            static_assert(algorithm::count_despecialised_v<T, typename Owner::policies_type> == 1,
                          "Policy must be present and unique in owner");
        }
    }
};

/** A rule condition that checks a policy is unique up to the nearest mode or root - but skips the 
 * owner.
 * 
 * @tparam ModeTypes Pack of types that are considered modes
 */
template <template <typename...> typename... ModeTypes>
struct policy_unique_from_owner_parent_to_mode_or_root {
    static_assert(sizeof...(ModeTypes), "Must be at least one mode type");

    template <typename Policy, typename PathToThis>
    struct checker {
        template <typename Current, typename... Parents>
        constexpr static void fn() noexcept
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

    template <typename T>
    using is_mode =
        boost::mp11::mp_any_of<std::tuple<traits::is_specialisation_of<T, ModeTypes>...>,
                               boost::mp11::mp_to_bool>;

    // Don't recurse into child modes, they effectively have their own namespace
    struct skipper {
        template <typename Current, typename...>
        [[nodiscard]] constexpr static bool fn() noexcept
        {
            return is_mode<Current>::value;
        }
    };

    template <typename T, typename... Parents>
    constexpr static void check() noexcept
    {
        using ParentTuple = std::tuple<Parents...>;
        constexpr auto NumParents = sizeof...(Parents);

        // Make sure there's at least one parent beyond the owner
        if constexpr (NumParents > 1) {
            // Find a mode type, if there's one present we stop moving up through the ancestors at
            // that point, otherwise we go up to the root
            constexpr auto mode_index = boost::mp11::mp_find_if<ParentTuple, is_mode>::value;
            using start_type = boost::mp11::mp_eval_if_c<mode_index == NumParents,
                                                         boost::mp11::mp_back<ParentTuple>,
                                                         boost::mp11::mp_at,
                                                         ParentTuple,
                                                         traits::integral_constant<mode_index>>;

            using path_type =
                boost::mp11::mp_take_c<ParentTuple, std::min(mode_index + 1, NumParents)>;

            // Recurse the tree from the oldest generation, testing that no other policy matches
            // ours
            utility::tree_type_recursor<checker<T, path_type>, skipper, start_type>();
        }
    }
};

/** Defines the mapping of a parent ancestry index against an expected node type.
 *
 * @tparam Index Ancestry index, 0 being the owner
 * @tparam ParentType Despecialised type expected to be at the index
 */
template <std::size_t Index, template <typename...> typename ParentType>
struct parent_index_pair_type {
    /** Ancestry index */
    constexpr static std::size_t index = Index;

    /** Functional type that does the despecialised comparison of @a ParentType against @a T.
     *
     * @tparam T Type to despecialised compare against
     */
    template <typename T>
    using fn = traits::is_specialisation_of<T, ParentType>;
};

/** A rule condition that checks one of the parent index and type pairs given in @a ParentIndexTypes
 * matches @a Parents.
 *
 * @a ParentIndexTypes can have multiple entries against the same index, as long as one of the types
 * is a match then the check will pass.
 * @tparam ParentIndexTypes Pack of parent_index_pair_type
 */
template <typename... ParentIndexTypes>
struct parent_types {
    static_assert(sizeof...(ParentIndexTypes) > 0, "Must be at least one parent_index_pair_type");

    template <std::size_t MaxIndex>
    struct index_filter {
        template <typename Pair>
        using fn = boost::mp11::mp_bool<(Pair::index < MaxIndex)>;
    };

    template <typename... Parents>
    struct checker {
        template <typename Pair>
        using fn =
            typename Pair::template fn<std::tuple_element_t<Pair::index, std::tuple<Parents...>>>;
    };

    template <typename T, typename... Parents>
    constexpr static void check() noexcept
    {
        // Remove any entries whose index is beyond the Parents list size
        using clamped_indices = boost::mp11::mp_filter_q<index_filter<sizeof...(Parents)>,
                                                         std::tuple<ParentIndexTypes...>>;

        // Check that each despecialised parent type matches the corresponding parent in the tree
        using matches = boost::mp11::mp_transform_q<checker<Parents...>, clamped_indices>;

        static_assert(boost::mp11::mp_any_of<matches, boost::mp11::mp_to_bool>::value,
                      "Parent must be one of a set of types");
    }
};

/** A rule condition that checks that @a T's policies contain all of @a Policies.
 *
 * @tparam Policies Despecialised policies to check for
 */
template <template <typename...> typename... Policies>
struct must_have_policies {
    template <typename T>
    using checker = boost::mp11::mp_all_of<
        std::tuple<algorithm::has_specialisation<Policies, typename T::policies_type>...>,
        boost::mp11::mp_to_bool>;

    template <typename T, typename...>
    constexpr static void check() noexcept
    {
        static_assert(checker<T>::value, "T must have all these policies");
    }
};

/** A rule condition that checks that @a T's policies contain none of @a Policies.
 *
 * @tparam Policies Despecialised policies to check for
 */
template <template <typename...> typename... Policies>
struct must_not_have_policies {
    template <typename T>
    using checker = boost::mp11::mp_none_of<
        std::tuple<algorithm::has_specialisation<Policies, typename T::policies_type>...>,
        boost::mp11::mp_to_bool>;

    template <typename T, typename...>
    constexpr static void check() noexcept
    {
        static_assert(checker<T>::value, "T must have none of these policies");
    }
};

template <template <typename...> typename Policy>
struct basic_child_must_have_policy {
    template <typename Child>
    using child_checker = algorithm::has_specialisation<Policy, typename Child::policies_type>;

    template <typename T>
    constexpr static bool value =
        boost::mp11::mp_all_of<typename T::children_type, child_checker>::value;
};

/** A rule condition that checks that every child under @a T has a particular policy.
 *
 * @tparam Policy Despecialised policy to check for
 */
template <template <typename...> typename Policy>
struct child_must_have_policy {
    template <typename T, typename...>
    constexpr static void check() noexcept
    {
        static_assert(basic_child_must_have_policy<Policy>::template value<T>,
                      "All children of T must have this policy");
    }
};

/** A rule condition that checks that every child under @a T does not have a particular policy.
 *
 * @tparam Policy Despecialised policy to check for
 */
template <template <typename...> typename Policy>
struct child_must_not_have_policy {
    template <typename T, typename...>
    constexpr static void check() noexcept
    {
        if constexpr ((std::tuple_size_v<typename T::children_type>) > 0) {
            static_assert(!basic_child_must_have_policy<Policy>::template value<T>,
                          "All children of T must not have this policy");
        }
    }
};

/** A rule condition that checks the parent node of the policy @a T does not contain @a Policy.
 */
template <template <typename...> typename Policy>
struct policy_parent_must_not_have_policy {
    template <typename T, typename... Parents>
    constexpr static void check() noexcept
    {
        static_assert(policy::is_policy_v<T>, "T must be a policy");
        static_assert(sizeof...(Parents) >= 1, "Must be at least one parent");

        using parent = boost::mp11::mp_first<std::tuple<Parents...>>;
        static_assert(!algorithm::has_specialisation_v<Policy, typename parent::policies_type>,
                      "Parent must not have this policy");
    }
};

/** A rule condition that checks if there are more than one child of @a T that is a mode, only one
 * can be anonymous.
 * 
 * @tparam ModeTypes Pack of types that are considered modes
 */
template <template <typename...> typename... ModeTypes>
struct single_anonymous_mode {
    static_assert(sizeof...(ModeTypes), "Must be at least one mode type");

    template <typename T>
    using is_mode =
        boost::mp11::mp_any_of<std::tuple<traits::is_specialisation_of<T, ModeTypes>...>,
                               boost::mp11::mp_to_bool>;

    template <typename T>
    struct is_anonymous_mode {
        constexpr static auto value = []() {
            if constexpr (is_mode<T>::value) {
                return T::is_anonymous;
            }
            return false;
        }();
    };

    template <typename T, typename...>
    constexpr static void check() noexcept
    {
        constexpr auto num_anonymous = boost::mp11::mp_count_if<  //
            typename T::children_type,
            is_anonymous_mode>::value;

        static_assert((num_anonymous <= 1), "Only one child mode can be anonymous");
    }
};

/** A rule condition that checks that one of the @a Policies is in T.
 *
 * @tparam Policies Pack of policies
 */
template <template <typename...> typename... Policies>
struct at_least_one_of_policies {
    static_assert(sizeof...(Policies) >= 2, "Condition requires at least two policies");

    template <typename T, typename...>
    constexpr static void check() noexcept
    {
        static_assert(
            (algorithm::count_specialisation_v<Policies, typename T::policies_type> + ...) >= 1,
            "T must have at least one of the policies");
    }
};

/** A rule condition that checks that positional arg @a T is at the end of the non-positional args
 * in a node's child list.
 * 
 * The above is a long-winded way of saying that positional args must be at the end of a child list
 * for a node, no policies or other node types may appear after.
 * 
 * @tparam PositionalArgTypes Pack of types that are considered positional args
 */
template <template <typename...> typename... PositionalArgTypes>
struct positional_args_must_be_at_end {
    static_assert(sizeof...(PositionalArgTypes), "Must be at least one positional arg type");

    template <typename T>
    using is_positonal_arg =
        boost::mp11::mp_any_of<std::tuple<traits::is_specialisation_of<T, PositionalArgTypes>...>,
                               boost::mp11::mp_to_bool>;

    template <typename T, typename...>
    constexpr static void check() noexcept
    {
        // Find the first index of a positional arg, then for each element type after, check that it
        // is also a positional arg

        // Remap the child types to a tuple of booleans, where true means that they are
        // specialisations of one of the given positional arg types
        using children_type = typename T::children_type;
        using is_pos_arg_map = boost::mp11::mp_transform_q<
            boost::mp11::mp_bind<boost::mp11::mp_to_bool,
                                 boost::mp11::mp_bind<is_positonal_arg, boost::mp11::_1>>,

            children_type>;

        constexpr auto first_pos_arg =
            boost::mp11::mp_find<is_pos_arg_map, boost::mp11::mp_true>::value;
        if constexpr (first_pos_arg != std::tuple_size_v<is_pos_arg_map>) {
            using drop_before_first_pos_arg = boost::mp11::mp_drop_c<is_pos_arg_map, first_pos_arg>;
            static_assert(
                boost::mp11::mp_all_of<drop_before_first_pos_arg, boost::mp11::mp_to_bool>::value,
                "Positional args must all appear at the end of "
                "nodes/policy list for a node");
        }
    }
};

/** A rule condition that checks that positional arg @a T has a fixed argument count if not at the
 * end of the positional arg list.
 * 
 * @tparam PositionalArgTypes Pack of types that are considered positional args
 */
template <template <typename...> typename... PositionalArgTypes>
struct positional_args_must_have_fixed_count_if_not_at_end {
    static_assert(sizeof...(PositionalArgTypes), "Must be at least one positional arg type");

    template <typename T>
    using is_positonal_arg =
        boost::mp11::mp_any_of<std::tuple<traits::is_specialisation_of<T, PositionalArgTypes>...>,
                               boost::mp11::mp_to_bool>;

    template <typename T>
    struct fixed_count_checker {
        [[nodiscard]] constexpr static bool f() noexcept
        {
            if constexpr (traits::has_minimum_count_method_v<T> &&
                          traits::has_maximum_count_method_v<T>) {
                return T::minimum_count() == T::maximum_count();
            }

            return false;
        }

        constexpr static auto value = f();
    };

    template <typename T, typename...>
    constexpr static void check() noexcept
    {
        using children_type = typename T::children_type;
        using pos_args = boost::mp11::mp_filter<is_positonal_arg, children_type>;

        constexpr auto num_pos_args = std::tuple_size_v<pos_args>;
        if constexpr (num_pos_args > 0) {
            using needs_fixed_count = boost::mp11::mp_take_c<pos_args, num_pos_args - 1>;
            using has_fixed_count = boost::mp11::mp_all_of<needs_fixed_count, fixed_count_checker>;
            static_assert(has_fixed_count::value,
                          "Positional args not at the end of the list must "
                          "have a fixed count");
        }
    }
};

namespace detail
{
template <typename RuleTuple>
struct validator_from_tuple_impl {
    static_assert(traits::always_false_v<RuleTuple>, "RuleTuple must be a tuple-like type");
};

template <template <typename...> typename Tuple, typename... Rules>
struct validator_from_tuple_impl<Tuple<Rules...>> {
    using type = validator<Rules...>;
};
}  // namespace detail

/** Defines a validator type built from a tuple-like type of rules.
 *
 * @tparam RuleTuple Tuple of rules
 */
template <typename RuleTuple>
using validator_from_tuple = typename detail::validator_from_tuple_impl<RuleTuple>::type;

/** The default validator instance. */
inline constexpr auto default_validator = validator<
    // List the policies first as there are more of them and therefore more likely to be the target
    // Name policy rules
    rule_q<common_rules::despecialised_any_of_rule<policy::long_name_t, policy::short_name_t>,
           despecialised_unique_in_owner,
           policy_unique_from_owner_parent_to_mode_or_root<arg_router::mode_t>>,
    // None name
    rule_q<common_rules::despecialised_any_of_rule<policy::none_name_t>,
           despecialised_unique_in_owner,
           policy_unique_from_owner_parent_to_mode_or_root<arg_router::mode_t>,
           policy_parent_must_not_have_policy<policy::long_name_t>,
           policy_parent_must_not_have_policy<policy::short_name_t>,
           policy_parent_must_not_have_policy<policy::display_name_t>>,
    // Display name
    rule_q<common_rules::despecialised_any_of_rule<policy::display_name_t>,
           despecialised_unique_in_owner,
           policy_parent_must_not_have_policy<policy::long_name_t>,
           policy_parent_must_not_have_policy<policy::short_name_t>,
           policy_parent_must_not_have_policy<policy::none_name_t>>,
    // Required
    rule_q<common_rules::despecialised_any_of_rule<policy::required_t>,
           despecialised_unique_in_owner,
           policy_parent_must_not_have_policy<policy::default_value>>,
    // Default value
    rule_q<common_rules::despecialised_any_of_rule<policy::default_value>,
           despecialised_unique_in_owner,
           policy_parent_must_not_have_policy<policy::required_t>>,
    // Router
    rule_q<common_rules::despecialised_any_of_rule<policy::router>,
           despecialised_unique_in_owner,
           parent_types<parent_index_pair_type<0, mode_t>, parent_index_pair_type<1, root_t>>>,
    // Generic policy rule
    rule<policy::is_policy,  //
         despecialised_unique_in_owner>,

    // Tree nodes
    // Flag
    rule_q<common_rules::despecialised_any_of_rule<flag_t>,
           must_not_have_policies<policy::multi_stage_value,
                                  policy::no_result_value,
                                  policy::required_t,
                                  policy::validation::validator>>,
    // Arg
    rule_q<common_rules::despecialised_any_of_rule<arg_t>,
           must_not_have_policies<policy::multi_stage_value,
                                  policy::no_result_value,
                                  policy::validation::validator>>,
    // Counting flag
    rule_q<common_rules::despecialised_any_of_rule<counting_flag_t>,
           must_not_have_policies<policy::no_result_value,
                                  policy::required_t,
                                  policy::validation::validator>>,
    // Positional arg
    rule_q<common_rules::despecialised_any_of_rule<positional_arg_t>,
           must_not_have_policies<policy::alias_t,
                                  policy::multi_stage_value,
                                  policy::no_result_value,
                                  policy::validation::validator>>,
    // one_of
    rule_q<common_rules::despecialised_any_of_rule<dependency::one_of_t>,
           must_not_have_policies<policy::no_result_value,
                                  policy::multi_stage_value,
                                  policy::validation::validator>,
           child_must_not_have_policy<policy::required_t>,
           child_must_not_have_policy<policy::default_value>,
           parent_types<parent_index_pair_type<0, mode_t>>>,
    // alias_group
    rule_q<common_rules::despecialised_any_of_rule<dependency::one_of_t, dependency::alias_group_t>,
           must_not_have_policies<policy::no_result_value, policy::validation::validator>,
           child_must_not_have_policy<policy::required_t>,
           child_must_not_have_policy<policy::default_value>,
           parent_types<parent_index_pair_type<0, mode_t>>>,
    // Mode
    rule_q<common_rules::despecialised_any_of_rule<mode_t>,
           must_not_have_policies<policy::multi_stage_value,  //
                                  policy::required_t>,
           positional_args_must_be_at_end<positional_arg_t>,
           positional_args_must_have_fixed_count_if_not_at_end<positional_arg_t>,
           parent_types<parent_index_pair_type<0, root_t>, parent_index_pair_type<0, mode_t>>>,
    // Help
    rule_q<common_rules::despecialised_any_of_rule<help_t>,
           must_not_have_policies<policy::multi_stage_value,
                                  policy::required_t,
                                  policy::short_form_expander_t,
                                  policy::validation::validator>,
           parent_types<parent_index_pair_type<0, root_t>>>,
    // Root
    rule_q<common_rules::despecialised_any_of_rule<root_t>,
           must_have_policies<policy::validation::validator>,
           must_not_have_policies<policy::multi_stage_value, policy::no_result_value>,
           child_must_not_have_policy<policy::required_t>,
           child_must_not_have_policy<policy::alias_t>,
           single_anonymous_mode<arg_router::mode_t>>>{};
}  // namespace validation
}  // namespace policy
}  // namespace arg_router
