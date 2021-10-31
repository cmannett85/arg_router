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
 * is itself the root.
 * 
 * @tparam T Type the rule is for.  You can use any template
 * parameters in the definition, as it is despecialised when comparing against
 * the user's tree types
 * @tparam Conditions A pack of conditions that all must be satisfied for
 * compilation to be successful
 */
template <typename T, typename... Conditions>
using rule = std::tuple<T, Conditions...>;

/** A policy that provides validation checking against a parse tree root.
 *
 * Unless you have created a custom tree component, there is no need to specify
 * this on the root as it will automatically use the standard_validator if one
 * is not specified.
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
    template <typename T>
    struct policy_or_tree_node {
        constexpr static bool value =
            policy::is_policy_v<T> || is_tree_node_v<T>;
    };

    static_assert(
        boost::mp11::mp_all_of<
            rules_type,
            boost::mp11::mp_bind<
                policy_or_tree_node,
                boost::mp11::mp_bind<boost::mp11::mp_first,
                                     boost::mp11::_1>>::template fn>::value,
        "All validator keys must be policies or tree_nodes");

    template <typename Current>
    using rule_lookup = boost::mp11::mp_bind<
        traits::is_same_when_despecialised,
        boost::mp11::mp_bind<boost::mp11::mp_first, boost::mp11::_1>,
        Current>;

    struct validate_fn {
        template <typename Current, typename... Parents>
        constexpr static void fn()
        {
            constexpr auto rule_index = boost::mp11::mp_find_if<
                rules_type,
                rule_lookup<Current>::template fn>::value;
            static_assert(rule_index != std::tuple_size_v<rules_type>,
                          "No rule for Current");

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
     * A <TT>static_assert</TT> will fail will a useful(ish) error message if
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
 * 
 * @tparam ModeType Mode type
 */
template <template <typename...> typename ModeType>
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

    template <typename T, typename... Parents>
    constexpr static void check()
    {
        using ParentTuple = std::tuple<Parents...>;
        constexpr auto NumParents = sizeof...(Parents);

        // Make sure there's at least one parent beyond the owner
        if constexpr (NumParents > 1) {
            // Find a mode type, if there's one present we stop moving up through
            // the ancestors at that point, otherwise we go up to the root
            constexpr auto mode_index =
                algorithm::find_specialisation_v<ModeType, ParentTuple>;
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
            utility::tree_recursor<checker<T, path_type>, start_type>();
        }
    }
};

/** A rule condition that checks the parent index @a I is one of the types in
 * @a ParentTypes.
 * 
 * <TT>I == 0</TT> is the owner.  The comparison is done against despecialised
 * versions.
 * @tparam I Parent index
 * @tparam ParentTypes Parent types to check against
 */
