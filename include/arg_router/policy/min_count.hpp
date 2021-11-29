#pragma once

#include "arg_router/exception.hpp"
#include "arg_router/parsing.hpp"

#include <boost/core/ignore_unused.hpp>

namespace arg_router
{
namespace policy
{
/** For arguments that can repeat e.g. counter_flag and positional_arg, this
 * sets the inclusive minimum number of those repeats.
 *
 * @tparam T Compile-time integral constant value
 */
template <typename T>
class min_count_t
{
    static_assert(boost::mp11::mp_valid<traits::get_value_type, T>::value,
                  "T must have a value_type");
    static_assert(std::is_convertible_v<typename T::value_type, std::size_t>,
                  "T must have a value_type that is implicitly convertible to "
                  "std::size_t");
    static_assert(std::is_integral_v<typename T::value_type>,
                  "T must be an integral type");
    static_assert((T::value >= 0),
                  "T must have a value_type that is a positive number");

public:
    /** @return Minimum count value. */
    constexpr static std::size_t minimum_count() { return T::value; }

protected:
    /** Check the number elements in @a value is not less than minimum_count().
     * 
     * @note This phase does not exist if @a ValueType does not have a
     * <TT>size()</TT> method
     * @tparam ValueType Parsed value type
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param value Parsed value
     * @param parents Parents instances pack
     * @return void
     * @exception parse_exception Thrown if @a value has a size() less than
     * minimum_count()
     */
    template <typename ValueType, typename... Parents>
    std::enable_if_t<traits::has_size_method_v<ValueType>> validation_phase(
        const ValueType& value,
        const Parents&... parents) const
    {
        static_assert(sizeof...(Parents) >= 1,
                      "Alias requires at least 1 parent");

        using node_type = boost::mp11::mp_first<std::tuple<Parents...>>;

        boost::ignore_unused(parents...);
        if (value.size() < minimum_count()) {
            throw parse_exception{"Minimum count not reached",
                                  parsing::node_token_type<node_type>()};
        }
    }
};

/** Constant variable helper.
 *
 * @tparam Value Minimum count value
 */
template <std::size_t Value>
constexpr auto min_count = min_count_t<traits::integral_constant<Value>>{};

template <typename T>
struct is_policy<min_count_t<T>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
