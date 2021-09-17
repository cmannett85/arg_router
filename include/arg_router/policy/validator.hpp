#pragma once

#include "arg_router/flag.hpp"
#include "arg_router/policy/custom_parser.hpp"
#include "arg_router/policy/default_value.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/short_name.hpp"
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

    template <typename T, typename... Parents>
    constexpr static void check()
    {
        // Make sure there's at least one parent beyond the owner
        if constexpr (sizeof...(Parents) > 1) {
            // Find a mode type, if there's one present we stop moving up through
            // the ancestors at that point, otherwise we go up to the root
            //constexpr auto generation_count =
            //algorithm::find_specialisation_in_tuple_v<mode, older_generations> + 1;
            using start_type = boost::mp11::mp_back<std::tuple<Parents...>>;
            using path_type = std::tuple<Parents...>;

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

/** A rule condition that checks that @a T's policies do contain @a Policy.
 *
 * @tparam Policy Despecialised policy to check for
 */
template <template <typename...> typename Policy>
struct must_have_policy {
    template <typename T, typename...>
    constexpr static void check()
    {
        static_assert(
            algorithm::has_specialisation_v<Policy, typename T::policies_type>,
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
        static_assert(
            !algorithm::has_specialisation_v<Policy, typename T::policies_type>,
            "T must not have this policy");
    }
};

/** A rule condition that checks that every child under @a T has a particular
 * policy.
 *
 * @tparam Policy Despecialised policy to check for
 */
template <template <typename...> typename Policy>
struct child_must_have_policy {
    template <typename Child>
    using child_checker =
        algorithm::has_specialisation<Policy, typename Child::policies_type>;

    template <typename T, typename...>
    constexpr static void check()
    {
        static_assert(boost::mp11::mp_all_of<typename T::children_type,
                                             child_checker>::value,
                      "All children of T must have this policy");
    }
};

/** The default validator instance. */
inline constexpr auto default_validator =
    validator<rule<policy::long_name_t<S_("rule")>,
                   despecialised_unique_in_owner,
                   policy_unique_from_owner_parent_to_mode_or_root>,
              rule<policy::short_name_t<traits::integral_constant<'a'>>,
                   despecialised_unique_in_owner,
                   policy_unique_from_owner_parent_to_mode_or_root>,
              rule<policy::description_t<S_("rule")>,  //
                   despecialised_unique_in_owner>,
              rule<policy::router<std::less<>>,  //
                   parent_type<1, root<policy::validation::validator<>>>>,
              rule<policy::validation::validator<>,  //
                   despecialised_unique_in_owner>,
              rule<flag<>,  //
                   must_not_have_policy<policy::required_t>,
                   must_not_have_policy<policy::default_value>,
                   must_not_have_policy<policy::custom_parser>,
                   must_not_have_policy<policy::validation::validator>,
                   must_have_policy<policy::description_t>>,
              rule<root<policy::validation::validator<>>,  //
                   must_have_policy<policy::validation::validator>,
                   child_must_have_policy<policy::router>>>{};
}  // namespace validation
}  // namespace policy
}  // namespace arg_router
