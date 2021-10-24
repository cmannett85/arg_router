#pragma once

#include "arg_router/policy/has_value_tokens.hpp"

namespace arg_router
{
namespace policy
{
namespace detail
{
//This is needed due so it can be used in template template parameters
template <typename = void>
struct basic_has_contiguous_value_tokens_t : has_value_tokens_t {
};
}  // namespace detail

/** Used to a mark a node as having one or more value tokens that follow it on
 * the command line.
 *
 * This is not for general users of the library, but developers of new node
 * types.
 */
using has_contiguous_value_tokens_t =
    detail::basic_has_contiguous_value_tokens_t<>;

/** Evaluates to true if @a T is marked as having a value token.
 *
 * @tparam T Type to test
 */
template <typename T>
using has_contiguous_value_tokens =
    std::is_base_of<has_contiguous_value_tokens_t, T>;

/** Helper variable for has_value_token.
 *
 * @tparam T Type to test
 */
template <typename T>
constexpr bool has_contiguous_value_tokens_v =
    has_contiguous_value_tokens<T>::value;

template <>
struct is_policy<has_contiguous_value_tokens_t> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
