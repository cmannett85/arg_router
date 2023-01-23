// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/exception.hpp"
#include "arg_router/parsing/parse_target.hpp"
#include "arg_router/parsing/parsing.hpp"
#include "arg_router/policy/policy.hpp"
#include "arg_router/utility/compile_time_optional.hpp"

#include <limits>

namespace arg_router::policy
{
/** Exposes the number of tokens the owning node will consume.
 *
 * It also checks that there are enough pending tokens available to reach the minimum in the
 * pre-parse phase.
 * @tparam MinType Compile-time integral constant value representing the minimum
 * @tparam MaxType Compile-time integral constant value representing the maximum
 */
template <typename MinType, typename MaxType>
class min_max_count_t
{
    static_assert(traits::has_value_type_v<MinType> && traits::has_value_type_v<MaxType>,
                  "MinType and MaxType must have a value_type");
    static_assert(std::is_convertible_v<typename MinType::value_type, std::size_t> &&
                      std::is_convertible_v<typename MaxType::value_type, std::size_t>,
                  "MinType and MaxType must have a value_type that is implicitly "
                  "convertible to std::size_t");
    static_assert(std::is_integral_v<typename MinType::value_type> &&
                      std::is_integral_v<typename MaxType::value_type>,
                  "MinType and MaxType value_types must be integrals");
    static_assert((MinType::value >= 0) && (MaxType::value >= 0),
                  "MinType and MaxType must have a value that is a positive number");
    static_assert(MinType::value <= MaxType::value,
                  "MinType must be less than or equal to MaxType");

public:
    /** Policy priority. */
    constexpr static auto priority = std::size_t{750};

    /** @return Minimum count value. */
    [[nodiscard]] constexpr static std::size_t minimum_count() noexcept { return MinType::value; }

    /** @return Maximum count value. */
    [[nodiscard]] constexpr static std::size_t maximum_count() noexcept { return MaxType::value; }

    /** Copies an appropriate amount of tokens from @a args to @a result.
     *
     * This policy performs label and bulk value token processing.  If the owning node is named,
     * then the first token is expected to match, if not then false is returned immediately.
     *
     * Then up to maximum_count() @result the @arg tokens are processed.  If the maximum available
     * is less than minimum_count(), a parse error will occur.
     *
     * @tparam ProcessedTarget @a processed_target payload type
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param tokens Currently processed tokens
     * @param processed_target Previously processed parse_target of parent node, or empty is there
     * is no non-root parent
     * @param target Pre-parse generated target
     * @param parents Parent node instances
     * @return Either true if successful, or a multi_lang_exception if the minimum count is not
     * reached
     */
    template <typename ProcessedTarget, typename... Parents>
    [[nodiscard]] parsing::pre_parse_result pre_parse_phase(
        parsing::dynamic_token_adapter& tokens,
        [[maybe_unused]] utility::compile_time_optional<ProcessedTarget> processed_target,
        [[maybe_unused]] parsing::parse_target& target,
        [[maybe_unused]] const Parents&... parents) const
    {
        static_assert((sizeof...(Parents) >= 1), "At least one parent needed for min_max_count_t");

        using owner_type = boost::mp11::mp_first<std::tuple<Parents...>>;

        // The min/max values are for argument counts, and so need adjusting to accomodate the label
        // tokens
        constexpr auto mn_count = minimum_count() + (owner_type::is_named ? 1 : 0);
        constexpr auto mx_count = maximum_count() == 0 ?
                                      maximum_count() :
                                      (maximum_count() - (owner_type::is_named ? 0 : 1));

        // If we are unnamed then check that tokens haven't already been parsed for the owner.  This
        // is for the scenario where there are multiple positional arg-like types under a mode, and
        // once one has been processed, then it should be skipped so subsequent ones are used
        // instead
        if constexpr (!owner_type::is_named) {
            static_assert(!processed_target.empty, "processed_target cannot be empty");

            if (has_filled_tokens(*processed_target, utility::type_hash<owner_type>(), mx_count)) {
                return parsing::pre_parse_action::skip_node;
            }
        }

        // Check that we are in the bounds.  We can't check that we have exceeded the maximum
        // because there may be other nodes that will consume tokens, so this node will take its
        // maximum (potentially) and any left over tokens will be trigger an error later in the
        // processing
        if (tokens.size() < mn_count) {
            return multi_lang_exception{error_code::minimum_count_not_reached,
                                        parsing::node_token_type<owner_type>()};
        }

        // Transfer any remaining up to the maximum count
        tokens.transfer(tokens.begin() + std::min(mx_count, tokens.size()));

        return parsing::pre_parse_action::valid_node;
    }

private:
    [[nodiscard]] static bool has_filled_tokens(const parsing::parse_target& target,
                                                std::size_t owner_hash_code,
                                                std::size_t mx_count) noexcept
    {
        if (target.node_type() == owner_hash_code) {
            return target.tokens().size() >= mx_count;
        }

        for (const auto& sub_target : target.sub_targets()) {
            if (has_filled_tokens(sub_target, owner_hash_code, mx_count)) {
                return true;
            }
        }

        return false;
    }
};

/** Constant variable helper.
 *
 * @tparam MinValue Minimum count value
 * @tparam MaxValue Maximum count value
 */
template <std::size_t MinValue, std::size_t MaxValue>
constexpr auto min_max_count =
    min_max_count_t<traits::integral_constant<MinValue>, traits::integral_constant<MaxValue>>{};

/** Constant variable helper for a minimum count with an unbounded maximum count.
 *
 * @tparam Value Minimum count value
 */
template <std::size_t Value>
constexpr auto min_count =
    min_max_count_t<traits::integral_constant<Value>,
                    traits::integral_constant<std::numeric_limits<std::size_t>::max()>>{};

/** Constant variable helper for a maximum count with a minimum count of zero.
 *
 * @tparam Value Maximum count value
 */
template <std::size_t Value>
constexpr auto max_count =
    min_max_count_t<traits::integral_constant<std::size_t{0}>, traits::integral_constant<Value>>{};

/** Constant variable helper for a count of fixed size.
 *
 * @tparam Value Minimum and maximum count value
 */
template <std::size_t Value>
constexpr auto fixed_count =
    min_max_count_t<traits::integral_constant<Value>, traits::integral_constant<Value>>{};

template <typename MinType, typename MaxType>
struct is_policy<min_max_count_t<MinType, MaxType>> : std::true_type {
};
}  // namespace arg_router::policy
