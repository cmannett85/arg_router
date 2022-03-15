/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/algorithm.hpp"
#include "arg_router/policy/policy.hpp"

namespace arg_router
{
namespace policy
{
/** Represents the character that separates a label token from its value
 * token(s).
 *
 * Your terminal will take of separating tokens using whitespace by default, but
 * often a different character is used e.g. <TT>--arg=42</TT> - this policy
 * specifies that.
 * @note This is only applied to long form name tokens in root_t
 * @tparam S Integral constant that can be implicitly converted to a char
 */
template <typename S>
class value_separator_t
{
    static_assert(
        std::is_convertible_v<S, char>,
        "Value separator type must be implicitly convertible to char");
    static_assert(!algorithm::is_whitespace(S::value),
                  "Value separator character must not be whitespace");

    constexpr static auto value = S::value;

public:
    /** String type. */
    using string_type = S;

    /** Returns the separator.
     *
     * @return Separator character
     */
    [[nodiscard]] constexpr static std::string_view value_separator() noexcept
    {
        return std::string_view{&value, 1};
    }
};

/** Constant variable helper.
 *
 * @tparam S Arg/value separator character
 */
template <char S>
constexpr auto value_separator =
    value_separator_t<traits::integral_constant<S>>{};

template <typename S>
struct is_policy<value_separator_t<S>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
