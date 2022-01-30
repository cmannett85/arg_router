#pragma once

#include "arg_router/config.hpp"
#include "arg_router/exception.hpp"
#include "arg_router/parsing.hpp"
#include "arg_router/policy/policy.hpp"

#include <limits>

namespace arg_router
{
namespace policy
{
/** Provides inclusive minimum and maximum values for an parsed value.
 *
 * By default <TT>operator\<</TT> is used for comparisons, but can be
 * overridden.
 * @tparam ValueType Minimum and maximum value type
 * @tparam LessThanCompare Less than comparator type
 */
template <typename ValueType, typename LessThanCompare = std::less<ValueType>>
class min_max_value
{
public:
    /** Value type. */
    using value_type = ValueType;
    /** Less than comparator type. */
    using less_than_compare = LessThanCompare;

    /** Constructor.
     * 
     * Unlike min_max_value_t. value_type is not guaranteed to be compile-time
     * constructible, and so to avoid runtime overhead, @em no compile-time or
     * runtime checking is done on the validity of @a min and @a max.
     * @param min Minimum value
     * @param max Maximum value
     * @param compare Comparator instance
     */
    constexpr min_max_value(value_type min,
                            value_type max,
                            less_than_compare compare = less_than_compare{}) :
        min_(std::move(min)),
        max_(std::move(max)),  //
        comp_{std::move(compare)}
    {
    }

    /** Returns the minimum value.
     *
     * @return Value, a reference to it if the object is larger than a
     * cache line
     */
    constexpr auto minimum_value() const
        -> std::conditional_t<config::l1_cache_size() >= sizeof(value_type),
                              value_type,
                              const value_type&>
    {
        return min_;
    }

    /** Returns the maximum value.
     *
     * @return Value, a reference to it if the object is larger than a
     * cache line
     */
    constexpr auto maximum_value() const
        -> std::conditional_t<config::l1_cache_size() >= sizeof(value_type),
                              value_type,
                              const value_type&>
    {
        return max_;
    }

    /** Comparator.
     *
     * @return Compare function object
     */
    constexpr const less_than_compare& comp() const { return comp_; }

    /** Checks that @a value is between the minimum and maximum values.
     *
     * @tparam InputValueType Parsed value type, must be implicitly
     * constructible from value_type
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param value Parsed input value to check
     * @param parents Parents instances pack
     */
    template <typename InputValueType, typename... Parents>
    void validation_phase(const InputValueType& value,
                          [[maybe_unused]] const Parents&... parents) const
    {
        static_assert(sizeof...(Parents) >= 1,
                      "Min/max value requires at least 1 parent");

        using node_type = boost::mp11::mp_first<std::tuple<Parents...>>;

        if (comp_(value, min_)) {
            throw parse_exception{"Minimum value not reached",
                                  parsing::node_token_type<node_type>()};
        } else if (comp_(max_, value)) {
            throw parse_exception{"Maximum value exceeded",
                                  parsing::node_token_type<node_type>()};
        }
    }

private:
    value_type min_;
    value_type max_;
    less_than_compare comp_;
};

template <typename ValueType, typename LessThanCompare>
struct is_policy<min_max_value<ValueType, LessThanCompare>> : std::true_type {
};

template <typename ValueType>
struct is_policy<min_max_value<ValueType>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
