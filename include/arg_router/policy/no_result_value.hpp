#pragma once

#include <type_traits>

namespace arg_router
{
namespace policy
{
/** This isn't a policy, but a trait type for policies.  It indicates that the
 * derived policy's owning node does not create a return value when parsed.
 * 
 * Policies should derive from this.
 */
struct no_result_value {
};

/** Evaluates to true if @a T uses no_result_value.
 *
 * @tparam T Type to test
 */
template <typename T>
using has_no_result_value = std::is_base_of<no_result_value, T>;

/** Helper variable for has_no_result_value.
 *
 * @tparam T Type to test
 */
template <typename T>
constexpr bool has_no_result_value_v = has_no_result_value<T>::value;
}  // namespace policy
}  // namespace arg_router
