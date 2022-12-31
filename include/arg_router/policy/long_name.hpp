// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/policy/policy.hpp"

namespace arg_router::policy
{
/** Represents the long name of a node.
 *
 * If using C++17 then use the template variable helper with the <TT>S_</TT> macro; for C++20 and
 * higher, use the constructor directly with a compile-time string literal:
 * @code
 * constexpr auto a = ar::policy::long_name<S_("hello")>;
 * constexpr auto b = ar::policy::long_name_t{"hello"_S};
 * @endcode
 * @note Long names must be greater than one character and cannot contain any whitespace characters
 * @tparam S Compile-time string
 */
template <typename S>
class long_name_t
{
public:
    /** String type. */
    using string_type = S;

    /** Constructor.
     *
     * @param str String instance
     */
    constexpr explicit long_name_t([[maybe_unused]] S str = {}) noexcept {}

    /** Returns the name.
     *
     * @return Long name
     */
    [[nodiscard]] constexpr static std::string_view long_name() noexcept { return S::get(); }

private:
    static_assert(utility::utf8::count(long_name()) > 1,
                  "Long names must be longer than one character");
    static_assert(!utility::utf8::contains_whitespace(long_name()),
                  "Long names cannot contain whitespace");
};

/** Constant variable helper.
 *
 * @tparam S compile_time_string
 */
template <typename S>
constexpr auto long_name = long_name_t<S>{};

template <typename S>
struct is_policy<long_name_t<S>> : std::true_type {
};
}  // namespace arg_router::policy
