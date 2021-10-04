#pragma once

#include "arg_router/parsing.hpp"

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
class alias_t
{
public:
    /** Tuple of policy types. */
    using aliased_policies_type = std::tuple<AliasedPolicies...>;

private:
    // You could do this a single inline mp11 expression, but it would be
    // unreadable...
    template <typename Node>
    struct node_match {
        constexpr static auto value = boost::mp11::mp_any_of<
            aliased_policies_type,
            boost::mp11::mp_bind<boost::mp11::mp_contains,
                                 typename Node::policies_type,
                                 boost::mp11::_1>::template fn>::value;
    };

    template <typename ModeType>
    constexpr static auto create_aliased_node_indices()
    {
        using children_type = typename ModeType::children_type;

        // Zip together an index-sequence of ModeType's children and the
        // elements of it
        using zipped = algorithm::zip_t<
            boost::mp11::mp_iota_c<std::tuple_size_v<children_type>>,
            children_type>;

        // Filter out the elements that have a long or short name that match
        // our alias
        using filtered = boost::mp11::mp_filter<
            boost::mp11::mp_bind<
                node_match,
                boost::mp11::mp_bind<boost::mp11::mp_second,
                                     boost::mp11::_1>>::template fn,
            zipped>;

        return typename algorithm::unzip<filtered>::first_type{};
    }

public:
    /** Given a mode-like parent, iterate over its children and return the
     * indices for those that are aliased by this policy.
     *
     * @tparam ModeType Mode-like type
     */
    template <typename ModeType>
    using aliased_node_indices =
        decltype(create_aliased_node_indices<ModeType>());

    /** Constructor.
     *
     * @param policies Policy instances
     */
    constexpr explicit alias_t(AliasedPolicies&&... policies)
    {
        (boost::ignore_unused(policies), ...);
    }

private:
    template <typename T>
    struct policy_checker {
        constexpr static auto value =
            traits::is_detected_v<parsing::has_long_name_checker, T> ||
            traits::is_detected_v<parsing::has_short_name_checker, T>;
    };

    static_assert((sizeof...(AliasedPolicies) > 0),
                  "At least one name needed for alias");
    static_assert(policy::is_all_policies_v<aliased_policies_type>,
                  "All parameters must be policies");
    static_assert(
        boost::mp11::mp_all_of<aliased_policies_type, policy_checker>::value,
        "All parameters must provide a long and/or short form name");
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
