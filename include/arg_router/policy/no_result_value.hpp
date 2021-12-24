#pragma once

#include "arg_router/policy/policy.hpp"

namespace arg_router
{
namespace policy
{
/** Indicates that a node or derived policy's owning node does not create a
 * return value when parsed.
 * 
 * Policies or nodes can derive from this.
 */
template <typename = void>  // This is needed due so it can be used in
struct no_result_value {    // template template parameters
};

/** Evaluates to true if @a T uses no_result_value.
 *
 * @tparam T Type to test
 */
template <typename T>
using has_no_result_value = std::is_base_of<no_result_value<>, T>;

/** Helper variable for has_no_result_value.
 *
 * @tparam T Type to test
 */
template <typename T>
constexpr bool has_no_result_value_v = has_no_result_value<T>::value;

template <>
struct is_policy<no_result_value<>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