template <std::size_t I, typename... ParentTypes>
struct parent_type {
    template <typename T, typename... Parents>
    constexpr static void check()
    {
        static_assert(sizeof...(ParentTypes) > 0,
                      "Must be at least one owner type");

        static_assert(sizeof...(Parents) > I,
                      "Not enough parents for condition to pass");

        using parent_type = std::tuple_element_t<I, std::tuple<Parents...>>;

        static_assert(
            algorithm::count_despecialised_v<parent_type,
                                             std::tuple<ParentTypes...>>,
            "Policy's owner must be one of a set of specific types");
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

/** A rule condition that checks if there are more than one child of T that is a
 * mode, all must be named.
 *
 * @tparam ModeType Mode type
 */
template <template <typename...> typename ModeType>
struct multiple_named_modes {
    template <typename Child>
    using mode_filter = traits::is_specialisation_of<Child, ModeType>;

    template <typename Mode>
    using has_long_name = traits::has_long_name_method<Mode>;

    template <typename T, typename...>
    constexpr static void check()
    {
        using modes =
            boost::mp11::mp_filter<mode_filter, typename T::children_type>;
        if constexpr ((std::tuple_size_v<modes>) > 1) {
            static_assert(boost::mp11::mp_all_of<modes, has_long_name>::value,
                          "If there are multiple modes, all must be named");
        }
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
 * @tparam PositionalArgTypes The types to be regarded as positional args
 */
template <template <typename...> typename... PositionalArgTypes>
struct positional_args_must_be_at_end {
    static_assert((sizeof...(PositionalArgTypes)),
                  "Must be at least one PositionalArgTypes");

    template <typename T>
    using pos_arg_checker = boost::mp11::mp_any_of<
        std::tuple<traits::is_specialisation_of<T, PositionalArgTypes>...>,
        boost::mp11::mp_to_bool>;

    template <typename T, typename...>
    constexpr static void check()
    {
        // Find the first index of a positional arg, then for each element type
        // after, check that it is also a positional arg

        // Remap the child types to a tuple of booleans, where true means that
        // they are specialisations of one of the given positional arg types
        using children_type = typename T::children_type;
        using is_pos_arg_map =
            boost::mp11::mp_transform<pos_arg_checker, children_type>;

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
 * 
 * @tparam PositionalArgTypes The types to be regarded as positional args
 */
template <template <typename...> typename... PositionalArgTypes>
struct positional_args_must_have_fixed_count_if_not_at_end {
    static_assert((sizeof...(PositionalArgTypes)),
                  "Must be at least one PositionalArgTypes");

    template <typename T>
    using pos_arg_checker = typename positional_args_must_be_at_end<
        PositionalArgTypes...>::template pos_arg_checker<T>;

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
        using pos_args = boost::mp11::mp_filter<pos_arg_checker, children_type>;

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

/** The default validator instance. */
inline constexpr auto default_validator = validator<
    rule<policy::long_name_t<S_("rule")>,
         despecialised_unique_in_owner,
         policy_unique_from_owner_parent_to_mode_or_root<mode_t>>,
    rule<policy::short_name_t<traits::integral_constant<'a'>>,
         despecialised_unique_in_owner,
         policy_unique_from_owner_parent_to_mode_or_root<mode_t>>,
    rule<policy::description_t<S_("rule")>,  //
         despecialised_unique_in_owner>,
    rule<policy::router<std::less<>>,  //
         parent_type<1, root_t<policy::validation::validator<>>>>,
    rule<policy::validation::validator<>,  //
         despecialised_unique_in_owner>,
    rule<policy::required_t<>,  //
         despecialised_unique_in_owner>,
    rule<policy::default_value<int>,  //
         despecialised_unique_in_owner>,
    rule<policy::custom_parser<int>,  //
         despecialised_unique_in_owner>,
    rule<policy::min_count_t<traits::integral_constant<std::size_t{1}>>,  //
         despecialised_unique_in_owner>,
    rule<policy::max_count_t<traits::integral_constant<std::size_t{1}>>,  //
         despecialised_unique_in_owner>,
    rule<policy::count_t<traits::integral_constant<std::size_t{1}>>,  //
         despecialised_unique_in_owner>,
    rule<policy::alias_t<policy::short_name_t<traits::integral_constant<'a'>>>,
         aliased_must_not_be_in_owner,
         despecialised_unique_in_owner>,
    rule<policy::has_value_tokens_t,  //
         despecialised_unique_in_owner>,
    rule<policy::has_contiguous_value_tokens_t,  //
         despecialised_unique_in_owner>,
    rule<flag_t<>,  //
         must_not_have_policy<policy::required_t>,
         must_not_have_policy<policy::custom_parser>,
         must_not_have_policy<policy::validation::validator>,
         at_least_one_of_policies<policy::long_name_t, policy::short_name_t>,
         must_have_policy<policy::description_t>>,
    rule<arg_t<int>,  //
         must_not_have_policy<policy::validation::validator>,
         at_least_one_of_policies<policy::long_name_t, policy::short_name_t>,
         one_of_policies_if_parent_is_not_root<policy::required_t,
                                               policy::default_value,
                                               policy::alias_t>,
         must_have_policy<policy::description_t>>,
    rule<positional_arg_t<std::vector<int>>,  //
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
    rule<mode_t<flag_t<>>,  //
         must_not_have_policy<policy::short_name_t>,
         must_not_have_policy<policy::custom_parser>,
         must_not_have_policy<policy::default_value>,
         child_must_not_have_policy<policy::router>,
         positional_args_must_be_at_end<positional_arg_t>,
         positional_args_must_have_fixed_count_if_not_at_end<positional_arg_t>,
         // By requiring its parent to be root, in 'inherits' the root's child
         // policy requirements
         parent_type<0, root_t<policy::validation::validator<>>>>,
    rule<root_t<policy::validation::validator<>>,  //
         must_have_policy<policy::validation::validator>,
         min_child_count<1>,
         child_must_have_policy<policy::router>,
         child_must_not_have_policy<policy::required_t>,
         child_must_not_have_policy<policy::alias_t>,
         multiple_named_modes<mode_t>>>{};
}  // namespace validation
}  // namespace policy
}  // namespace arg_router
