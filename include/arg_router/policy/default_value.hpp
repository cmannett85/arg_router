#pragma once

#include "arg_router/config.hpp"
#include "arg_router/policy/policy.hpp"

#include <boost/core/ignore_unused.hpp>

#include <optional>

namespace arg_router
{
namespace policy
{
/** Provides a default value for non-required arguments.
 *
 * @tparam T Value type
 */
template <typename T>
class default_value
{
public:
    /** Alias of @a T. */
    using value_type = T;

    /** Constructor
     *
     * @param value Default value
     */
    constexpr explicit default_value(T value) : value_{std::move(value)} {}

    /** Returns the default value.
     *
     * @return Default value, a reference to it if the object is larger than a
     * cache line
     */
    constexpr auto get_default_value() const
        -> std::conditional_t<config::l1_cache_size() >= sizeof(T), T, const T&>
    {
        return value_;
    }

protected:
    /** If @a value is empty, then populate it with the default value.
     * 
     * @tparam ValueType Parsed value type, must be implicitly constructible
     * from value_type
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param value Parsed value, may be empty
     * @param parents Parents instances pack
     */
    template <typename ValueType, typename... Parents>
    void post_parse_phase(std::optional<ValueType>& value,
                          const Parents&... parents) const
    {
        boost::ignore_unused(parents...);
        if (!value) {
            value = value_;
        }
    }

private:
    T value_;
};

template <typename T>
struct is_policy<default_value<T>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
