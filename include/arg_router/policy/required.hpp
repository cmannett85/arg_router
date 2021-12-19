#pragma once

#include "arg_router/exception.hpp"
#include "arg_router/parsing.hpp"
#include "arg_router/policy/policy.hpp"

#include <boost/core/ignore_unused.hpp>

#include <optional>

namespace arg_router
{
namespace policy
{
/** Used to a mark a command line argument type as required i.e. it is a parse
 * error if the token is missing.
 */
template <typename = void>  // This is needed due so it can be used in
class required_t            // template template parameters
{
public:
    /** Throw an error.
     * 
     * @tparam ValueType Parsed value type, not used in this method
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param parents Parents instances pack, not used in this method
     * @return Nothing, it will throw if called
     * @exception parse_exception Thrown if this method is called
     */
    template <typename ValueType, typename... Parents>
    [[noreturn]] ValueType missing_phase(const Parents&... parents) const
    {
        static_assert(sizeof...(Parents) >= 1,
                      "Alias requires at least 1 parent");

        using node_type = boost::mp11::mp_first<std::tuple<Parents...>>;

        boost::ignore_unused(parents...);
        throw parse_exception{"Missing required argument",
                              parsing::node_token_type<node_type>()};
    }
};

/** Constant variable helper. */
constexpr auto required = required_t<>{};

/** Evaluates to true if @a T is marked as required.
 *
 * @tparam T Type to test
 */
template <typename T>
using is_required = std::is_base_of<required_t<>, T>;

/** Helper variable for is_required.
 *
 * @tparam T Type to test
 */
template <typename T>
constexpr bool is_required_v = is_required<T>::value;

template <>
struct is_policy<required_t<>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
