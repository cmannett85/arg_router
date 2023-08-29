// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/policy/policy.hpp"

namespace arg_router::policy
{
/** Represents the long name of a node.
 *
 * @code
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

template <typename S>
struct is_policy<long_name_t<S>> : std::true_type {
};
}  // namespace arg_router::policy
