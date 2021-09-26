#pragma once

#include "arg_router/policy/policy.hpp"

namespace arg_router
{
namespace policy
{
/** Used to a mark a command line argument type as required i.e. it is a parse
 * error if the token is missing.
 */
template <typename = void>  // This is needed due so it can be used in
struct required_t {         // template template parameters
};

/** Constant variable helper. */
constexpr auto required = required_t<>{};

/** Evaluates to true if @a T is marked as required.
 *
 * @tparam T Type to test
 */
template <typename T>
using is_required = std::is_base_of<required_t<>, T>;

/** Helper variable for is_required.
 *
 * @tparam T Type to test
 */
template <typename T>
constexpr bool is_required_v = is_required<T>::value;

template <>
struct is_policy<required_t<>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
