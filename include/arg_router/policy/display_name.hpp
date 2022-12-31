// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/policy/policy.hpp"

namespace arg_router::policy
{
/** Represents the display name of a node.
 *
 * A display name is a label given to a node that appears in the help output, but is not used in the
 * token parsing.
 *
 * In the default validator, this policy is not allowed to be used with long_name and short_name -
 * as we shouldn't be trying to confuse the user...
 *
 * If using C++17 then use the template variable helper with the <TT>S_</TT> macro; for C++20 and
 * higher, use the constructor directly with a compile-time string literal:
 * @code
 * constexpr auto a = ar::policy::display_name<S_("hello")>;
 * constexpr auto b = ar::policy::display_name_t{"hello"_S};
 * @endcode
 * @note Display names must not be empty
 * @tparam S Compile-time string
 */
template <typename S>
class display_name_t
{
public:
    /** String type. */
    using string_type = S;

    /** Constructor.
     *
     * @param str String instance
     */
    constexpr explicit display_name_t([[maybe_unused]] S str = {}) noexcept {}

    /** Returns the name.
     *
     * @return Display name
     */
    [[nodiscard]] constexpr static std::string_view display_name() noexcept { return S::get(); }

private:
    static_assert(!display_name().empty(), "Display name must not be empty");
};

/** Constant variable helper.
 *
 * @tparam S Compile-time string
 */
template <typename S>
constexpr auto display_name = display_name_t<S>{};

template <typename S>
struct is_policy<display_name_t<S>> : std::true_type {
};
}  // namespace arg_router::policy
