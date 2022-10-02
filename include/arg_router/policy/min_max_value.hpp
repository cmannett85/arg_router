/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/exception.hpp"
#include "arg_router/parsing/parsing.hpp"
#include "arg_router/policy/policy.hpp"

#include <limits>

namespace arg_router::policy
{
/** Provides inclusive minimum and maximum values for a parsed value.
 *
 * By default <TT>operator\<</TT> is used for comparisons, but can be overridden.
 * @tparam ValueType Minimum and maximum value type
 * @tparam LessThanCompare Less than comparator type
 */
template <typename ValueType, typename LessThanCompare = std::less<ValueType>>
class min_max_value_t
{
public:
    /** Value type. */
    using value_type = ValueType;
    /** Less than comparator type. */
    using less_than_compare = LessThanCompare;

    /** Min and max value constructor.
     *
     * Unlike min_max_count_t. value_type is not guaranteed to be compile-time constructible, and so
     * @em no compile-time or runtime checking is done on the validity of @a min and @a max.  Use
     * min_max_count_ct when the input types can be represented in a <TT>std::integral_constant</TT>
     * for extra compile-time checks.
     * @param min Minimum value, or empty optional if no minimum bound
     * @param max Maximum value, or empty optional if no maximum bound
     * @param compare Comparator instance
     */
    constexpr min_max_value_t(std::optional<value_type> min,
                              std::optional<value_type> max,
                              less_than_compare compare = less_than_compare{}) noexcept :
        min_(std::move(min)),
        max_(std::move(max)),  //
        comp_{std::move(compare)}
    {
    }

    /** Comparator.
     *
     * @return Compare function object
     */
    [[nodiscard]] constexpr const less_than_compare& comp() const noexcept { return comp_; }

    /** Checks that @a value is between the minimum and maximum values.
     *
     * @tparam InputValueType Parsed value type, must be implicitly constructible from value_type
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param value Parsed input value to check
     * @param parents Parents instances pack
     */
    template <typename InputValueType, typename... Parents>
    void validation_phase(const InputValueType& value,
                          [[maybe_unused]] const Parents&... parents) const
    {
        static_assert(sizeof...(Parents) >= 1, "Min/max value requires at least 1 parent");

        using node_type = boost::mp11::mp_first<std::tuple<Parents...>>;

        if (min_ && comp_(value, *min_)) {
            throw parse_exception{"Minimum value not reached",
                                  parsing::node_token_type<node_type>()};
        }
        if (max_ && comp_(*max_, value)) {
            throw parse_exception{"Maximum value exceeded", parsing::node_token_type<node_type>()};
        }
    }

private:
    std::optional<value_type> min_;
    std::optional<value_type> max_;
    less_than_compare comp_;
};

/** Compile-time equivalent to min_max_value_t.
 *
 * Use the min_max_value() function to create for ease. <TT>operator\<</TT> is used for comparisons.
 *
 * Using this policy also improves help output.
 *
 * @tparam MinType Compile-time integral constant value representing the minimum, or void if no
 * minimum bound
 * @tparam MaxType Compile-time integral constant value representing the maximum, or void if no
 * minimum bound
 */
template <typename MinType, typename MaxType>
class min_max_value_ct
{
    static_assert(!(std::is_void_v<MinType> && std::is_void_v<MaxType>),
                  "MinType and MaxType cannot both be void");

public:
    /** Constructor. */
    explicit min_max_value_ct() noexcept
    {
        if constexpr (!std::is_void_v<MinType>) {
            static_assert(traits::has_value_type_v<MinType>, "MinType must have a value_type");
            static_assert(std::is_integral_v<typename MinType::value_type> ||
                              std::is_enum_v<typename MinType::value_type>,
                          "MinType value_type must be integrals or enums");
        }
        if constexpr (!std::is_void_v<MaxType>) {
            static_assert(traits::has_value_type_v<MaxType>, "MaxType must have a value_type");
            static_assert(std::is_integral_v<typename MaxType::value_type> ||
                              std::is_enum_v<typename MaxType::value_type>,
                          "MaxType value_type must be integrals or enums");
        }

        if constexpr (!std::is_void_v<MinType> && !std::is_void_v<MaxType>) {
            static_assert(
                std::is_same_v<typename MinType::value_type, typename MaxType::value_type>,
                "MinType and MaxType must have the same value_type");
            static_assert(MinType::value <= MaxType::value,
                          "MinType must be less than or equal to MaxType");
        }
    }

    /** Returns the minimum value.
     *
     * This method is only available if there is a minimum value.
     * @return Minimum value
     */
    template <typename T = MinType>
    [[nodiscard]] constexpr static auto minimum_value(
        std::enable_if_t<!std::is_void_v<T>>* = nullptr) noexcept
    {
        return MinType::value;
    }

