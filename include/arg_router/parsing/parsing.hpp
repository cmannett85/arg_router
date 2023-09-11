// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/algorithm.hpp"
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
using pre_parse_result = utility::result<pre_parse_action, multi_lang_exception>;

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
 * @note The error name is preferred over all others as this function is only used for exception
 * string generation.  If no known name method is detected, it is a compilation error
 * @tparam Node Node type
 * @return token_type
 */
template <typename Node>
[[nodiscard]] consteval token_type node_token_type() noexcept
{
    if constexpr (traits::has_error_name_method_v<Node>) {
        return {prefix_type::none, Node::error_name()};
    } else if constexpr (traits::has_display_name_method_v<Node>) {
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

/** Remove the leading entries from the node ancestry list that are derived from @a BaseNode.
 *
 * To allow derived types to call the inherited implementations of the pre-parse and parse stages,
 * we need to clean the multiple leading `*this` references that each parent method call will add.
 * Due to the use of policies we can @em almost guarantee that OOP parents will not be confused
 * with structural parents (i.e. parents in the tree).  It is an @em almost guarantee because
 * someone could create a custom node that allows for identical policies to it's tree parent and
 * then not define a validator rule to prevent this - but I'd consider that to have bigger
 * problems...
 *
 * This method is used in tree_node, so anything derived from that that uses its inherited pre-parse
 * and parse methods will not need to use this function.
 * @tparam BaseNode Base node type to be used to find derived types in @a derived_and_parents
 * @tparam DerivedAndParents Pack of (possibly) derived types in descending OOP ancestry order, and
 * then parent tree nodes in ascending ancestry order
 * @param base_node Base node instance
 * @param derived_and_parents Derived type and parent node instances
 * @return Tuple of cleaned const reference ancestors in <TT>std::reference_wrapper</TT>
 */
template <typename BaseNode, typename... DerivedAndParents>
[[nodiscard]] constexpr auto clean_node_ancestry_list(
    const BaseNode& base_node,
    const DerivedAndParents&... derived_and_parents)
{
    // Iterate along the ancestry until you hit a node type that is _not_ a base of Node, then
    // remove the elements up to that element
    using parents_tuple = std::tuple<std::decay_t<DerivedAndParents>...>;

    constexpr auto remove_n = boost::mp11::mp_find_if_q<
        parents_tuple,
        boost::mp11::mp_not_fn_q<
            boost::mp11::mp_bind<std::is_base_of, std::decay_t<BaseNode>, boost::mp11::_1>>>::value;
    if constexpr (remove_n == 0) {
        return std::tuple{std::cref(base_node), std::cref(derived_and_parents)...};
    } else {
        return algorithm::tuple_drop<remove_n - 1>(std::tuple{std::cref(derived_and_parents)...});
    }
}

/** Returns true if @a node or any of its @a parents are marked as runtime disabled.
 *
 * @tparam Node Current node type
 * @tparam Parents Parent nodes' type
 * @param node Current node instance
 * @param parents Parent nodes of @a node
 * @return True if the ancestry chain is effectively runtime disabled
 */
template <typename Node, typename... Parents>
[[nodiscard]] bool is_runtime_disabled(const Node& node, const Parents&... parents) noexcept
{
    auto any_disabled = false;
    utility::tuple_iterator(
        [&]([[maybe_unused]] auto i, auto entry) {
            using entry_type = typename std::decay_t<decltype(entry)>::type;

            if constexpr (traits::has_runtime_enabled_method_v<entry_type>) {
                any_disabled = any_disabled || !entry.get().runtime_enabled();
            }
        },
        std::tuple{std::cref(node), std::cref(parents)...});

    return any_disabled;
}
}  // namespace arg_router::parsing
