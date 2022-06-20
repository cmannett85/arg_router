/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/parsing/parse_target.hpp"
#include "arg_router/parsing/parsing.hpp"
#include "arg_router/policy/policy.hpp"
#include "arg_router/traits.hpp"
#include "arg_router/utility/compile_time_optional.hpp"

namespace arg_router
{
namespace policy
{
/** Policy implementing a pre-parse phase that expands a collapsed short-form
 * raw token into multiple parsing::token_type instances.
 * 
 * This is provided for node implementers, so this behaviour can be re-used
 * amongst flag-like nodes - library users should not use it (you will likely
 * break your node's parsing behaviour if it has a short name policy).
 */
template <typename = void>   // This is needed due so it can be used in
class short_form_expander_t  // template template parameters
{
public:
    /** Policy priority. */
    constexpr static auto priority = std::size_t{900};

    /** Performs the expansion in the pre-parse phase.
     *  
     * Checks if the token's first character matches the owning node's short
     * name.  If there isn't a match or the owner does not have short name
     * policy then it just returns false.  Otherwise all the
     * characters in the token are converted into short form tokens, added
     * to @a tokens.
     *
     * @tparam ProcessedTarget @a processed_target payload type
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param tokens Currently processed tokens
     * @param processed_target Previously processed parse_target of parent
     * node, or empty is there is no non-root parent
     * @param target Pre-parse generated target
     * @param parents Parent node instances
     * @return Always returns true because if the token doesn't match the short
     * form name, the node may have a long form one that does.  No exception is
     * stored in the return value
     */
    template <typename ProcessedTarget, typename... Parents>
    [[nodiscard]] parsing::pre_parse_result pre_parse_phase(
        parsing::dynamic_token_adapter& tokens,
        [[maybe_unused]] utility::compile_time_optional<ProcessedTarget>
            processed_target,
        [[maybe_unused]] parsing::parse_target& target,
        [[maybe_unused]] const Parents&... parents) const
    {
        using owner_type = boost::mp11::mp_first<std::tuple<Parents...>>;

        static_assert(
            traits::has_short_name_method_v<owner_type>,
            "Short-form expansion support requires a short name policy");

        static_assert(owner_type::short_name().size() == 1,
                      "Short name must be a single character");

        auto first = tokens.begin();
        auto first_token = *first;
        if (first_token.prefix == parsing::prefix_type::none) {
            // The token has _probably_ not been processed yet, so try to
            // convert to short form
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

        // Insert the extra flags at the front of the unprocessed section, so
        // they will be processed independently
        tokens.unprocessed().reserve(tokens.unprocessed().size() +
                                     first_token.name.size() - 1);
        auto it = tokens.unprocessed().begin();
        for (auto i = 1u; i < first_token.name.size(); ++i) {
            tokens.unprocessed().insert(
                it++,
                {parsing::prefix_type::short_,
                 std::string_view{&(first_token.name[i]), 1}});
        }

        // Shrink the first to a single character
        first.set(
            {parsing::prefix_type::short_, first_token.name.substr(0, 1)});

        return parsing::pre_parse_action::valid_node;
    }
};

/** Constant variable helper. */
constexpr auto short_form_expander = short_form_expander_t<>{};

template <>
struct is_policy<short_form_expander_t<>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
