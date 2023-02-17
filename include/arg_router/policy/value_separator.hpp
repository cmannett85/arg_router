// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/parsing/parse_target.hpp"
#include "arg_router/parsing/parsing.hpp"
#include "arg_router/policy/policy.hpp"
#include "arg_router/utility/compile_time_optional.hpp"
#include "arg_router/utility/compile_time_string.hpp"

namespace arg_router::policy
{
/** Represents the character that separates a label token from its value token(s).
 *
 * Your terminal will separate tokens using whitespace by default, but often a different character
 * is used e.g. <TT>--arg=42</TT> - this policy specifies that character.
 *
 * If using C++17 then use the template variable helper with the <TT>S_</TT> macro or char; for
 * C++20 and higher, use the char variable helper or the constructor directly with a compile-time
 * string literal:
 * @code
 * constexpr auto a = ar::policy::value_separator<'='>;
 * constexpr auto b = ar::policy::value_separator_utf8<S_("=")>;
 * constexpr auto c = ar::policy::value_separator_t{"="_S};
 * @endcode
 * @tparam S Compile-time string
 */
template <typename S>
class value_separator_t
{
    static_assert(utility::utf8::count(S::get()) == 1,
                  "Value separator must only be one character");
    static_assert(!utility::utf8::is_whitespace(S::get()),
                  "Value separator character must not be whitespace");

public:
    /** String type. */
    using string_type = S;

    /** Policy priority. */
    constexpr static auto priority = std::size_t{1000};

    /** Constructor.
     *
     * @param str String instance
     */
    constexpr explicit value_separator_t([[maybe_unused]] S str = {}) noexcept {}

    /** Returns the separator.
     *
     * @return Separator character
     */
    [[nodiscard]] constexpr static std::string_view value_separator() noexcept { return S::get(); }

    /** Splits the label token from the value using the separator.
     *
     * @tparam ProcessedTarget @a processed_target payload type
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param tokens Currently processed tokens
     * @param processed_target Previously processed parse_target of parent node, or empty is there
     * is no non-root parent
     * @param target Pre-parse generated targets
     * @param parents Parent node instances
     * @return True if the owning node's label token matches the label part of the first token,
     * false otherwise.  No exception is stored in the return value
     */
    template <typename ProcessedTarget, typename... Parents>
    [[nodiscard]] parsing::pre_parse_result pre_parse_phase(
        parsing::dynamic_token_adapter& tokens,
        [[maybe_unused]] utility::compile_time_optional<ProcessedTarget> processed_target,
        [[maybe_unused]] parsing::parse_target& target,
        [[maybe_unused]] const Parents&... parents) const
    {
        static_assert((sizeof...(Parents) >= 1),
                      "At least one parent needed for value_separator_t");

        using owner_type = boost::mp11::mp_first<std::tuple<Parents...>>;

        static_assert(traits::has_minimum_count_method_v<owner_type> &&
                          traits::has_maximum_count_method_v<owner_type>,
                      "Value separator support requires an owning node to have "
                      "minimum and maximum count policies");
        static_assert((owner_type::minimum_count() == 1) && (owner_type::maximum_count() == 1),
                      "Value separator support requires an owning node to have "
                      "a fixed count of 1");

        if (tokens.empty()) {
            return parsing::pre_parse_action::valid_node;
        }

        auto first = tokens.begin();
        const auto first_token = *first;

        // Find the separator
        const auto separator_index = first_token.name.find_first_of(value_separator());
        if (separator_index == std::string_view::npos) {
            return parsing::pre_parse_action::skip_node;
        }

        const auto value_arg = first_token.name.substr(separator_index + S::get().size());
        if (value_arg.empty()) {
            return parsing::pre_parse_action::skip_node;
        }

        // Insert value token after the label one
        tokens.insert(first + 1, {parsing::prefix_type::none, value_arg});

        // Remove the value and separator part of the label token
        const auto label_arg = first_token.name.substr(0, separator_index);
        first.set({first_token.prefix, label_arg});

        return parsing::pre_parse_action::valid_node;
    }
};

/** Constant variable helper.
 *
 * @tparam S Arg/value separator character
 */
template <char S>
constexpr auto value_separator = value_separator_t<AR_STRING(S)>{};

/** Constant variable helper that supports UTF-8 code points.
 *
 * @tparam S UTF-8 code point
 */
template <typename S>
constexpr auto value_separator_utf8 = value_separator_t<S>{};

template <typename S>
struct is_policy<value_separator_t<S>> : std::true_type {
};
}  // namespace arg_router::policy
