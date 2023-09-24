// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/parsing/parse_target.hpp"
#include "arg_router/parsing/parsing.hpp"
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

    /** Policy priority. */
    constexpr static auto priority = std::size_t{750};

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

    /** Performs the expansion in the pre-parse phase.
     *
     * Checks if the token's first character matches the owning node's short name.  If there isn't a
     * match or the owner does not have short name policy then it just returns false.  Otherwise all
     * the characters in the token are converted into short form tokens, added to @a tokens.
     *
     * @note If a short-form expander is used, the long and short prefixes must be different
     *
     * @tparam ProcessedTarget @a processed_target payload type
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param tokens Currently processed tokens
     * @param processed_target Previously processed parse_target of parent node, or empty is there
     * is no non-root parent
     * @param target Pre-parse generated target
     * @param parents Parent node instances
     * @return Always returns true because if the token doesn't match the short form name, the node
     * may have a long form one that does.  No exception is stored in the return value
     */
    template <typename ProcessedTarget, typename... Parents>
    [[nodiscard]] parsing::pre_parse_result pre_parse_phase(
        parsing::dynamic_token_adapter& tokens,
        [[maybe_unused]] utility::compile_time_optional<ProcessedTarget> processed_target,
        [[maybe_unused]] parsing::parse_target& target,
        [[maybe_unused]] const Parents&... parents) const
    {
        // The owner is named so there must be at least one token
        if (tokens.empty()) {
            return parsing::pre_parse_action::skip_node;
        }

        const auto token = *tokens.begin();
    }

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
