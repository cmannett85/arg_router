// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/policy/policy.hpp"
#include "arg_router/utility/utf8.hpp"

namespace arg_router::policy
{
/** Represents the long name of an argument.
 *
 * @note Long names must be greater than one character and cannot contain any whitespace characters
 * @tparam S compile_time_string
 */
template <typename S>
class long_name_t
{
public:
    /** String type. */
    using string_type = S;

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
