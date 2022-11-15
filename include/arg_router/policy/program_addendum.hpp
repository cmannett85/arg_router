// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/policy/policy.hpp"

namespace arg_router::policy
{
/** Represents the program addendum.
 *
 * Used by help nodes to display supplementary information (usually) after the argument output.
 *
 * If using C++17 then use the template variable helper with the <TT>S_</TT> macro; for C++20 and
 * higher, use the constructor directly with a compile-time string literal:
 * @code
 * constexpr auto a = ar::policy::program_addendum<S_("hello")>;
 * constexpr auto b = ar::policy::program_addendum_t{"hello"_S};
 * @endcode
 * @note Must be greater than one character
 * @tparam S Compile-time string
 */
template <typename S>
class program_addendum_t
{
public:
    /** String type. */
    using string_type = S;

    /** Constructor.
     *
     * @param str String instance
     */
    constexpr explicit program_addendum_t([[maybe_unused]] S str = {}) noexcept {}

    /** Returns the program name.
     *
     * @return Program name
     */
    [[nodiscard]] constexpr static std::string_view program_addendum() noexcept { return S::get(); }

private:
    static_assert(utility::utf8::count(program_addendum()) > 1,
                  "Program addendum must be longer than one character");
};

/** Constant variable helper.
 *
 * @tparam S Compile-time string
 */
template <typename S>
constexpr auto program_addendum = program_addendum_t<S>{};

template <typename S>
struct is_policy<program_addendum_t<S>> : std::true_type {
};
}  // namespace arg_router::policy
