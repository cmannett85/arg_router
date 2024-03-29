// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/policy/policy.hpp"

namespace arg_router::policy
{
/** Represents the program name.
 *
 * Used by help nodes to produce their output, though in principle can be used by anything that
 * wants to.
 *
 * If using C++17 then use the template variable helper with the <TT>S_</TT> macro; for C++20 and
 * higher, use the constructor directly with a compile-time string literal:
 * @code
 * constexpr auto a = ar::policy::program_name<S_("hello")>;
 * constexpr auto b = ar::policy::program_name_t{"hello"_S};
 * @endcode
 * @note Names must be greater than one character and cannot contain any whitespace characters
 * @tparam S Compile-time string
 */
template <typename S>
class program_name_t
{
public:
    /** String type. */
    using string_type = S;

    /** Constructor.
     *
     * @param str String instance
     */
    constexpr explicit program_name_t([[maybe_unused]] S str = {}) noexcept {}

    /** Returns the program name.
     *
     * @return Program name
     */
    [[nodiscard]] constexpr static std::string_view program_name() noexcept { return S::get(); }

private:
    static_assert(utility::utf8::count(program_name()) > 1,
                  "Program names must be longer than one character");
    static_assert(!utility::utf8::contains_whitespace(program_name()),
                  "Program names cannot contain whitespace");
};

/** Constant variable helper.
 *
 * @tparam S Compile-time string
 */
template <typename S>
constexpr auto program_name = program_name_t<S>{};

template <typename S>
struct is_policy<program_name_t<S>> : std::true_type {
};
}  // namespace arg_router::policy
