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
    static_assert(std::is_integral_v<typename T::value_type>,
                  "T must have a value_type that is an integral");

public:
    /** Underlying value type. */
    using value_type = typename T::value_type;

    /** @return Minimum count value. */
    constexpr static value_type minimum_count() { return T::value; }
};

/** Constant variable helper.
 *
 * @tparam Value Minimum count value
 */
template <auto Value>
constexpr auto min_count = min_count_t<traits::integral_constant<Value>>{};

template <typename T>
struct is_policy<min_count_t<T>> : std::true_type {
};

/** For arguments that can repeat e.g. counter_flag and positional_arg, this
 * sets the inclusive maximum number of those repeats.
 *
 * @tparam T Compile-time integral constant value
 */
template <typename T>
class max_count_t
{
    static_assert(std::is_integral_v<typename T::value_type>,
                  "T must have a value_type that is an integral");

public:
    /** Underlying value type. */
    using value_type = typename T::value_type;

    /** @return Maximum count value. */
    constexpr static value_type maximum_count() { return T::value; }
};

/** Constant variable helper.
 *
 * @tparam Value Maximum count value
 */
template <auto Value>
constexpr auto max_count = max_count_t<traits::integral_constant<Value>>{};

template <typename T>
struct is_policy<max_count_t<T>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
