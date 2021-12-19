#pragma once

#include "arg_router/token_type.hpp"
#include "arg_router/traits.hpp"

namespace arg_router
{
/** Namespace containing types and functions to aid parsing. */
namespace parsing
{
/** Takes the main function arguments and creates a token_list from it.
 *
 * This function will ignore the first argument (program name) and expand out
 * any collapsed short form arguments.
 * @param argc Argument count
 * @param argv Argument string array
 * @return Token list
 */
token_list expand_arguments(int argc, const char* argv[]);

/** The standard implementation of the match method.
 *
 * @tparam T Type to implement the method for
 * @param token The token to match against
 * @return True if token matches the long or short form name
 */
template <typename T>
bool default_match(const token_type& token)
{
    if constexpr (traits::has_long_name_method_v<T>) {
        if ((token.prefix == prefix_type::LONG ||
             token.prefix == prefix_type::NONE) &&
            (token.name == T::long_name())) {
            return true;
        }
    }
    if constexpr (traits::has_short_name_method_v<T>) {
        if ((token.prefix == prefix_type::SHORT) &&
            (token.name == T::short_name())) {
            return true;
        }
    }

    return false;
}

/** Returns the token_type of @a Node, the long form name is preferred.
 *
 * @note If @a Node does not have a long or short name, it is a compliation
 * failure
 * @tparam Node Node type
 * @return token_type
 */
template <typename Node>
token_type node_token_type()
{
    if constexpr (traits::has_long_name_method_v<Node>) {
        return {prefix_type::LONG, Node::long_name()};
    } else if constexpr (traits::has_short_name_method_v<Node>) {
        return {prefix_type::SHORT, Node::short_name()};
    } else {
        static_assert(traits::always_false_v<Node>,
                      "Node does not have a name");
    }
}
}  // namespace parsing
}  // namespace arg_router
