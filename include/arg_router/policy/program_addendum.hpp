/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/policy/policy.hpp"
#include "arg_router/utility/utf8.hpp"

namespace arg_router::policy
{
/** Represents the program addendum.
 *
 * Used by help nodes to display supplementary information (usually) after the argument output.
 * @note Must be greater than one character
 * @tparam S compile_time_string
 */
template <typename S>
class program_addendum_t
{
public:
    /** String type. */
    using string_type = S;

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
 * @tparam S compile_time_string
 */
template <typename S>
constexpr auto program_addendum = program_addendum_t<S>{};

template <typename S>
struct is_policy<program_addendum_t<S>> : std::true_type {
};
}  // namespace arg_router::policy
