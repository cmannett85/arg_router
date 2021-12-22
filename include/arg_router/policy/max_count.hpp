#pragma once

#include "arg_router/policy/policy.hpp"
#include "arg_router/token_type.hpp"
#include "arg_router/traits.hpp"

#include <boost/core/ignore_unused.hpp>

namespace arg_router
{
namespace policy
{
/** For arguments that can repeat e.g. counter_flag and positional_arg, this
 * sets the inclusive maximum number of those repeats.
 *
 * @tparam T Compile-time integral constant value
 */
template <typename T>
class max_count_t
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
    /** @return Maximum count value. */
    constexpr static std::size_t maximum_count() { return T::value; }

protected:
    /** Limit the number entries in @a view to the first maximum_count().
     * 
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param tokens Token list as received from the owning node, this is not
     * modified
     * @param view The tokens to be used in the remaining parse phases
     * @param parents Parents instances pack
     */
    template <typename... Parents>
    void pre_parse_phase(parsing::token_list& tokens,
                         utility::span<const parsing::token_type>& view,
                         const Parents&... parents) const
    {
        boost::ignore_unused(tokens, parents...);
        if (view.size() > maximum_count()) {
            view = view.subspan(0, maximum_count());
        }
    }
};

/** Constant variable helper.
 *
 * @tparam Value Maximum count value
 */
template <std::size_t Value>
constexpr auto max_count = max_count_t<traits::integral_constant<Value>>{};

template <typename T>
struct is_policy<max_count_t<T>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
