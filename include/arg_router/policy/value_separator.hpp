/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/algorithm.hpp"
#include "arg_router/parsing/parse_target.hpp"
#include "arg_router/parsing/parsing.hpp"
#include "arg_router/policy/policy.hpp"
#include "arg_router/utility/compile_time_optional.hpp"

namespace arg_router
{
namespace policy
{
/** Represents the character that separates a label token from its value token(s).
 *
 * Your terminal will separate tokens using whitespace by default, but often a different character
 * is used e.g. <TT>--arg=42</TT> - this policy specifies that character.
 * @tparam S Integral constant that can be implicitly converted to a char
 */
template <typename S>
class value_separator_t
{
    static_assert(std::is_convertible_v<S, char>,
                  "Value separator type must be implicitly convertible to char");
    static_assert(!algorithm::is_whitespace(S::value),
                  "Value separator character must not be whitespace");

    constexpr static auto value = S::value;

public:
    /** String type. */
    using string_type = S;

    /** Policy priority. */
    constexpr static auto priority = std::size_t{1000};

    /** Returns the separator.
     *
     * @return Separator character
     */
    [[nodiscard]] constexpr static std::string_view value_separator() noexcept
    {
        return std::string_view{&value, 1};
    }

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

        auto first = tokens.begin();
        const auto first_token = *first;

        // Find the separator
        const auto separator_index = first_token.name.find_first_of(value_separator());
        if (separator_index == std::string_view::npos) {
            return parsing::pre_parse_action::skip_node;
        }

        const auto value_arg = first_token.name.substr(separator_index + 1);
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
constexpr auto value_separator = value_separator_t<traits::integral_constant<S>>{};

template <typename S>
struct is_policy<value_separator_t<S>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
