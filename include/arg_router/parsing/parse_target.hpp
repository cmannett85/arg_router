/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/parsing/token_type.hpp"
#include "arg_router/tree_node_fwd.hpp"
#include "arg_router/utility/type_hash.hpp"
#include "arg_router/utility/unsafe_any.hpp"

#include <functional>

namespace arg_router::parsing
{
/** A parse target i.e. a target node optionally with tokens for parsing.
 *
 * This type is the result of a pre-parse phase, and is used to trigger a parse of the given tokens
 * (optional) on the target node.  Sub-targets can also be attached allowing a node to trigger the
 * parse of other nodes e.g. mode-like types.
 *
 * The target can only be invoked once, invoking a second or more time is a no-op.
 */
class parse_target
{
public:
    /** Constructor.
     *
     * @tparam Node Target node type
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param tokens Tokens to be parsed by @a node, not including any label
     * token
     * @param node Target node instance
     * @param parents Parents of @a node
     */
    template <typename Node, typename... Parents>
    parse_target(vector<token_type> tokens, const Node& node, const Parents&... parents) noexcept :
        node_type_{utility::type_hash<std::decay_t<Node>>()}, tokens_(std::move(tokens))
    {
        static_assert(is_tree_node_v<Node>, "Target must be a tree_node");

        parse_ = [&node, &parents...](parse_target target) -> utility::unsafe_any {
            constexpr bool no_parse_value =
                std::is_void_v<decltype(node.parse(std::move(target), parents...))>;

            if constexpr (no_parse_value) {
                node.parse(std::move(target), parents...);
                return {};
            } else {
                return node.parse(std::move(target), parents...);
            }
        };
    }

    /** No token constructor.
     *
     * @tparam Node Target node type
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param node Target node instance
     * @param parents Parents of @a node
     */
    template <typename Node, typename... Parents>
    // https://github.com/llvm/llvm-project/issues/37250
    // NOLINTNEXTLINE(*-member-init)
    explicit parse_target(const Node& node,  //
                          const Parents&... parents) noexcept :
        parse_target(vector<token_type>{}, node, parents...)
    {
    }

    /** The tokens associated with this target.
     *
     * @return Tokens reference
     */
    [[nodiscard]] vector<token_type>& tokens() noexcept { return tokens_; }

    /** Const overload.
     *
     * @return Tokens reference
     */
    [[nodiscard]] const vector<token_type>& tokens() const noexcept { return tokens_; }

    /** The sub-targets associated with this target.
     *
     * @return Sub-targets reference
     */
    [[nodiscard]] vector<parse_target>& sub_targets() noexcept { return sub_targets_; }

    /** Const overload.
     *
     * @return Sub-targets reference
     */
    [[nodiscard]] const vector<parse_target>& sub_targets() const noexcept { return sub_targets_; }

    /** Bool conversion operator.
     *
     * @return True if target is invocable (i.e. trigger a parse)
     */
    [[nodiscard]] explicit operator bool() const noexcept { return static_cast<bool>(parse_); }

    /** Returns the hash code for the target node.
     *
     * @return Target node hash code
     */
    [[nodiscard]] std::size_t node_type() const noexcept { return node_type_; }

    /** Append a sub-target.
     *
     * The tokens of @a target are appended to this target.
     * @param target Sub-target
     */
    void add_sub_target(parse_target target) { sub_targets_.push_back(std::move(target)); }

    /** Set the tokens for this node.
     *
     * @param tokens New tokens
     */
    void tokens(vector<token_type> tokens) { tokens_ = std::move(tokens); }

    /** Trigger the parse of this target.
     *
     * @return Parse result or empty
     */
    utility::unsafe_any operator()()
    {
        if (parse_) {
            auto parse = std::move(parse_);
            parse_ = decltype(parse_){};
            return parse(std::move(*this));
        }

        return {};
    }

private:
    std::size_t node_type_;
    vector<token_type> tokens_;
    vector<parse_target> sub_targets_;
    std::function<utility::unsafe_any(parse_target)> parse_;
};
}  // namespace arg_router::parsing
