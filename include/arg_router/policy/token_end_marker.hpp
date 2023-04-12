// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/parsing/parsing.hpp"
#include "arg_router/policy/multi_stage_value.hpp"

namespace arg_router::policy
{
/** Represents the token in a variable length value list that marks the end of the list at runtime.
 *
 * Typically a variable length value list (e.g. the value tokens for a positional_arg_t) sits at the
 * end of the input tokens as the corresponding node will consume the tokens until the maximum
 * count or token list end is reached.  This policy can adjust that behaviour by defining a token
 * that marks the end of list.  This allows multiple variable length value list nodes to be used
 * under a single mode.
 *
 * If using C++17 then use the template variable helper with the <TT>S_</TT> macro; for C++20 and
 * higher, use the constructor directly with a compile-time string literal:
 * @code
 * constexpr auto a = ar::policy::token_end_marker<S_("hello")>;
 * constexpr auto b = ar::policy::token_end_marker_t{"hello"_S};
 * @endcode
 * @note Token must have at least one character and cannot contain any whitespace characters
 * @tparam S Compile-time string
 */
template <typename S>
class token_end_marker_t
{
public:
    /** String type. */
    using string_type = S;

    /** Policy priority. */
    constexpr static auto priority = std::size_t{760};

    /** Constructor.
     *
     * @param str String instance
     */
    constexpr explicit token_end_marker_t([[maybe_unused]] S str = {}) noexcept {}

    /** Returns the token end marker.
     *
     * @return Token end marker
     */
    [[nodiscard]] constexpr static std::string_view token_end_marker() noexcept { return S::get(); }

    /** Checks that the owner expects a variable length list of token values. This policy does not
     * perform any operation on the input tokens.
     *
     * @tparam ProcessedTarget @a processed_target payload type
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param tokens Currently processed tokens
     * @param processed_target Previously processed parse_target of parent node, or empty is there
     * is no non-root parent
     * @param target Pre-parse generated target
     * @param parents Parent node instances
     * @return Always returns parsing::pre_parse_action::valid_node
     */
    template <typename ProcessedTarget, typename... Parents>
    [[nodiscard]] parsing::pre_parse_result pre_parse_phase(
        parsing::dynamic_token_adapter& tokens,
        [[maybe_unused]] utility::compile_time_optional<ProcessedTarget> processed_target,
        [[maybe_unused]] parsing::parse_target& target,
        [[maybe_unused]] const Parents&... parents) const
    {
        static_assert((sizeof...(Parents) >= 1),
                      "At least one parent needed for token_end_marker_t");

        using owner_type = boost::mp11::mp_first<std::tuple<Parents...>>;

        constexpr auto has_count = traits::has_minimum_count_method_v<owner_type> &&
                                   traits::has_maximum_count_method_v<owner_type>;
        static_assert(has_count, "Token end marker can only be used in variable list length nodes");

        if constexpr (has_count) {
            static_assert((owner_type::minimum_count() != owner_type::maximum_count()) &&
                              !policy::has_multi_stage_value_v<owner_type>,
                          "Token end marker can only be used in variable list length nodes");
        }

        // Transfer the tokens up to the end marker.  If not found then the whole token list is
        // used.  If found, the token is removed as it should be in the parsed results
        const auto it = std::find_if(tokens.begin(), tokens.end(), [](auto&& token) {
            return token.name == token_end_marker();
        });
        tokens.transfer(it);
        tokens.erase(it);

        return parsing::pre_parse_action::valid_node;
    }

private:
    static_assert(utility::utf8::count(token_end_marker()) > 0,
                  "Token end markers must not be an empty string");
    static_assert(!utility::utf8::contains_whitespace(token_end_marker()),
                  "Token end markers cannot contain whitespace");
};

/** Constant variable helper.
 *
 * @tparam S compile_time_string
 */
template <typename S>
constexpr auto token_end_marker = token_end_marker_t<S>{};

template <typename S>
struct is_policy<token_end_marker_t<S>> : std::true_type {
};
}  // namespace arg_router::policy
