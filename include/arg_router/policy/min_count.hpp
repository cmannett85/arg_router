#pragma once

#include "arg_router/policy/policy.hpp"
#include "arg_router/traits.hpp"

namespace arg_router
{
namespace policy
{
/** For arguments that can repeat e.g. counter_flag and positional_arg, this
 * sets the inclusive minimum number of those repeats.
 *
 * @tparam T Compile-time integral constant value
 */
template <typename T>
class min_count_t
{
    static_assert(traits::is_detected_v<traits::has_value_type_checker, T>,
                  "T must have a value_type");
    static_assert(std::is_convertible_v<typename T::value_type, std::size_t>,
                  "T must have a value_type that is implicitly convertible to "
                  "std::size_t");
    static_assert(std::is_integral_v<typename T::value_type>,
                  "T must be an integral type");
    static_assert((T::value >= 0),
                  "T must have a value_type that is a positive number");

public:
    /** @return Minimum count value. */
    constexpr static std::size_t minimum_count() { return T::value; }
};

/** Constant variable helper.
 *
 * @tparam Value Minimum count value
 */
template <std::size_t Value>
constexpr auto min_count = min_count_t<traits::integral_constant<Value>>{};

template <typename T>
struct is_policy<min_count_t<T>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
