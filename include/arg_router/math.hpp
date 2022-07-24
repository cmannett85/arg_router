/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "type_traits"

/** Mathematical functions and types. */
namespace arg_router::math
{
/** Returns the absolute value of the integer @a value.
 *
 * This only exists because <TT>std::abs(T)</TT> is not constexpr.
 * @tparam T Integral type
 * @param value Input
 * @return Absolute value
 */
template <typename T>
[[nodiscard]] constexpr T abs(T value) noexcept
{
    static_assert(std::is_integral_v<T>, "T must be an integral");
    return value < T{0} ? -value : value;
}

/** Returns the number of digits in @a value.
 *
 * Basicaly <TT>log10(value)+1</TT> but constexpr.
 * @tparam T Integral type
 * @param value Input
 * @return Number of digits.
 */
template <typename T>
[[nodiscard]] constexpr T num_digits(T value) noexcept
{
    static_assert(std::is_integral_v<T>, "T must be an integral");

    constexpr auto base = T{10};

    value = abs(value);
    auto i = T{1};
    while (value /= base) {
        ++i;
    }

    return i;
}

/** Power function.
 *
 * This only exists because <TT>std::power(T)</TT> is not constexpr.
 * @tparam Base Power base
 * @tparam T Integral type
 * @param exp Exponent
 * @return @a Base raised to the power @a exp
 */
template <auto Base, typename T>
[[nodiscard]] constexpr T pow(T exp) noexcept
{
    static_assert(std::is_integral_v<T>, "T must be an integral");
    static_assert(Base > 0, "Base must be greater than zero");

    return exp <= T{0} ? T{1} : Base * pow<Base>(exp - T{1});
}
}  // namespace arg_router::math
