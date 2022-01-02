#pragma once

#include "arg_router/exception.hpp"
#include "arg_router/parsing.hpp"
#include "arg_router/policy/policy.hpp"

#include <boost/core/ignore_unused.hpp>

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
    /** @return Minimum count value. */
    constexpr static std::size_t minimum_count() { return MinType::value; }

    /** @return Maximum count value. */
    constexpr static std::size_t maximum_count() { return MaxType::value; }

protected:
    /** Checks that there are enough pending tokens in the list to reach the
     * minimum count.
     * 
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param tokens Token list as received from the owning node, this is not
     * modified
     * @param parents Parents instances pack
     * @exception parse_exception Thrown if minimum count not reached
     */
    template <typename... Parents>
    void pre_parse_phase(parsing::token_list& tokens,
                         const Parents&... parents) const
    {
        static_assert(sizeof...(Parents) >= 1,
                      "Alias requires at least 1 parent");

        using node_type = boost::mp11::mp_first<std::tuple<Parents...>>;

        boost::ignore_unused(parents...);
        if (tokens.pending_view().size() < minimum_count()) {
            throw parse_exception{"Minimum count not reached",
                                  parsing::node_token_type<node_type>()};
        }
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
