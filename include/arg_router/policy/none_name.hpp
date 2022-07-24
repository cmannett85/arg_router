/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/policy/policy.hpp"
#include "arg_router/utility/utf8.hpp"

namespace arg_router::policy
{
/** Represents the name of an argument that does not use any token prefix (i.e.
 * parsing::prefix_type == none).
 *
 * The only node that uses this in the library is mode_t.
 * @note Display names must not be empty
 * @tparam S compile_time_string
 */
template <typename S>
class none_name_t
{
public:
    /** String type. */
    using string_type = S;

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
 * @tparam S compile_time_string
 */
template <typename S>
constexpr auto none_name = none_name_t<S>{};

template <typename S>
struct is_policy<none_name_t<S>> : std::true_type {
};
}  // namespace arg_router::policy
