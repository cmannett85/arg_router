/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/basic_types.hpp"
#include "arg_router/exception.hpp"
#include "arg_router/policy/multi_stage_value.hpp"
#include "arg_router/traits.hpp"
#include "arg_router/utility/string_view_ops.hpp"
#include "arg_router/utility/tuple_iterator.hpp"

#include <bitset>

namespace arg_router
{
/** Namespace containing types and functions to aid parsing. */
namespace parsing
{
namespace detail
{
template <typename Node>
[[nodiscard]] constexpr std::optional<token_type> handle_value_separator(
    parsing::token_type token) noexcept
{
    auto result = std::optional<token_type>{};

    constexpr auto split_char = Node::value_separator();
    const auto index = token.name.find_first_of(split_char);
    if (index != std::string_view::npos) {
        result = {token.prefix, token.name.substr(0, index)};
    }

    return result;
}

template <typename Node, typename Fn, std::size_t N>
[[nodiscard]] constexpr bool find(const Node& node,
                                  parsing::token_type token,
                                  std::bitset<N> hit_flags,
                                  const Fn& visitor)
{
    auto result = false;
    utility::tuple_iterator(
        [&](auto i, const auto& node) {
            if (result || hit_flags[i]) {
                return;
            }

            // Wrap the caller's visitor with one that forwards the child
            // index
            const auto wrapped_visitor = [&](const auto& child) {
                visitor(i, child);
            };

            if (node.match(token, wrapped_visitor)) {
                result = true;
            }
        },
        node.children());

    return result;
}

template <typename Node>
void expand_arguments(const Node& node,
                      span<const char*>& args,
                      token_list& result)
{
    using namespace std::string_literals;
    using namespace utility::string_view_ops;

    using hit_flags_type =
        std::bitset<std::tuple_size_v<typename Node::children_type>>;

    auto hit_flags = hit_flags_type{};
    while (!args.empty()) {
        // Convert to a token_type and try to find the matching node
        const auto tt = get_token_type(args.front());

        // Expand out if it's collapsed short-form flags, this is valid because
        // no value tokens are at the start of token processing chain - unless
        // this is for positional arguments but that's user ambiguity which we
        // can't eliminate (it's not psychic)
        if (tt.prefix == prefix_type::SHORT && tt.name.size() > 1u) {
            // Expand it out
            for (auto i = 0u; i < tt.name.size(); ++i) {
                result.emplace_pending(prefix_type::SHORT,
                                       std::string_view{&(tt.name[i]), 1});
            }
            args = args.subspan(1);
            continue;
        }

        const auto consume = [&]() {
            result.add_pending(tt);
            args = args.subspan(1);
        };

        const auto found_node =
            find(node, tt, hit_flags, [&](auto i, const auto& child) {
                // We have a match, but is it just a positional_arg absorbing
                // everything?  Check by looking at it's name policies
                using child_type = std::decay_t<decltype(child)>;

                constexpr auto is_named =
                    traits::has_long_name_method_v<child_type> ||
                    traits::has_short_name_method_v<child_type> ||
                    traits::has_none_name_method_v<child_type>;

                // Don't revisit nodes, this prevents the first positional_arg
                // from consuming everything
                hit_flags[i] = !policy::has_multi_stage_value_v<child_type>;

                if (tt.prefix == prefix_type::LONG) {
                    if constexpr (traits::has_value_separator_method_v<
                                      child_type>) {
                        const auto split_token =
                            detail::handle_value_separator<child_type>(tt);
                        if (!split_token) {
                            throw parse_exception{
                                "Expected to find value separator '"s +
                                child_type::value_separator() + "' in \"" +
                                args[0] + "\""};
                        }
                        result.add_pending(*split_token);

                        // Update the args entry to point just to the value part
                        const auto offset =
                            split_token->name.size() +
                            child_type::value_separator().size();
                        args[0] +=
                            to_string(split_token->prefix).size() + offset;

                        // Someone has put the value separator but then no value
                        // after it...
                        if (offset == tt.name.size()) {
                            throw parse_exception{
                                "Unable to find value after separator",
                                tt};
                        }
                    } else if constexpr (is_named) {
                        // It's not a positional_arg, so add the name token
                        consume();
                    }
                } else if (is_named) {
                    // It's not a positional_arg, so add the name token
                    consume();
                }

                // Using the count of the node, add the correct number of NONE
                // (i.e. value) tokens to the list
                if constexpr (traits::has_maximum_count_method_v<child_type>) {
                    const auto value_count =
                        std::min(child_type::maximum_count(), args.size());
                    if (value_count > 0u) {
                        result.reserve(result.pending_view().size() +
                                       value_count);
                        for (auto i = 0u; i < value_count; ++i) {
                            result.emplace_pending(prefix_type::NONE, args[i]);
                        }
                        args = args.subspan(value_count);
                    }
                } else if constexpr (traits::has_process_value_tokens_method_v<
                                         child_type>) {
                    child_type::process_value_tokens(args, result);
                }

                // Recurse using the found node if necessary
                if constexpr ((std::tuple_size_v<
                                  typename child_type::children_type>) > 0) {
                    if (!args.empty()) {
                        expand_arguments(child, args, result);
                    }
                }
            });

        // If we can't find the node, just add it as our best guess. We do not
        // error here as the purpose of this parsing phase is to categorise as
        // best we can the tokens, better error checking happens later
        if (!found_node) {
            consume();
        }
    }
}
}  // namespace detail

/** Takes the main function arguments and creates a token_list from it.
 *
 * Collapsed short names are expanded.
 * @tparam Root Root node type
 * @param root Root instance
 * @param argc Argument count
 * @param argv Argument string array
 * @return Token list
 */
template <typename Root>
[[nodiscard]] token_list expand_arguments(const Root& root,
                                          int argc,
                                          char* argv[])
{
    auto args = span<const char*>(const_cast<const char**>(&argv[1]), argc - 1);
    auto result = token_list{};

    // Arguments must be brute forced as we don't know at the this stage if they
    // are args, flags, or values for a positional_arg - not even the prefix is
    // proof!
    detail::expand_arguments(root, args, result);

    return result;
}

/** The standard implementation of the match method.
 *
 * @tparam T Type to implement the method for
 * @param token The token to match against
 * @return True if token matches the long or short form name
 */
template <typename T>
[[nodiscard]] constexpr bool default_match(token_type token) noexcept
{
    using namespace std::string_literals;

    if constexpr (traits::has_long_name_method_v<T>) {
        if (token.prefix == prefix_type::LONG) {
            if constexpr (traits::has_value_separator_method_v<T>) {
                const auto split_token =
                    detail::handle_value_separator<T>(token);
                if (split_token) {
                    token = *split_token;
                }
            }

            if (token.name == T::long_name()) {
                return true;
            }
        }
    }
    if constexpr (traits::has_short_name_method_v<T>) {
        if ((token.prefix == prefix_type::SHORT) &&
            (token.name == T::short_name())) {
            return true;
        }
    }
    if constexpr (traits::has_none_name_method_v<T>) {
        if ((token.prefix == prefix_type::NONE) &&
            (token.name == T::none_name())) {
            return true;
        }
    }

    return false;
}

/** Returns the token_type of @a Node, the long form name is preferred if
 * @a Node has both short and long form names.
 *
 * @note If @a Node does not have a display, none, long, or short name; it is a
 * compliation failure
 * @tparam Node Node type
 * @return token_type
 */
template <typename Node>
[[nodiscard]] constexpr token_type node_token_type() noexcept
{
    if constexpr (traits::has_display_name_method_v<Node>) {
        return {prefix_type::NONE, Node::display_name()};
    } else if constexpr (traits::has_long_name_method_v<Node>) {
        return {prefix_type::LONG, Node::long_name()};
    } else if constexpr (traits::has_short_name_method_v<Node>) {
        return {prefix_type::SHORT, Node::short_name()};
    } else if constexpr (traits::has_none_name_method_v<Node>) {
        return {prefix_type::NONE, Node::none_name()};
    } else {
        static_assert(traits::always_false_v<Node>,
                      "Node does not have a name");
    }
}
}  // namespace parsing
}  // namespace arg_router
