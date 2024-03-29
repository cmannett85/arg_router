// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/policy/policy.hpp"

namespace arg_router::policy
{
/** Represents the name of a node that does not use any token prefix (i.e.
 * parsing::prefix_type == none).
 *
 * The only node that uses this in the library is mode_t.
 *
 * If using C++17 then use the template variable helper with the <TT>S_</TT> macro; for C++20 and
 * higher, use the constructor directly with a compile-time string literal:
 * @code
 * constexpr auto a = ar::policy::none_name<S_("hello")>;
 * constexpr auto b = ar::policy::none_name_t{"hello"_S};
 * @endcode
 * @note Display names must not be empty
 * @tparam S Compile-time string
 */
template <typename S>
class none_name_t
{
public:
    /** String type. */
    using string_type = S;

    /** Constructor.
     *
     * @param str String instance
     */
    constexpr explicit none_name_t([[maybe_unused]] S str = {}) noexcept {}

    /** Returns the name.
     *
     * @return None name
     */
    [[nodiscard]] constexpr static std::string_view none_name() noexcept { return S::get(); }

private:
    static_assert(utility::utf8::count(none_name()) > 1,
                  "None names must be longer than one character");
    static_assert(!utility::utf8::contains_whitespace(none_name()),
                  "None names cannot contain whitespace");
};

/** Constant variable helper.
 *
 * @tparam S Compile-time string
 */
template <typename S>
constexpr auto none_name = none_name_t<S>{};

template <typename S>
struct is_policy<none_name_t<S>> : std::true_type {
};
}  // namespace arg_router::policy
