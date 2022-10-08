/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/math.hpp"
#include "arg_router/traits.hpp"

#include <boost/mp11/algorithm.hpp>
#include <boost/preprocessor/repetition/enum.hpp>

#include <array>

/** @file
 */

namespace arg_router::utility
{
template <char... Cs>
class compile_time_string;

/** Compile time string.
 *
 * @tparam Cs Pack of chars
 */
template <char... Cs>
class compile_time_string
{
public:
    /** Array of characters as a type. */
    using array_type = std::tuple<traits::integral_constant<Cs>...>;

    /** Number of characters in string.
     *
     * @return String size
     */
    [[nodiscard]] constexpr static std::size_t size() noexcept { return sizeof...(Cs); }

    /** True if string is empty.
     *
     * @return True if number of characters is zero
     */
    [[nodiscard]] constexpr static bool empty() noexcept { return size() == 0; }

    /** Returns the string data as a view.
     *
     * @return View of the string data
     */
    [[nodiscard]] constexpr static std::string_view get() noexcept
    {
        return {sv_.data(), sv_.size()};
    }

    /** Appends @a T to this string type.
     *
     * @tparam T String to append
     */
    template <typename T>
    struct append;

    template <char... OtherCs>
    struct append<compile_time_string<OtherCs...>> {
        using type = compile_time_string<Cs..., OtherCs...>;
    };

    /** Helper alias for append.
     *
     * @tparam T String to append
     */
    template <typename T>
    using append_t = typename append<T>::type;

    /** Concatentation operator.
     *
     * @tparam OtherCs Character pack from other instance
     * @param other Instance to concatenate (only used for CTAD)
     * @return New instance
     */
    template <char... OtherCs>
    [[nodiscard]] constexpr auto operator+(
        [[maybe_unused]] const compile_time_string<OtherCs...>& other) const noexcept
    {
        return append_t<compile_time_string<OtherCs...>>{};
    }

private:
    constexpr static auto sv_ = std::array<char, sizeof...(Cs)>{Cs...};
};

/** Provides a compile time string that is a repeating sequence of @a C @a N characters long.
 *
 * @tparam N Number of times to repeat @a C
 * @tparam C Character to repeat
 */
template <std::size_t N, char C>
class create_sequence_cts
{
    using seq = boost::mp11::mp_repeat_c<std::tuple<traits::integral_constant<C>>, N>;

    template <typename T>
    struct converter {
    };

    template <typename... Cs>
    struct converter<std::tuple<Cs...>> {
        using type = compile_time_string<Cs::value...>;
    };

public:
    using type = typename converter<seq>::type;
};

/** Helper alias for create_sequence_cts.
 *
 * @tparam N Number of times to repeat @a C
 * @tparam C Character to repeat
 */
template <std::size_t N, char C>
using create_sequence_cts_t = typename create_sequence_cts<N, C>::type;

/** Converts the char integral constant array-like type @a T to a compile time string.
 *
 * @tparam T Char integral constant array-like type
 */
template <typename T>
struct convert_to_cts;

template <template <typename...> typename Array, typename... Cs>
struct convert_to_cts<Array<Cs...>> {
    using type = utility::compile_time_string<Cs::value...>;
};

/** Helper alias
 *
 * @tparam T Char integral constant array-like type
 */
template <typename T>
using convert_to_cts_t = typename convert_to_cts<T>::type;

/** Converts the integral @a Value to a compile time string.
 *
 * @tparam Value Integral to convert
 */
template <auto Value>
struct convert_integral_to_cts {
private:
    static_assert(std::is_integral_v<decltype(Value)>, "Value must be an integral");

    template <typename Str, auto NewValue>
    [[nodiscard]] constexpr static auto build() noexcept
    {
        constexpr auto num_digits = math::num_digits(NewValue);
        constexpr auto power10 = math::pow<10>(num_digits - 1);
        constexpr auto digit = NewValue / power10;
        [[maybe_unused]] constexpr auto next_value = NewValue % power10;

        using digit_str = typename Str::template append_t<compile_time_string<'0' + digit>>;

        if constexpr (num_digits != 1) {
            return build<digit_str, next_value>();
        } else {
            return digit_str{};
        }
    }

    using digit_str = decltype(build<compile_time_string<>, math::abs(Value)>());

public:
    using type = std::conditional_t<(Value < 0),
                                    compile_time_string<'-'>::template append_t<digit_str>,
                                    digit_str>;
};

/** Helper alias for convert_integral_to_cts.
 *
 * @tparam Value Integral to convert
 */
template <auto Value>
using convert_integral_to_cts_t = typename convert_integral_to_cts<Value>::type;

namespace cts_detail
{
template <int N>
// NOLINTNEXTLINE(*-avoid-c-arrays)
[[nodiscard]] constexpr char get(const char (&str)[N], std::size_t i) noexcept
{
    return i < N ? str[i] : '\0';
}

[[nodiscard]] constexpr char get(std::string_view str, std::size_t i) noexcept
{
    return i < str.size() ? str[i] : '\0';
}

[[nodiscard]] constexpr char get(char c, std::size_t i) noexcept
{
    return i < 1 ? c : '\0';
}

// Required so that the extra nulls S_ adds can be removed before defining the compile_time_string.
// Otherwise any compiler warnings you hit are just walls of null template args...
template <char... Cs>
struct builder {
    struct is_null_char {
        template <typename T>
        using fn = std::is_same<std::integral_constant<char, '\0'>, T>;
    };

    using strip_null =
        boost::mp11::mp_remove_if_q<boost::mp11::mp_list_c<char, Cs...>, is_null_char>;

    template <char... StrippedCs>
    [[nodiscard]] constexpr static auto list_to_string(
        [[maybe_unused]] boost::mp11::mp_list_c<char, StrippedCs...> chars) noexcept
    {
        return compile_time_string<StrippedCs...>{};
    }

    using type = decltype(list_to_string(strip_null{}));
};
}  // namespace cts_detail
}  // namespace arg_router::utility

#define AR_STR_CHAR(z, n, tok) arg_router::utility::cts_detail::get(tok, n)

#define AR_STR_N(n, tok) \
    typename arg_router::utility::cts_detail::builder<BOOST_PP_ENUM(n, AR_STR_CHAR, tok)>::type

/** Macro that represents the type of a compile-time string, useful for policies that require a
 * compile string.
 *
 * There is no requirement to use this, it just makes definitions easier to read.
 * @note The size limit is set by using the AR_MAX_CTS_SIZE define (defaults to 128).  Increasing
 * this will not increase the size of your program, but will increase build time as the preprocessor
 * and compiler have to do more work
 *
 * @param tok Can be a string literal, a <TT>std::string_view</TT>, or a single char
 */
#define S_(tok) AR_STR_N(AR_MAX_CTS_SIZE, tok)
