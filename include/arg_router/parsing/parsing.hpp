/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/traits.hpp"
#include "arg_router/utility/result.hpp"

/** Namespace containing types and functions to aid parsing. */
namespace arg_router::parsing
{
/** Owning nodes requested action by a pre-parse implementing policy. */
enum class pre_parse_action : std::uint8_t {
    /** This policy is not applicable to the tokens */
    skip_node,
    /** This policy is applicable to the tokens */
    valid_node,
    /** Same as skip_node, but any changes to the given tokens should be made permanent as long as
     * no later policy returns skip_node
     */
    skip_node_but_use_sub_targets
};

/** Policy-level pre-parse phase result type. */
using pre_parse_result = utility::result<pre_parse_action, parse_exception>;

/** Matches @a token to @a T by comparing the token against the long, short, or none name traits.
 *
 * @tparam T Type providing the long, short, or none name methods
 * @param token The token to match against
 * @return True if token matches
 */
template <typename T>
[[nodiscard]] constexpr bool match(token_type token) noexcept
{
    using namespace std::string_literals;

    if constexpr (traits::has_long_name_method_v<T>) {
        if ((token.prefix == prefix_type::long_) && (token.name == T::long_name())) {
            return true;
        }
    }
    if constexpr (traits::has_short_name_method_v<T>) {
        if ((token.prefix == prefix_type::short_) && (token.name == T::short_name())) {
            return true;
        }
    }
    if constexpr (traits::has_none_name_method_v<T>) {
        if ((token.prefix == prefix_type::none) && (token.name == T::none_name())) {
            return true;
        }
    }

    return false;
}

/** Returns the token_type of @a Node, the long form name is preferred if @a Node has both short and
 * long form names.
 *
 * @note If @a Node does not have a display, none, long, or short name; it is a compliation failure
 * @tparam Node Node type
 * @return token_type
 */
template <typename Node>
[[nodiscard]] constexpr token_type node_token_type() noexcept
{
    if constexpr (traits::has_display_name_method_v<Node>) {
        return {prefix_type::none, Node::display_name()};
    } else if constexpr (traits::has_long_name_method_v<Node>) {
        return {prefix_type::long_, Node::long_name()};
    } else if constexpr (traits::has_short_name_method_v<Node>) {
        return {prefix_type::short_, Node::short_name()};
    } else if constexpr (traits::has_none_name_method_v<Node>) {
        return {prefix_type::none, Node::none_name()};
    } else {
        static_assert(traits::always_false_v<Node>, "Node does not have a name");
    }
}
}  // namespace arg_router::parsing
