// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/parsing/parse_target.hpp"
#include "arg_router/parsing/parsing.hpp"
#include "arg_router/policy/policy.hpp"
#include "arg_router/traits.hpp"
#include "arg_router/utility/compile_time_optional.hpp"
#include "arg_router/utility/utf8.hpp"

namespace arg_router::policy
{
/** Policy implementing a pre-parse phase that expands a collapsed short-form raw token into
 * multiple parsing::token_type instances.
 *
 * This is provided for node implementers, so this behaviour can be re-used amongst flag-like
 * nodes - library users should not use it (you will likely break your node's parsing behaviour if
 * it has a short name policy).
 */
template <typename = void>   // This is needed due so it can be used in
class short_form_expander_t  // template template parameters
{
public:
    /** Policy priority. */
    constexpr static auto priority = std::size_t{900};

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
        static_assert(sizeof...(Parents) > 0,
                      "Short-form expansion policy requires at least one parent");
        using owner_type = boost::mp11::mp_first<std::tuple<Parents...>>;

        static_assert(traits::has_short_name_method_v<owner_type>,
                      "Short-form expansion support requires a short name policy");

        static_assert(utility::utf8::count(owner_type::short_name()) == 1,
                      "Short name must only be 1 character");

        // Parents is always greater than zero, but having it in this statement causes it to only be
        // evaluated upon template instantiation - otherwise it would fail even if the class is
        // never used
        static_assert((sizeof...(Parents) > 0) && (config::short_prefix != config::long_prefix),
                      "Short and long prefixes cannot be the same");

        if (tokens.empty()) {
            return parsing::pre_parse_action::valid_node;
        }

        auto first = tokens.begin();
        auto first_token = *first;
        if (first_token.prefix == parsing::prefix_type::none) {
            // The token has _probably_ not been processed yet, so try to convert to short form
            const auto tt = parsing::get_token_type(first_token.name);
            if (tt.prefix != parsing::prefix_type::short_) {
                return parsing::pre_parse_action::valid_node;
            }

            first_token = tt;
        } else if (first_token.prefix == parsing::prefix_type::long_) {
            return parsing::pre_parse_action::valid_node;
        }

        // Exit early if there's no expansion to be done
        if (first_token.name.size() == owner_type::short_name().size()) {
            return parsing::pre_parse_action::valid_node;
        }

        // Move the token to the processed container
        tokens.transfer(first);

        // Insert the extra flags at the front of the unprocessed section, so they will be processed
        // independently
        tokens.unprocessed().reserve(tokens.unprocessed().size() + first_token.name.size() - 1);
        auto it = tokens.unprocessed().begin();

        // Skip past the first grapheme cluster as we'll re-use the existing short form token for
        // that
        for (auto gc_it = ++utility::utf8::iterator{first_token.name};
             gc_it != utility::utf8::iterator{};
             ++gc_it, ++it) {
            it = tokens.unprocessed().insert(it, {parsing::prefix_type::short_, *gc_it});
        }

        // Shrink the first to a single grapheme cluster
        first.set({parsing::prefix_type::short_, *utility::utf8::iterator{first_token.name}});

        return parsing::pre_parse_action::valid_node;
    }
};

/** Constant variable helper. */
constexpr auto short_form_expander = short_form_expander_t<>{};

template <>
struct is_policy<short_form_expander_t<>> : std::true_type {
};
}  // namespace arg_router::policy