    /** Returns the maximum value.
     *
     * This method is only available if there is a maximum value.
     * @return Maximum value
     */
    template <typename T = MaxType>
    [[nodiscard]] constexpr static auto maximum_value(
        std::enable_if_t<!std::is_void_v<T>>* = nullptr) noexcept
    {
        return MaxType::value;
    }

    /** Checks that @a value is between the minimum and maximum values.
     *
     * @tparam InputValueType Parsed value type, must be implicitly constructible from value_type
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param value Parsed input value to check
     * @param parents Parents instances pack
     */
    template <typename InputValueType, typename... Parents>
    void validation_phase(const InputValueType& value,
                          [[maybe_unused]] const Parents&... parents) const
    {
        static_assert(sizeof...(Parents) >= 1, "Min/max value requires at least 1 parent");

        using node_type = boost::mp11::mp_first<std::tuple<Parents...>>;

        if constexpr (!std::is_void_v<MinType>) {
            if (value < minimum_value()) {
                throw parse_exception{"Minimum value not reached",
                                      parsing::node_token_type<node_type>()};
            }
        }
        if constexpr (!std::is_void_v<MaxType>) {
            if (value > maximum_value()) {
                throw parse_exception{"Maximum value exceeded",
                                      parsing::node_token_type<node_type>()};
            }
        }
    }
};

/** Constructs a min_max_value_t with the given params.
 *
 * @tparam ValueType Minimum and maximum value type
 * @tparam LessThanCompare Less than comparator type
 * @param min Minimum value
 * @param max Maximum value
 * @param compare Comparator instance
 * @return min_max_value_t instance
 */
template <typename ValueType, typename LessThanCompare = std::less<ValueType>>
[[nodiscard]] constexpr auto min_max_value(ValueType&& min,
                                           ValueType&& max,
                                           LessThanCompare&& compare = LessThanCompare{}) noexcept
{
    return min_max_value_t<ValueType, LessThanCompare>{std::forward<ValueType>(min),
                                                       std::forward<ValueType>(max),
                                                       std::forward<LessThanCompare>(compare)};
}

/** Constructs a min_max_value_t with the given params.
 *
 * @tparam ValueType Minimum value type
 * @tparam LessThanCompare Less than comparator type
 * @param min Minimum value
 * @param compare Comparator instance
 * @return min_max_value_t instance
 */
template <typename ValueType, typename LessThanCompare = std::less<ValueType>>
[[nodiscard]] constexpr auto min_value(ValueType&& min,
                                       LessThanCompare&& compare = LessThanCompare{}) noexcept
{
    return min_max_value_t<ValueType, LessThanCompare>{std::forward<ValueType>(min),
                                                       {},
                                                       std::forward<LessThanCompare>(compare)};
}

/** Constructs a min_max_value_t with the given params.
 *
 * @tparam ValueType Maximum value type
 * @tparam LessThanCompare Less than comparator type
 * @param max Maximum value
 * @param compare Comparator instance
 * @return min_max_value_t instance
 */
template <typename ValueType, typename LessThanCompare = std::less<ValueType>>
[[nodiscard]] constexpr auto max_value(ValueType&& max,
                                       LessThanCompare&& compare = LessThanCompare{}) noexcept
{
    return min_max_value_t<ValueType, LessThanCompare>{{},
                                                       std::forward<ValueType>(max),
                                                       std::forward<LessThanCompare>(compare)};
}

/** Constructs a min_max_value_ct with the given params.
 *
 * @tparam Min Minimum value
 * @tparam Max Maximum value
 * @return min_max_value_ct instance
 */
template <auto Min, auto Max>
[[nodiscard]] constexpr auto min_max_value() noexcept
{
    return min_max_value_ct<traits::integral_constant<Min>, traits::integral_constant<Max>>{};
}

/** Constructs a min_max_value_ct with the given params.
 *
 * @tparam Min Minimum value
 * @return min_max_value_ct instance
 */
template <auto Min>
[[nodiscard]] constexpr auto min_value() noexcept
{
    return min_max_value_ct<traits::integral_constant<Min>, void>{};
}

/** Constructs a min_max_value_ct with the given params.
 *
 * @tparam Max Maximum value
 * @return min_max_value_ct instance
 */
template <auto Max>
[[nodiscard]] constexpr auto max_value() noexcept
{
    return min_max_value_ct<void, traits::integral_constant<Max>>{};
}

template <typename ValueType, typename LessThanCompare>
struct is_policy<min_max_value_t<ValueType, LessThanCompare>> : std::true_type {
};

template <typename ValueType>
struct is_policy<min_max_value_t<ValueType>> : std::true_type {
};

template <typename MinType, typename MaxType>
struct is_policy<min_max_value_ct<MinType, MaxType>> : std::true_type {
};
}  // namespace arg_router::policy
