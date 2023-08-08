// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/exception.hpp"
#include "arg_router/parsing/parsing.hpp"
#include "arg_router/policy/policy.hpp"
#include "arg_router/traits.hpp"
#include "arg_router/utility/compile_time_optional.hpp"

#include <limits>

namespace arg_router
{
namespace policy
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
        constexpr auto named_offset = owner_type::is_named ? 1u : 0u;
        constexpr auto mn_count = minimum_count() == std::numeric_limits<std::size_t>::max() ?
                                      minimum_count() :
                                      minimum_count() + named_offset;
        constexpr auto mx_count = maximum_count() == std::numeric_limits<std::size_t>::max() ?
                                      maximum_count() :
                                      maximum_count() + named_offset;

        auto num_tokens = tokens.size();
        if constexpr (traits::has_token_end_marker_method_v<owner_type>) {
            // If there's a token end marker attached, then it will have already processed the
            // tokens as it is a higher priority policy
            num_tokens = tokens.processed().size();

            // We can only check that we have exceeded the maximum if there's a token end marker
            // because know that all the tokens preceding it are for the owner
            if (num_tokens > mx_count) {
                return multi_lang_exception{error_code::maximum_count_exceeded,
                                            parsing::node_token_type<owner_type>()};
            }
        }

        // Check that we are within the minimum bound
        if (num_tokens < mn_count) {
            return multi_lang_exception{error_code::minimum_count_not_reached,
                                        parsing::node_token_type<owner_type>()};
        }

        // Transfer any remaining up to the maximum count
        tokens.transfer(tokens.begin() + std::min(mx_count - 1, num_tokens - 1));

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
}  // namespace policy

/** Provides a tree_node type with an unbounded policy::min_max_count_t if a compatible one is not
 * present in @a Policies.
 *
 * If no policy implementing <TT>minimum_count()</TT> and <TT>maximum_count()</TT> methods is in
 * @a Policies (e.g. policy::min_max_count_t), then an unbounded policy::min_max_count_t is
 * prepended to @a Policies.
 *
 * This is used via inheritance in nodes, e.g.:
 * @code
 * template <typename T, typename... Policies>
 * class my_arg_t : public add_missing_min_max_policy<0, Policies...>::type
 * {
 * public:
 *  template <auto has_min_max = add_missing_min_max_policy<0, Policies...>::has_min_max>
 *  constexpr explicit my_arg_t(Policies... policies,
 *                              std::enable_if_t<has_min_max>* = nullptr) noexcept :
 *      parent_type{std::move(policies)...}
 * {
 * }
 *
 * template <auto has_min_max = add_missing_min_max_policy<0, Policies...>::has_min_max>
 * constexpr explicit multi_arg_base_t(Policies... policies,
 *                                     std::enable_if_t<!has_min_max>* = nullptr) noexcept :
 *   parent_type{policy::min_count<0>, std::move(policies)...}
 * {
 * }
 * };
 * @endcode
 * The constructor specialisation is needed because the parent constructor calls need to match the
 * inherited policies count and order.
 *
 * @tparam MinCount Minimum count value to use if one not specified by user
 * @tparam Policies Pack of policies that define its behaviour
 */
template <std::size_t MinCount, typename... Policies>
class add_missing_min_max_policy
{
    template <typename Policy>
    struct has_min_max_t {
        constexpr static bool value = traits::has_minimum_count_method_v<Policy> &&
                                      traits::has_maximum_count_method_v<Policy>;
    };

    using policies_tuple = std::tuple<std::decay_t<Policies>...>;
    using range_policy_type =
        policy::min_max_count_t<traits::integral_constant<MinCount>,
                                traits::integral_constant<std::numeric_limits<std::size_t>::max()>>;

public:
    /** True if there is no policy implementing <TT>minimum_count()</TT> and
     * <TT>maximum_count()</TT> methods in @a Policies.
     */
    constexpr static auto has_min_max =
        boost::mp11::mp_find_if<policies_tuple, has_min_max_t>::value !=
        std::tuple_size_v<policies_tuple>;

    /** Equivalent to <TT>tree_node\<decltype(policy::min_count(0)), Policies...\></TT> if
     * has_min_max is false, otherwise <TT>tree_node\<Policies...\></TT>
     */
    using type = std::conditional_t<
        has_min_max,
        boost::mp11::mp_rename<policies_tuple, tree_node>,
        boost::mp11::mp_rename<boost::mp11::mp_push_front<policies_tuple, range_policy_type>,
                               tree_node>>;
};
}  // namespace arg_router
