#pragma once

#include <type_traits>

#include <boost/mp11/algorithm.hpp>

namespace arg_router
{
/** Policy namespace.
 *
 * arg_router uses policies to compose behaviours on command line argument
 * types.
 */
namespace policy
{
/** Evaluates to true if @a T is a policy.
 *
 * Unfortunately due to the way that policies form an is-a relationship with
 * their owners, we can't tag a type as a policy vai inheritance.  So all
 * policies must create a specialisation of this and manually mark themselves as
 * a policy.
 * @tparam T Type to test
 */
template <typename T, typename... Args>
struct is_policy : std::false_type {
};

/** Helper alias variable for is_policy.
 *
 * @tparam T Type to test
 */
template <typename T>
constexpr auto is_policy_v = is_policy<T>::value;

/** Evaluates to true if @a Tuple only contains policy types.
 *
 * @tparam Tuple Tuple of types to test
 */
template <typename Tuple>
struct is_all_policies : boost::mp11::mp_all_of<Tuple, is_policy> {
};

/** Helper alias variable for is_all_policies.
 *
 * @tparam Tuple Tuple of types to test
 */
template <typename Tuple>
constexpr auto is_all_policies_v = is_all_policies<Tuple>::value;
}  // namespace policy
}  // namespace arg_router
