#pragma once

#include "arg_router/policy/max_count.hpp"
#include "arg_router/policy/min_count.hpp"

namespace arg_router
{
namespace policy
{
/** For arguments that can repeat e.g. counter_flag and positional_arg, this
 * sets the number of those repeats.
 *
 * @tparam T Compile-time integral constant value
 */
template <typename T>
class count_t : public min_count_t<T>, public max_count_t<T>
{
public:
    /** @return Count value. */
    constexpr static std::size_t count() { return T::value; }
};

/** Constant variable helper.
 *
 * @tparam Value Maximum count value
 */
template <std::size_t Value>
constexpr auto count = count_t<traits::integral_constant<Value>>{};

template <typename T>
struct is_policy<count_t<T>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
