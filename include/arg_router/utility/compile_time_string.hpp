// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/math.hpp"

#include <algorithm>
#include <array>
#include <span>
#include <string_view>

namespace arg_router
{
namespace utility
{
namespace detail
{
template <std::size_t N>
class compile_time_string_storage
{
public:
    constexpr compile_time_string_storage() = default;

    // We need all the constructors to support implicit conversion, because that's kind of the
    // point of this class...
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    constexpr compile_time_string_storage(std::array<char, N> str) : value(str) {}

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    constexpr compile_time_string_storage(std::span<const char, N> str)
    {
        std::copy(str.begin(), str.end(), value.begin());
    }

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions,*-c-arrays)
    constexpr compile_time_string_storage(const char (&str)[N])
    {
        std::copy_n(&str[0], N, value.begin());
    }

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    constexpr compile_time_string_storage(char c) : value{c} {}

    std::array<char, N> value = {};
};

compile_time_string_storage()->compile_time_string_storage<0>;
compile_time_string_storage(char)->compile_time_string_storage<1>;
}  // namespace detail

/** Compile-time string.
 *
 * There is a string literal operator available in arg_router::literals, that allows use like this:
 * @code
 * using str_t = utility::str<"hello">;
 *
 * using namespace arg_router::literals;
 * constexpr auto str = "hello"_S;
 * @endcode
 *
 * @note This cannot use a <TT>std::string_view</TT> as a storage input type, use
 * <TT>std::array<char, N></TT> or <TT>std::span<char, N></TT> instead
 *
 * @tparam S Internal storage type
 */
template <detail::compile_time_string_storage S = detail::compile_time_string_storage<0>{}>
class str
{
public:
    /** Number of characters in string.
     *
     * @return String size
     */
    [[nodiscard]] constexpr static std::size_t size() noexcept { return size_; }

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
        return {S.value.data(), size()};
    }

    /** Concatentation operator.
     *
     * @tparam S2 Compile-time string from other instance
     * @param other Instance to concatenate
     * @return New instance
     */
    template <detail::compile_time_string_storage S2>
    [[nodiscard]] constexpr auto operator+([[maybe_unused]] const str<S2>& other) const noexcept
    {
        return []<std::size_t... LIs, std::size_t... RIs>(std::index_sequence<LIs...>,
                                                          std::index_sequence<RIs...>)
        {
            return str<std::array{S.value[LIs]..., S2.value[RIs]..., '\0'}>{};
        }
        (std::make_index_sequence<size()>{}, std::make_index_sequence<str<S2>::size()>{});
    }

    /** Returns a substring of this type, consisting of @a Count characters starting at @a Pos.
     *
     * It is a compile-time failure if @a Pos + @a Count is greater than or equal to size().
     * @tparam Pos Start index
     * @tparam Count Number of characters to use
     * @return Substring
     */
    template <std::size_t Pos, std::size_t Count>
    [[nodiscard]] constexpr static auto substr() noexcept
    {
        static_assert((Pos + Count) < size_, "Pos+Count must be less than string size");

        return substr_impl<Pos>(std::make_index_sequence<Count>{});
    }

    /** Appends @a T to this string type.
     *
     * @tparam T String to append
     */
    template <typename T>
    struct append;

    template <detail::compile_time_string_storage S2>
    struct append<str<S2>> {
        using type = std::decay_t<decltype(str{} + str<S2>{})>;
    };

    /** Helper alias for append.
     *
     * @tparam T String to append
     */
    template <typename T>
    using append_t = typename append<T>::type;

private:
    // This is a necessary evil, as we can't use std::char_traits<char>::length(..) in a
    // constant expression
    constexpr static std::size_t calculate_size() noexcept
    {
        auto count = std::size_t{0};
        for (; count < S.value.size(); ++count) {
            if (S.value[count] == '\0') {
                break;
            }
        }
        return count;
    }

    template <std::size_t Pos, std::size_t... Is>
    constexpr static auto substr_impl([[maybe_unused]] std::index_sequence<Is...> seq)
    {
        return str<std::array{S.value[Pos + Is]..., '\0'}>{};
    }

    constexpr static std::size_t size_ = calculate_size();
};

/** TMP helper for concatenating two compile-time strings together.
 *
 * @tparam S1 Prefix
 * @tparam S2 Suffix
 */
template <typename S1, typename S2>
using str_concat = typename S1::template append_t<S2>;

/** Provides a compile time string that is a repeating sequence of @a C @a N characters long.
 *
 * @tparam N Number of times to repeat @a C
 * @tparam C Character to repeat
 */
template <std::size_t N, char C>
class create_sequence_cts
{
    template <std::size_t>
    [[nodiscard]] constexpr static char get() noexcept
    {
        return C;
    }

    template <std::size_t... Is>
    [[nodiscard]] constexpr static std::array<char, N> builder(
        [[maybe_unused]] std::index_sequence<Is...> seq) noexcept
    {
        return {get<Is>()...};
    }

public:
    using type = str<builder(std::make_index_sequence<N>{})>;
};

/** Helper alias for create_sequence_cts.
 *
 * @tparam N Number of times to repeat @a C
 * @tparam C Character to repeat
 */
template <std::size_t N, char C>
using create_sequence_cts_t = typename create_sequence_cts<N, C>::type;

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

        using digit_str = str<'0' + digit>;

        if constexpr (num_digits != 1) {
            constexpr auto next_value = NewValue % power10;
            return build<typename Str::template append_t<digit_str>, next_value>();
        } else {
            return typename Str::template append_t<digit_str>{};
        }
    }

    using digit_str = decltype(build<str<>, math::abs(Value)>());

public:
    using type = std::conditional_t<(Value < 0), str<'-'>::template append_t<digit_str>, digit_str>;
};

/** Helper alias for convert_integral_to_cts.
 *
 * @tparam Value Integral to convert
 */
template <auto Value>
using convert_integral_to_cts_t = typename convert_integral_to_cts<Value>::type;
}  // namespace utility

/** Alias to make the compile-time string type available in the top-level namespace.
 *
 * @tparam S Internal storage type
 */
template <utility::detail::compile_time_string_storage S>
using str = utility::str<S>;

namespace traits
{
/** Evaluates to true if @a T is a compile-time string-like type.
 *
 * @tparam T Type to test
 */
template <typename T>
struct is_compile_time_string_like : std::false_type {
};

template <auto S>
struct is_compile_time_string_like<utility::str<S>> : std::true_type {
};

/** Helper variable for is_compile_time_string_like.
 *
 * @tparam T Type to test
 */
template <typename T>
constexpr bool is_compile_time_string_like_v = is_compile_time_string_like<T>::value;
}  // namespace traits
}  // namespace arg_router

#define AR_STRING_SV(tok) arg_router::utility::str<std::span<const char, tok.size()>{tok}>
