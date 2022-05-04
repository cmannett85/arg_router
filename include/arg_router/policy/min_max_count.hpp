/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/exception.hpp"
#include "arg_router/parsing.hpp"
#include "arg_router/policy/policy.hpp"

#include <limits>

namespace arg_router
{
namespace policy
{
/** Exposes the number of tokens the owning node will consume.
 *
 * It also checks that there are enough pending tokens available to reach the
 * minimum in the pre-parse phase.
 * @tparam MinType Compile-time integral constant value representing the minimum
 * @tparam MaxType Compile-time integral constant value representing the maximum
 */
template <typename MinType, typename MaxType>
class min_max_count_t
{
    static_assert(traits::has_value_type_v<MinType> &&
                      traits::has_value_type_v<MaxType>,
                  "MinType and MaxType must have a value_type");
    static_assert(
        std::is_convertible_v<typename MinType::value_type, std::size_t> &&
            std::is_convertible_v<typename MaxType::value_type, std::size_t>,
        "MinType and MaxType must have a value_type that is implicitly "
        "convertible to std::size_t");
    static_assert(std::is_integral_v<typename MinType::value_type> &&
                      std::is_integral_v<typename MaxType::value_type>,
                  "MinType and MaxType value_types must be integrals");
    static_assert(
        (MinType::value >= 0) && (MaxType::value >= 0),
        "MinType and MaxType must have a value that is a positive number");
    static_assert(MinType::value <= MaxType::value,
                  "MinType must be less than or equal to MaxType");

public:
    /** Policy priority. */
    constexpr static auto priority = std::size_t{750};

    /** @return Minimum count value. */
    [[nodiscard]] constexpr static std::size_t minimum_count() noexcept
    {
        return MinType::value;
    }

    /** @return Maximum count value. */
    [[nodiscard]] constexpr static std::size_t maximum_count() noexcept
    {
        return MaxType::value;
    }

    /** Copies an appropriate amount of tokens from @a args to @a result.
     * 
     * This policy performs label and bulk value token processing.  If the
     * owning node is named, then the first token is expected to match, if not
     * then false is returned immediately.
     * 
     * Then up to maximum_count() @result the @arg tokens are processed.  If the
     * maximum available is less than minimum_count(), a parse error will occur.
     * 
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param tokens Currently processed tokens
     * @param processed_tokens Processed tokens performed by previous pre-parse
     * phases calls on other nodes
     * @param parents Parent node instances
     * @return Either true if successful, or a parse_exception if the minimum
     * count is not reached
     */
    template <typename... Parents>
    [[nodiscard]] parsing::pre_parse_result pre_parse_phase(
        parsing::dynamic_token_adapter& tokens,
        [[maybe_unused]] parsing::token_list::pending_view_type
            processed_tokens,
        [[maybe_unused]] const Parents&... parents) const
    {
        static_assert((sizeof...(Parents) >= 1),
                      "At least one parent needed for min_max_count_t");

        using owner_type = boost::mp11::mp_first<std::tuple<Parents...>>;

        constexpr auto min_count =
            minimum_count() + (owner_type::is_named ? 1 : 0);
        constexpr auto max_count = maximum_count();

        // Check that we are in the bounds.  We can't check that we have
        // exceeded the maximum because there may be other nodes that will
        // consume tokens, so this node will take its maximum (potentially) and
        // any left over tokens will be trigger an error later in the processing
        if (tokens.size() < min_count) {
            return parse_exception{"Minimum count not reached",
                                   parsing::node_token_type<owner_type>()};
        }

        // Transfer any remaining up to the maximum count
        tokens.transfer(tokens.begin() + std::min(max_count, tokens.size()));

        return parsing::pre_parse_action::valid_node;
    }
};

/** Constant variable helper.
 *
 * @tparam MinValue Minimum count value
 * @tparam MaxValue Maximum count value
 */
template <std::size_t MinValue, std::size_t MaxValue>
constexpr auto min_max_count =
    min_max_count_t<traits::integral_constant<MinValue>,
                    traits::integral_constant<MaxValue>>{};

/** Constant variable helper for a minimum count with an unbounded maximum
 * count.
 *
 * @tparam Value Minimum count value
 */
template <std::size_t Value>
constexpr auto min_count = min_max_count_t<
    traits::integral_constant<Value>,
    traits::integral_constant<std::numeric_limits<std::size_t>::max()>>{};

/** Constant variable helper for a maximum count with a minimum count of zero.
 *
 * @tparam Value Maximum count value
 */
template <std::size_t Value>
constexpr auto max_count =
    min_max_count_t<traits::integral_constant<std::size_t{0}>,
                    traits::integral_constant<Value>>{};

/** Constant variable helper for a count of fixed size.
 *
 * @tparam Value Minimum and maximum count value
 */
template <std::size_t Value>
constexpr auto fixed_count =
    min_max_count_t<traits::integral_constant<Value>,
                    traits::integral_constant<Value>>{};

template <typename MinType, typename MaxType>
struct is_policy<min_max_count_t<MinType, MaxType>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
