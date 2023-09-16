// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/config.hpp"
#include "arg_router/traits.hpp"
#include "arg_router/utility/string_view_ops.hpp"

namespace arg_router::parsing
{
/** Enum for the prefix type on a token. */
enum class prefix_type : std::uint8_t {
    long_,   /// Long prefix
    short_,  /// Short prefix
    none     /// No prefix
};

/** Creates a string version of @a prefix.
 *
 * This uses config::long_prefix and config::short_prefix.
 * @param prefix Prefix type to convert
 * @return String version of @a prefix
 */
[[nodiscard]] constexpr std::string_view to_string(prefix_type prefix) noexcept
{
    switch (prefix) {
    case prefix_type::long_: return config::long_prefix;
    case prefix_type::short_: return config::short_prefix;
    default: return "";
    }
}

/** Pair-like structure carrying the token's prefix type and the token itself (stripped of prefix).
 */
struct token_type {
    /** Long form name constructor.
     *
     * @param p Prefix type
     * @param n Token name, stripped of prefix (if any)
     */
    constexpr token_type(prefix_type p, std::string_view n) noexcept : prefix{p}, name{n} {}

    /** Equality operator.
     *
     * @param other Instance to compare against
     * @return True if equal
     */
    [[nodiscard]] constexpr bool operator==(const token_type& other) const noexcept
    {
        return prefix == other.prefix && name == other.name;
    }

    /** Inequality operator.
     *
     * @param other Instance to compare against
     * @return True if not equal
     */
    [[nodiscard]] constexpr bool operator!=(const token_type& other) const noexcept
    {
        return !(*this == other);
    }

    prefix_type prefix;     ///< Prefix type
    std::string_view name;  ///< Token name, stripped of prefix (if any)
};

/** Creates a string representation of @a token, it effectively recreates the original token on the
 * command line.
 *
 * @param token Token to convert
 * @return String representation of @a token
 */
[[nodiscard]] inline std::string to_string(const token_type& token)
{
    using namespace utility::string_view_ops;
    return to_string(token.prefix) + token.name;
}

/** Creates a string representation of @a view.
 *
 * @param view Processed tokens to convert
 * @return String representation of @a view
 */
[[nodiscard]] std::string to_string(const std::vector<token_type>& view);

/** Analyse @a token and return a pair consisting of the prefix type and @a token stripped of the
 * token.
 *
 * @param token Token to analyse
 * @return Token type
 */
[[nodiscard]] token_type get_token_type(std::string_view token);

/** Overload that uses the naming policies of @a node to control the output.
 *
 * If the target node is available, this should be the preferred overload.
 * @tparam Node Target node type
 * @param node Node instance
 * @param token Token to analyse
 * @return Token type
 */
template <typename Node>
[[nodiscard]] token_type get_token_type([[maybe_unused]] const Node& node, std::string_view token)
{
    using namespace config;

    if constexpr (traits::has_long_name_method_v<Node>) {
        if (token.substr(0, long_prefix.size()) == long_prefix) {
            token.remove_prefix(long_prefix.size());
            return {prefix_type::long_, token};
        }
    }
    if constexpr (traits::has_short_name_method_v<Node>) {
        if ((token.substr(0, short_prefix.size()) == short_prefix) &&
            (token.substr(0, long_prefix.size()) != long_prefix)) {
            token.remove_prefix(short_prefix.size());
            return {prefix_type::short_, token};
        }
    }
    return {prefix_type::none, token};
}
}  // namespace arg_router::parsing
